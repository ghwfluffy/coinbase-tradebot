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

# Pick buy and sell points above/below current market
class SpreadTrader(BotThread):
    ALGORITHM: str = "Spread"

    class State(Enum):
        Init = 0
        Steady = 1
        Cautious = 2
        Optimistic = 3
        Waning = 4

    current_spreads: Dict[str, List[OrderPair]]
    state: State
    state_census: List[State]
    last_state_change: datetime

    def __init__(self, ctx: Context) -> None:
        super().__init__(ctx)

        self.current_spreads = {}
        self.state = SpreadTrader.State.Init
        self.state_census = []
        self.last_state_change = datetime.now()

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
        # See if market has changed
        self.update_state()

        # For each spread
        for spread in Settings.spreads:
            self.handle_spread(spread)

    def update_state(self) -> None:
        # Pick a state
        state: SpreadTrader.State = self.pick_state()

        # Keep track of most recent 20 picks
        self.state_census.append(state)
        while len(self.state_census) > 90:
            self.state_census.pop(0)

        # Don't change too often
        # Not enough time has passed to declare a state change
        if self.state != SpreadTrader.State.Init and self.last_state_change + relativedelta(minutes=1) > datetime.now():
            return None

        # Take a consensus
        agreement: bool = True
        for c in self.state_census:
            if c != self.state_census[0]:
                agreement = False
                break

        # State changed
        if agreement and self.state != state:
            self.state = state
            self.last_state_change = datetime.now()

            if self.state == SpreadTrader.State.Waning:
                Log.info("Market is going down.")
                self.ctx.notify.queue(f"Market is going down (${self.ctx.smooth_market.split:.2f})")

            elif self.state == SpreadTrader.State.Optimistic:
                Log.info("Market is optimistic")
                self.ctx.notify.queue(f"Market is optimistic (${self.ctx.smooth_market.split:.2f})")

            elif self.state == SpreadTrader.State.Cautious:
                Log.info("Market is cautious")
                self.ctx.notify.queue(f"Market is cautious (${self.ctx.smooth_market.split:.2f})")
            else:
                Log.info("Market steady")
                self.ctx.notify.queue(f"Market is steady (${self.ctx.smooth_market.split:.2f})")

    def pick_state(self) -> 'SpreadTrader.State':
        # Market is going down
        overall_waning: bool = self.ctx.phases.extended == Phase.Waning or self.ctx.phases.long == Phase.Waning
        now_waning: bool = self.ctx.phases.mid == Phase.Waning
        # Waning overall and current trend is waning
        if overall_waning and (now_waning or self.state == SpreadTrader.State.Waning):
            return SpreadTrader.State.Waning

        # Overall steady, but currently going up
        elif self.ctx.phases.extended == Phase.Plateau and self.ctx.phases.long == Phase.Waxing:
            return SpreadTrader.State.Optimistic

        # Overall steady, but currently going down
        elif self.ctx.phases.extended == Phase.Plateau and self.ctx.phases.long == Phase.Waning:
            return SpreadTrader.State.Cautious

        # Steady
        else:
            return SpreadTrader.State.Steady

    def handle_spread(self, spread_info: Spread) -> None:
        current_orders: List[OrderPair] = self.current_spreads[spread_info.name]

        # Waning: Just cut all our losses now before they get bigger
        if self.state == SpreadTrader.State.Waning:
            self.cut_losses(spread_info)
        else:
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
        current_market: float = self.ctx.smooth_market.bid

        bad_state: bool = False
        for pair in self.current_spreads[spread_info.name]:
            with pair.mtx:
                # The price i intend to buy at
                limit_price: float = pair.buy.get_limit_price()
                buy_ago: float = 0
                # The price i actually bought at
                if pair.buy.status == Order.Status.Complete:
                    assert pair.buy.info is not None
                    assert pair.buy.info.final_market is not None
                    assert pair.buy.info.final_time is not None
                    limit_price = pair.buy.info.final_market
                    buy_ago = (datetime.now() - pair.buy.info.final_time.replace(tzinfo=None)).total_seconds()

                # $200 leeway over the course of the first minute
                max_leeway: float = 200
                leeway_window: float = 60
                leeway: float = max_leeway
                if buy_ago > leeway_window:
                    leeway = 0
                else:
                    leeway = max_leeway * (1 - (buy_ago / leeway_window))

                # Buy price less than market price: skip
                if limit_price < (current_market + leeway):
                    #Log.debug(f"{limit_price} < {current_market} + {leeway}")
                    continue

                self.set_undesirable(
                    spread_info,
                    pair,
                    f"below threshold ({limit_price} vs {current_market})",
                )

                # Fell below buy margin
                if pair.buy.status == Order.Status.Complete:
                    bad_state = True

        # Move to cautious state if going down
        if bad_state:
            if self.state != SpreadTrader.State.Waning:
                self.state = SpreadTrader.State.Cautious
            self.last_state_change = datetime.now()

    def handle_new_spread(self, spread_info: Spread) -> None:
        # Maths
        market: float = self.ctx.smooth_market.split
        spread_usd: float = market * (spread_info.spread * (1 + (spread_info.spread * 1.5)))
        suffix: str
        buy_mode: Order.Status
        if self.ctx.phases.short != Phase.Waning:
            return None

        suffix = "G"
        buy_mode = Order.Status.OnHold
        buy_market = floor_usd(market - (spread_usd // 2))
        sell_market = floor_usd(market + (spread_usd // 2))
        btc: float = floor_btc(spread_info.usd / buy_market)
        buy_usd: float = spread_info.usd
        sell_usd: float = sell_market * btc

        #buy_fee: float = buy_usd * 0.015 # XXX
        #sell_usd = ceil_usd(sell_usd + buy_fee)
        #sell_market = ceil_usd(sell_usd / btc)

        # Sell is too close to market top
        #top_price: float = self.ctx.market_top.price
        #top_price: float = self.ctx.market_top.price * 0.982
        #if sell_market > top_price:
            #Log.debug("Spread ({:0.2f}|{:0.2f}) too low {:0.2f}".format(
            #    buy_market,
            #    sell_market,
            #    top_price));
            #return None

        # Does the buy auto complete or do we want to wait?
        if self.ctx.phases.short == Phase.Waxing:
            buy_mode = Order.Status.Pending
        elif self.ctx.phases.mid == Phase.Waxing:
            buy_mode = Order.Status.Pending
        elif self.ctx.phases.long == Phase.Waxing:
            buy_mode = Order.Status.Pending
        elif self.ctx.phases.extended == Phase.Waxing:
            buy_mode = Order.Status.Pending
        elif (self.ctx.phases.short == Phase.Plateau or
                self.ctx.phases.mid == Phase.Plateau or
                self.ctx.phases.long == Phase.Plateau or
                self.ctx.phases.extended == Phase.Plateau):
            buy_mode = Order.Status.Pending
        else:
            buy_mode = Order.Status.OnHold

        # Find the closest other pair
        closest_delta: float = spread_info.spread * 10
        for pair in self.current_spreads[spread_info.name]:
            buy_delta: float = abs(pair.buy.get_limit_price() - buy_market) / self.ctx.smooth_market.split
            if buy_delta < closest_delta:
                closest_delta = buy_delta

        # How frequently are we making bets right now
        min_delta: float
        if self.state == SpreadTrader.State.Steady:
            min_delta = spread_info.spread / 2
        elif self.state == SpreadTrader.State.Optimistic:
            min_delta = spread_info.spread / 4
        else: # Cautious
            min_delta = spread_info.spread

        # Too close
        if closest_delta < min_delta:
            return None

        Log.info("Market price {:.2f} triggering new {}-{} spread (${:.2f} USD).".format(
            self.ctx.smooth_market.split,
            spread_info.name,
            suffix,
            spread_info.usd,
        ))

        Log.debug("Spread: {:.2f}, BTC: {:.8f} Buy: ${:.2f}, Sell: ${:.2f}".format(
            spread_usd,
            btc,
            buy_usd,
            sell_usd))

        # Buy order
        buy: Order = Order(Order.Type.Buy, btc, buy_usd)
        buy.status = buy_mode

        # Sell order
        sell: Order = Order(Order.Type.Sell, btc, sell_usd)
        sell.status = Order.Status.Pending

        new_pair: OrderPair = OrderPair(SpreadTrader.ALGORITHM + "-" + spread_info.name + "-" + suffix, buy, sell)
        new_pair.event_price = market

        # Queue order
        self.ctx.order_book.append(new_pair)
        self.current_spreads[spread_info.name].append(new_pair)

    # Wait for most recent change to be upwards before allowing a buy to be filled
    # This stops orders being filled during the start of a downards run
    def wait_for_waxing(self, spread_info: Spread) -> None:
        # Waxing right now?
        #acute_waxing: bool = self.ctx.phases.acute != Phase.Waxing
        #mid_waxing: bool = self.ctx.phases.short == Phase.Waxing and self.ctx.phases.mid == Phase.Waxing
        #if not acute_waxing and not mid_waxing:
        #    return None
        #if self.ctx.phases.mid == Phase.Waning:
        #    return None
        if self.ctx.phases.short != Phase.Waxing:
            return None

        for pair in self.current_spreads[spread_info.name]:
            with pair.mtx:
                if pair.status != OrderPair.Status.OnHold:
                    continue
                # And price is below what we want to buy at
                if pair.buy.get_limit_price() < self.ctx.smooth_market.bid:
                    #Log.debug("XXX: Not low yet: {:.2f} vs {:.2f}".format(
                    #    pair.buy.get_limit_price(), self.ctx.smooth_market.bid))
                    continue
                delta: float = pair.buy.get_limit_price() - self.ctx.smooth_market.bid
                #Log.debug("XXX: delta: {:.2f}".format(delta))
                if delta < 60.0:
                    pair.buy.status = Order.Status.Pending
                    Log.debug("Spread {} buy is ready. {:.2f} USD away.".format(
                        spread_info.name,
                        delta,
                    ))

    # Put some stuff back on hold if we enter the cautious state
    def back_to_hold(self, spread_info: Spread) -> None:
        #if self.state != SpreadTrader.State.Cautious:
        #    return None

        for pair in self.current_spreads[spread_info.name]:
            with pair.mtx:
                delta: float = abs(pair.buy.get_limit_price() - self.ctx.smooth_market.bid)
                if delta < 200.0:
                    continue
                if pair.status == OrderPair.Status.Pending:
                    pair.buy.status = Order.Status.OnHold
                    Log.debug("Spread {} buy being paused. {:.2f} USD away.".format(
                        spread_info.name,
                        delta,
                    ))
                elif pair.status == OrderPair.Status.Active:
                    Log.info(f"Hold stale buy spread {spread_info.name} (${pair.buy.usd:.2f} USD).")
                    if cancel_order(self.ctx, SpreadTrader.ALGORITHM, pair.buy, "stale"):
                        pair.buy.status = Order.Status.OnHold

    # Cancel all buys, sell all positions at a loss
    def cut_losses(self, spread_info: Spread) -> None:
        # If we're not in an overall downward trend, we'll take our chances and hold our positions
        if self.ctx.phases.trend != Phase.Waning:
            return None
        for pair in self.current_spreads[spread_info.name]:
            with pair.mtx:
                self.set_undesirable(spread_info, pair, "market falling")

    def set_undesirable(self, spread_info: Spread, pair: OrderPair, reason: str) -> None:
        # Cancel buy
        if pair.status in [
            OrderPair.Status.Pending,
            OrderPair.Status.Active,
        ]:
            Log.info(f"Cancel buy spread {spread_info.name} {reason} (${pair.buy.usd:.2f} USD).")
            cancel_order(self.ctx, SpreadTrader.ALGORITHM, pair.buy, "below spread range")
            return None

        # Don't cancel sells
        return None

        # Do nothing if we just requeued this within the last couple minutes
        if pair.sell and pair.sell.info and datetime.now() < pair.sell.info.order_time + relativedelta(minutes=2):
            return None

        # Requeue cheaper sell
        if pair.sell and pair.status in [
            OrderPair.Status.OnHoldSell,
            OrderPair.Status.PendingSell,
            OrderPair.Status.ActiveSell,
        ]:
            # Cancel
            Log.info(f"Discount sell spread {spread_info.name} {reason} (${pair.buy.usd:.2f} USD).")
            self.ctx.notify.queue(f"Market fall discounting ${pair.buy.usd:.2f} sell.")
            if cancel_order(self.ctx, SpreadTrader.ALGORITHM, pair.sell, "below spread range"):
                # Requeue at market price
                pair.sell.status = Order.Status.Pending
                pair.sell.usd = self.ctx.smooth_market.bid * pair.sell.btc
                pair.sell.info = OrderInfo("", "", datetime.now(), self.ctx.smooth_market.bid)
