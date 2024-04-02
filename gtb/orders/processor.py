from typing import List, Callable
from functools import partial
from datetime import datetime
from dateutil.relativedelta import relativedelta
from dateutil import parser as date_parser

from gtb.core.thread import BotThread
from gtb.core.context import Context
from gtb.core.settings import Settings
from gtb.orders.order import Order, OrderInfo
from gtb.orders.order_pair import OrderPair
from gtb.orders.cancel import cancel_order
from gtb.utils.logging import Log
from gtb.utils.maths import floor_usd
from gtb.utils.wallet import Wallet

from coinbase.rest import orders as order_api

class OrderProcessor(BotThread):

    def __init__(self, ctx: Context) -> None:
        super().__init__(ctx, sleep_seconds = 0.2)

    def init(self) -> None:
        pass

    def think(self) -> None:
        # Process every pair in the orderbook
        pairs: List[int] = self.ctx.order_book.get_ids()
        for pair in pairs:
            self.ctx.order_book.process_id(pair, partial(OrderProcessor.process, self))

        # Cleanup orderbook
        self.ctx.order_book.cleanup(self.ctx.history)
        # Save orderbook changes to fs
        self.ctx.order_book.write_fs()

    def process(self, pair: OrderPair) -> None:
        # Find the active order to check status
        active_order: Order | None = None
        if pair.buy.status == Order.Status.Active:
            active_order = pair.buy
        elif pair.sell and pair.sell.status == Order.Status.Active:
            active_order = pair.sell

        # Check on the active order
        if active_order:
            assert active_order is not None
            try:
                self.check_active(pair, active_order)
            except Exception as e:
                Log.exception("Failed to check {} order status for {}".format(
                    active_order.order_type.name,
                    pair.algorithm,
                ), e)

        # If there is a pending order, see if it is time to queue it
        pending_order: Order | None = None
        if pair.buy.status == Order.Status.Pending:
            pending_order = pair.buy
        elif pair.sell and pair.sell.status == Order.Status.Pending and pair.buy.status == Order.Status.Complete:
            pending_order = pair.sell

        # Queue order
        if pending_order:
            assert pending_order is not None
            try:
                self.queue_pending(pair, pending_order)
            except Exception as e:
                Log.exception("Failed to queue pending {} order for {}".format(
                    pending_order.order_type.name,
                    pair.algorithm,
                ), e)

        # Update pair status
        pair.update_status()

    def check_active(self, pair: OrderPair, order: Order) -> None:
        assert order.info is not None
        # Query active order status
        data: dict = order_api.get_order(self.ctx.api, order_id=order.info.order_id)
        # Move active to complete/canceled if necessary
        status: str = data['order']['status']
        # Completed
        if status == "FILLED":
            order.status = Order.Status.Complete
            order.info.final_market = float(data['order']['average_filled_price'])
            order.info.final_fees = float(data['order']['total_fees'])
            order.info.final_time = date_parser.parse(data['order']['last_fill_time'])
            if order.order_type == Order.Type.Buy:
                order.info.final_usd = float(data['order']['total_value_after_fees'])
            else:
                order.info.final_usd = float(data['order']['filled_value'])
            Log.info("{} order for {} filled (${:.2f} USD @ ${:.2f}) (${:.2f} fee).".format(
                order.order_type.name,
                pair.algorithm,
                order.info.final_usd,
                order.info.final_market,
                order.info.final_fees,
            ))

            if order.order_type == Order.Type.Buy:
                self.ctx.notify.queue("Buy order ${:.2f} filled ({})".format(order.info.final_usd, pair.algorithm))
                pass
            else:
                assert pair.buy.info is not None
                assert pair.buy.info.final_fees is not None
                assert pair.buy.info.final_usd is not None
                assert pair.sell is not None
                assert pair.sell.info is not None
                assert pair.sell.info.final_fees is not None
                assert pair.sell.info.final_usd is not None
                fees: float = pair.sell.info.final_fees # buy price includes fees
                total: float = pair.sell.info.final_usd - pair.buy.info.final_usd - fees
                if total < 0.01 and total > -0.01:
                    self.ctx.notify.queue("Trade broke even ({})".format(pair.algorithm))
                elif total > 0:
                    self.ctx.notify.queue("Trade made ${:.2f} profit ({})".format(total, pair.algorithm))
                else:
                    self.ctx.notify.queue("Trade made ${:.2f} loss ({})".format(total * -1, pair.algorithm))
        # Canceled
        elif status != "OPEN":
            # TODO: Partially filled orders are possible?
            Log.info("{} order for {} canceled ({}).".format(
                order.order_type.name,
                pair.algorithm,
                status,
            ))
            order.status = Order.Status.Canceled
        # Stale
        elif order.info.order_time + relativedelta(minutes=2) < datetime.now():
            # If we're far off the mark, lets set it to pending so it gets requeued
            buy: bool = order.order_type == Order.Type.Buy
            market: float = self.ctx.current_market.bid if order.order_type == Order.Type.Buy else self.ctx.current_market.ask
            if (buy and market - order.info.order_market > 100.0) or (not buy and order.info.order_market - market > 100.0):
                Log.info("Stale {} for {}.".format(
                    order.order_type.name,
                    pair.algorithm,
                ))
                # Cancel and set back to pending
                if cancel_order(self.ctx, pair.algorithm, order, "stale"):
                    order.status = Order.Status.Pending

    def queue_pending(self, pair: OrderPair, order: Order) -> None:
        # Need at least a dollar spread between bid/ask for market conditions to be favorable
        if self.ctx.current_market.ask - self.ctx.current_market.bid < 1.0:
            return None

        if order.order_type == Order.Type.Buy and not self.check_allocation(pair.algorithm, order.usd):
            if not order.insufficient_funds:
                Log.error("No allocation available for ${:.2f} {}.".format(
                    order.usd,
                    pair.algorithm,
                ))
                order.insufficient_funds = True
            return None

        # This is the market price we want to buy/sell at
        order_price: float = order.get_limit_price()

        # Determine if it's the appropriate time to queue our order
        queue: Callable
        final_price: float
        queue_price: float
        if order.order_type == Order.Type.Buy:
            queue = order_api.limit_order_gtc_buy
            queue_price = floor_usd(self.ctx.current_market.ask - 40.0)
            final_price = queue_price - 20.0
            if queue_price < order_price:
                return None
            if final_price > order_price:
                return None
        elif order.order_type == Order.Type.Sell:
            queue = order_api.limit_order_gtc_sell
            queue_price = floor_usd(self.ctx.current_market.bid + 40.0)
            final_price = queue_price + 20.0
            if final_price < order_price:
                return None

        # Randomize a client order ID
        client_order_id: str = datetime.now().strftime("%Y-%m-%d-%H-%M-%S-%f-") + order.order_type.name

        # Queue order
        order_info: dict = queue(
            self.ctx.api,
            client_order_id=client_order_id,
            product_id='BTC-USD',
            base_size="{:.8f}".format(order.btc),
            limit_price=str(final_price),
        )

        # Success: Setup OrderInfo and move to active state
        if 'success' in order_info and order_info['success']:
            order.info = OrderInfo(order_info['order_id'], client_order_id, datetime.now(), final_price)
            order.status = Order.Status.Active
            Log.info("Created {} order for {} (${:.2f} USD @ ${:.2f}).".format(
                order.order_type.name,
                pair.algorithm,
                order.usd,
                final_price))
        # Log error
        else:
            if order_info['error_response']['error'] == 'INSUFFICIENT_FUND':
                if order.insufficient_funds:
                    return None
                order.insufficient_funds = True
            Log.error("Failed to created {} order for {} (${:.2f} USD @ ${:.2f}): {}".format(
                order.order_type.name,
                pair.algorithm,
                order.usd,
                final_price,
                order_info['error_response']['error'],
            ))

    def check_allocation(self, algorithm: str, usd: float) -> bool:
        # Drop sub-identifier
        algorithm = algorithm.split('-')[0]
        # Shouldn't happen
        if not algorithm in Settings.allocations:
            return True

        # Get wallet
        wallet: Wallet | None = Wallet.get(self.ctx)
        if not wallet:
            return False
        assert wallet is not None

        # If usd < wallet available: can't buy
        if wallet.usd_available < usd:
            return False

        # Get all USD for active trades and its allocations
        all_pending_usd: float = 0.0
        alg_pending_usd: float = 0.0
        for pair in self.ctx.order_book.order_pairs:
            if pair.status in [
                    OrderPair.Status.Active,
                    OrderPair.Status.OnHoldSell,
                    OrderPair.Status.PendingSell,
                    OrderPair.Status.ActiveSell,
                ]:
                all_pending_usd += pair.buy.usd
                if pair.algorithm == algorithm:
                    alg_pending_usd += pair.buy.usd

        # The percent allocation for this algorithm
        percent: float = Settings.allocations[algorithm]

        # The total amount of USD in the pot
        total_usd: float = wallet.usd_available + all_pending_usd
        # Check if there is any allocation left
        if ((total_usd * percent) - alg_pending_usd) < usd:
            return False

        return True
