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
from gtb.utils.maths import floor_btc, floor_usd

# Pick buy and sell points above/below current market
class SpreadTrader(BotThread):
    ALGORITHM: str = "Spread"

    current_spreads: Dict[str, List[OrderPair]]

    def __init__(self, ctx: Context) -> None:
        super().__init__(ctx)

        self.current_spreads = {}

    def init(self) -> None:
        # Make empty order lists
        for spread in Settings.spreads:
            self.current_spreads[spread.name] = []

        # Find all active spread trades
        for pair in self.ctx.order_book.order_pairs:
            if not pair.algorithm.startswith(SpreadTrader.ALGORITHM):
                continue

            spread_name: str = pair.algorithm.split('-')[1]

            if not spread_name in self.current_spreads:
                self.current_spreads[spread_name] = []
            self.current_spreads[spread_name].append(pair)

        for spread_name in self.current_spreads:
            Log.info("Found {} active {} spreads.".format(
                len(self.current_spreads[spread_name]),
                spread_name,
            ))

    def think(self) -> None:
        # For each spread
        for spread in Settings.spreads:
            self.handle_spread(spread)

    def handle_spread(self, spread_info: Spread) -> None:
        current_orders: List[OrderPair] = self.current_spreads[spread_info.name]

        # Take a loss on spreads we're likely not going to recover from
        self.end_bad_positions(spread_info)

        # Make new spread as long as the market isn't in a huge decline
        self.handle_new_spread(spread_info)

        # New spreads are put OnHold by default, so we buy on the way up not the way down
        self.wait_for_waxing(spread_info)

        # Cleanup spreads that are complete or too far away from current market
        self.cleanup_spread(spread_info)

    def cleanup_spread(self, spread_info: Spread) -> None:
        # Calculate what "too far" means
        max_delta: float = (spread_info.spread * 3) * self.ctx.smooth_market.split
        min_buy: float = self.ctx.smooth_market.split - max_delta
        max_buy: float = self.ctx.smooth_market.split + max_delta

        # Cancel far pairs
        pair: OrderPair
        for pair in self.current_spreads[spread_info.name]:
            with pair.mtx:
                # Only ones we haven't bought yet
                if not pair.status in [
                    OrderPair.Status.OnHold,
                    OrderPair.Status.Pending,
                    OrderPair.Status.Active,
                ]:
                    continue

                # Within valid range?
                limit_price: float = pair.buy.get_limit_price()
                if limit_price <= max_buy and limit_price >= min_buy:
                    continue

                # Cancel
                cancel_order(self.ctx, SpreadTrader.ALGORITHM, pair.buy, "spread too far")

        # Cleanup complete pairs
        index: int = 0
        while index < len(self.current_spreads[spread_info.name]):
            pair = self.current_spreads[spread_info.name][index]
            if pair.status in [
                OrderPair.Status.Complete,
                OrderPair.Status.Canceled,
            ]:
                self.current_spreads[spread_info.name].pop(index)
            else:
                index += 1

    def end_bad_positions(self, spread_info: Spread) -> None:
        # Calculate what "too much loss" is
        max_delta: float = spread_info.spread * self.ctx.smooth_market.split
        max_market: float = self.ctx.smooth_market.split + max_delta
        for pair in self.current_spreads[spread_info.name]:
            with pair.mtx:
                # The price i intend to buy at
                limit_price: float = pair.buy.get_limit_price()
                # The price i actually bought at
                if pair.buy.status == Order.Status.Complete:
                    assert pair.buy.info is not None
                    assert pair.buy.info.final_market is not None
                    limit_price = pair.buy.info.final_market

                # Within valid range?
                if limit_price < max_market:
                    continue

                # Cancel buy
                if pair.status in [
                    OrderPair.Status.Pending,
                    OrderPair.Status.Active,
                ]:
                    cancel_order(self.ctx, SpreadTrader.ALGORITHM, pair.buy, "below spread range")

                # Requeue cheaper sell
                elif pair.sell and pair.status in [
                    OrderPair.Status.OnHoldSell,
                    OrderPair.Status.PendingSell,
                    OrderPair.Status.ActiveSell,
                ]:
                    # Unless we just queued this within the last couple minutes
                    if pair.sell.info and datetime.now() < pair.sell.info.order_time + relativedelta(minutes=2):
                        continue
                    # Cancel
                    if cancel_order(self.ctx, SpreadTrader.ALGORITHM, pair.sell, "below spread range"):
                        # Requeue at market price
                        pair.sell.status = Order.Status.Pending
                        pair.sell.usd = self.ctx.smooth_market.bid * pair.sell.btc
                        pair.sell.info = OrderInfo("", "", datetime.now(), self.ctx.smooth_market.bid)

    def handle_new_spread(self, spread_info: Spread) -> None:
        # Market is in an extended decline, no new wagers
        if self.ctx.phases.extended == Phase.Waning:
            return None
        #if self.ctx.phases.extended == Phase.Plateau and self.ctx.phases.long == Phase.Waning:
        #    return None

        # Find the closest other pair
        min_delta: float = spread_info.spread * 10
        for pair in self.current_spreads[spread_info.name]:
            assert pair.sell is not None
            buy_delta: float = abs(pair.buy.get_limit_price() - self.ctx.smooth_market.split) / self.ctx.smooth_market.split
            sell_delta: float = abs(pair.sell.get_limit_price() - self.ctx.smooth_market.split) / self.ctx.smooth_market.split
            if buy_delta < min_delta:
                min_delta = buy_delta
            if sell_delta < min_delta:
                min_delta = sell_delta

        # Too close
        if min_delta < spread_info.spread / 2:
            return None

        Log.info("Market price {} triggering new {} spread (${:.2f} USD).".format(
            self.ctx.smooth_market.split,
            spread_info.name,
            spread_info.usd,
        ))

        # Maths
        spread_usd: float = self.ctx.smooth_market.split * spread_info.spread
        buy_market: float = floor_usd(self.ctx.smooth_market.split - (spread_usd / 2))
        sell_market: float = floor_usd(self.ctx.smooth_market.split + (spread_usd / 2))
        btc: float = floor_btc(spread_info.usd / buy_market)
        buy_usd: float = buy_market * btc
        sell_usd: float = sell_market * btc

        Log.debug("Spread: {:.2f}, BTC: {:.2f} Buy: ${:.2f}, Sell: ${:.2f}".format(
            spread_usd,
            btc,
            buy_usd,
            sell_usd))

        # Buy order
        buy: Order = Order(Order.Type.Buy, btc, buy_usd)
        buy.status = Order.Status.OnHold

        # Sell order
        sell: Order = Order(Order.Type.Sell, btc, sell_usd)
        sell.status = Order.Status.Pending

        new_pair: OrderPair = OrderPair(SpreadTrader.ALGORITHM + "-" + spread_info.name, buy, sell)

        # Queue order
        self.ctx.order_book.append(new_pair)
        self.current_spreads[spread_info.name].append(new_pair)

    def wait_for_waxing(self, spread_info: Spread) -> None:
        # Extended waning thread, don't make any buys
        if self.ctx.phases.extended == Phase.Waning:
            return None
        # Wait for most recent change to be upwards
        if self.ctx.phases.acute != Phase.Waxing:
            return None

        for pair in self.current_spreads[spread_info.name]:
            with pair.mtx:
                if pair.status != OrderPair.Status.OnHold:
                    continue
                # And price is below what we want to buy at
                if pair.buy.get_limit_price() < self.ctx.smooth_market.bid:
                    continue
                pair.buy.status = Order.Status.Pending
                delta: float = pair.buy.get_limit_price() - self.ctx.smooth_market.bid
                Log.debug("Spread {} buy is ready. {:.2f} USD away.".format(
                    spread_info.name,
                    delta,
                ))
