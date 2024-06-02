from enum import Enum
from datetime import datetime
from dateutil.relativedelta import relativedelta

from typing import Dict, List

from gtb.core.thread import BotThread
from gtb.core.context import Context
from gtb.core.settings import Settings, Spread
from gtb.orders.order import Order, OrderInfo
from gtb.orders.order_pair import OrderPair
from gtb.orders.cancel import cancel_order
from gtb.phases.phase import Phase
from gtb.utils.logging import Log
from gtb.utils.maths import floor_btc, floor_usd, ceil_usd

# One big bet
class AllInTrader(BotThread):
    ALGORITHM: str = "AllIn"

    active_pair: OrderPair | None
    market_top: float
    ready_to_sell: bool
    last_requeue: datetime

    def __init__(self, ctx: Context) -> None:
        super().__init__(ctx)
        self.active_pair = None
        self.market_top = 0
        self.ready_to_sell = False
        self.last_requeue = datetime.now()

    def init(self) -> None:
        # Find active trade
        for pair in self.ctx.order_book.order_pairs:
            if pair.algorithm.startswith(AllInTrader.ALGORITHM):
                Log.info("Found active AllIn order pair.")
                self.active_pair = pair
                break

    def think(self) -> None:
        if not self.active_pair:
            self.check_bet()
        else:
            with self.active_pair.mtx:
                self.check_position()

    def check_bet(self) -> None:
        assert self.active_pair is None

        # Wait a couple minutes between bets
        if (datetime.now() - self.last_requeue).total_seconds() < 120:
            return None

        if self.market_low():
            market: float = self.ctx.current_market.bid
            self.market_top = market
            self.ready_to_sell = False
            self.last_requeue = datetime.now()
            Log.info("AllIn new wager at ${:.2f}.".format(market))

            # Wager
            buy_usd: float = 100
            sell_market: float = market * 1.0014
            btc: float = floor_btc(buy_usd / market)
            sell_usd: float = sell_market * btc

            # Buy order
            buy: Order = Order(Order.Type.Buy, btc, buy_usd)
            buy.status = Order.Status.Pending

            # Sell order
            sell: Order = Order(Order.Type.Sell, btc, sell_usd)
            sell.status = Order.Status.OnHold

            # Order pair
            self.active_pair = OrderPair(AllInTrader.ALGORITHM, buy, sell)
            self.active_pair.event_price = market

            # Queue
            self.ctx.order_book.append(self.active_pair)

    def market_low(self) -> bool:
        # Going down
        if self.ctx.phases.short == Phase.Waning and self.ctx.phases.acute == Phase.Waning:
            return False
        if self.ctx.phases.mid == Phase.Waning:
            return False
        if self.ctx.phases.long == Phase.Waning:
            return False

        # Fell down a bit
        top_price: float = self.ctx.market_top.price * 0.98
        if self.ctx.current_market.split < top_price and self.ctx.phases.short == Phase.Plateau:
            return True

        # Going up
        if self.ctx.phases.short == Phase.Waxing and self.ctx.phases.acute == Phase.Waxing:
            return True

        # Indeterminate
        return False

    def check_position(self) -> None:
        assert self.active_pair is not None

        if self.active_pair.status == OrderPair.Status.Complete:
            Log.info("AllIn trade has been completed.")
            self.ctx.notify.queue("AllIn trade has been completed.")
            self.active_pair = None
            self.last_requeue = datetime.now()
        elif self.active_pair.status == OrderPair.Status.Canceled:
            Log.info("AllIn trade has been canceled.")
            self.ctx.notify.queue("AllIn trade has been canceled.")
            self.active_pair = None
        elif self.active_pair.status in [OrderPair.Status.Pending, OrderPair.Status.Active]:
            self.check_buy_position()
        elif self.active_pair.status in [OrderPair.Status.OnHoldSell, OrderPair.Status.PendingSell, OrderPair.Status.ActiveSell]:
            self.check_sell_position()
        else:
            Log.info("AllIn trade invalid state.")
            cancel_order(self.ctx, AllInTrader.ALGORITHM, self.active_pair.buy, "invalid state")
            if self.active_pair.sell:
                cancel_order(self.ctx, AllInTrader.ALGORITHM, self.active_pair.sell, "invalid state")

    def check_buy_position(self) -> None:
        assert self.active_pair is not None

        # Don't touch new bets for at least 20 minutes
        if (datetime.now() - self.active_pair.event_time).total_seconds() < 1200:
            return None

        # Pending bet too high for current market
        if (self.ctx.smooth_market.split - 200) > self.active_pair.buy.get_limit_price():
            Log.info("AllIn buy too cheap.")
            cancel_order(self.ctx, AllInTrader.ALGORITHM, self.active_pair.buy, "market went up")

    def check_sell_position(self) -> None:
        assert self.active_pair is not None
        assert self.active_pair.sell is not None
        assert self.active_pair.buy.info is not None
        assert self.active_pair.buy.info.final_time is not None
        assert self.active_pair.buy.info.final_market is not None

        sell: Order = self.active_pair.sell

        # Track top of the market for this trade
        if self.market_top < self.active_pair.buy.info.final_market:
            self.market_top = self.active_pair.buy.info.final_market
        if self.market_top < self.ctx.smooth_market.bid:
            self.market_top = self.ctx.smooth_market.bid

        # Don't touch new bets for at least 20 minutes
        if (datetime.now() - self.active_pair.buy.info.final_time.replace(tzinfo=None)).total_seconds() < 1200:
            return None
        # Requeue every 15 seconds get out of a bad position
        if (datetime.now() - self.last_requeue).total_seconds() < 15:
            return None

        # Fall below purchase price? ($50 leeway)
        if self.ctx.smooth_market.bid < (self.active_pair.buy.info.final_market - 50):
            self.last_requeue = datetime.now()
            Log.info("AllIn fell below purchace price.")
            if cancel_order(self.ctx, AllInTrader.ALGORITHM, sell, "in the red"):
                # Requeue at market price
                self.last_requeue = datetime.now()
                market: float = self.ctx.current_market.bid
                usd: float = market * self.active_pair.buy.btc
                sell.status = Order.Status.Pending
                sell.usd = usd
            return None

        # If we go above .1% we will trigger into sell mode
        if not self.ready_to_sell:
            delta: float = self.ctx.smooth_market.bid - self.active_pair.buy.info.final_market
            delta_percent: float = delta / self.active_pair.buy.info.final_market
            if delta_percent >= 0.1:
                Log.info("AllIn is ready to consider selling.")
                self.ready_to_sell = True

        going_up: bool = self.ctx.phases.short == Phase.Waxing and self.ctx.phases.acute == Phase.Waxing

        # We are trying to sell but the market is going back up again
        if sell.status in [Order.Status.Pending, Order.Status.Active] and going_up:
            Log.info("AllIn stop sell we're going back up.")
            if cancel_order(self.ctx, AllInTrader.ALGORITHM, sell, "market going up"):
                sell.status = Order.Status.OnHold
            return None

        # Are we going to sell now?
        if sell.status == Order.Status.OnHold and not going_up:
            # If we have gone down 15% of the max we went up, we'll start selling
            run_height: float = self.market_top - self.active_pair.buy.info.final_market
            cur_height: float = self.ctx.smooth_market.bid - self.active_pair.buy.info.final_market
            percent_of_max: float = cur_height / run_height
            if percent_of_max < 0.85:
                self.last_requeue = datetime.now()
                Log.info("AllIn wants to exit position.")
                sell.status = Order.Status.Pending
