import time
from datetime import datetime
from dateutil.relativedelta import relativedelta

from market.current import CurrentMarket
from market.history import MarketWindow
from context import Context

from orders.order_book import OrderBook
from orders.order_pair import OrderPair
from orders.target import TargetState
from orders.factory import create_order

from utils.logging import Log
from utils.math import floor_usd

# Configuration
desired_states: list[TargetState] = [
    # Break even
    TargetState(
        qty=2,
        spread=0.002,
        wager=20.0,
        longevity=24),

    # 37 cents
    TargetState(
        qty=10,
        spread=0.004,
        wager=21.0,
        longevity=48),

    # $1.14
    TargetState(
        qty=2,
        spread=0.008,
        wager=22.0,
        longevity=48),
]
# XXX: INITIAL TEST
desired_states: list[TargetState] = [
    TargetState(
        qty=4,
        spread=0.002,
        wager=50.0,
        longevity=1),
]

# Const expressions
# Idle sleep between tries
RETRY_SLEEP: int = 2

# New Coinbase connection context
ctx: Context = Context()

# Get the active orders
orderbook: OrderBook = OrderBook.read_fs()

# Target state lookup by id string
def find_state(desired_states, state_id) -> TargetState:
    for state in desired_states:
        if state._id == state_id:
            return state
    return None

def is_buy_active(pair: OrderPair) -> bool:
    return pair.status in [
                OrderPair.Status.Pending,
                OrderPair.Status.Active,
            ]

# Processing loop
while True:
    # Update status of active orders
    if not orderbook.churn(ctx):
        time.sleep(RETRY_SLEEP)
        continue

    # Remove completed trade pairs
    orderbook.cleanup(ctx)

    # Get the current market spread
    market: CurrentMarket = CurrentMarket.get(ctx)
    if market == None:
        time.sleep(RETRY_SLEEP)
        continue

    Log.debug("Market: ${:.2f} (${:.2f}) ${:.2f}".format(market.bid, market.split, market.ask))

    # Map each order to its target state
    active_orders: dict[str, list[OrderPair]] = {}

    # We need to cancel order pairs that no longer make sense
    for pair in orderbook:
        # Match to which desired state type it is
        desired_state: TargetState = find_state(desired_states, pair.target_id)

        # This is no longer a state we desire
        if not desired_state:
            Log.info("Pair no longer desired.")
            pair.cancel(ctx)
            continue

        # Is the buy side active?
        buy_active: bool = is_buy_active(pair)

        # Are we inside the longevity time window?
        longevity_end: datetime = pair.event_time + relativedelta(hours=desired_state.longevity)
        in_longevity_window: bool = datetime.now() < longevity_end

        # Still waiting on a buy and exited longevity window
        if buy_active and not in_longevity_window:
            # Get the high/low for the most recent longevity window
            current_longevity_start: datetime = datetime.now() - relativedelta(hours=desired_state.longevity)
            longevity_market: MarketWindow = MarketWindow.get(ctx, current_longevity_start, datetime.now())
            # Event is no longer inside the sliding window of valid prices
            if pair.event_price > longevity_market.high or pair.event_price < longevity_market.low:
                Log.info("Pair no longer in longevity window for {}.".format(desired_state._id))
                Log.debug("Low: {}, High: {}, Event: {}".format(longevity_market.low, longevity_market.high, pair.event_price))
                pair.cancel(ctx)
                continue

        # Keep track of active pairs for each target state
        if not pair.target_id in active_orders:
            active_orders[pair.target_id] = []
        active_orders[pair.target_id].append(pair)

    # Let's try to place new orders to fill our desired state
    # For each target state
    for target in desired_states:
        active_state = active_orders[target._id] if target._id in active_orders else []
        # We have the desired amount already
        if len(active_state) >= target.qty:
            Log.debug("Sufficient quantity of {}.".format(target._id))
            continue

        next_time = None
        next_price_up = None
        next_price_down = None

        has_active = False
        time_new_enough = True
        for pair in active_state:
            if not is_buy_active(pair):
                continue
            has_active = True
            # Check if we have waited long enough to make a new trade
            # How long from the event we must be
            pair_next_time: datetime = pair.event_time + relativedelta(hours=target.longevity / target.qty)
            if not next_time or pair_next_time > next_time:
                next_time = pair_next_time
            if datetime.now() < pair_next_time:
                time_new_enough = False
            # Check if price has changed enough to make a new trade
            pair_next_price_up = pair.event_price * (1 + target.spread)
            pair_next_price_down = pair.event_price * (1 - target.spread)
            if not next_price_up or next_price_up < pair_next_price_up:
                next_price_up = pair_next_price_up
            if not next_price_down or next_price_down < pair_next_price_down:
                next_price_down = pair_next_price_down

        price_changed_enough = False
        if has_active and (market.split < next_price_down or market.split > next_price_up):
            price_changed_enough = True

        # We have no reason to place a new trade yet
        if has_active and not time_new_enough and not price_changed_enough:
            Log.debug("Market not good yet for new trade of {} (Next time: {}, Next Price: ${:.2f} - ${:.2f}).".format(
                target._id,
                next_time.strftime("%Y-%m-%d %H:%M:%S"),
                floor_usd(next_price_down),
                floor_usd(next_price_up),
            ))
            continue

        # Try to create an order pair for this target state
        pair: OrderPair = create_order(market, target)
        if pair:
            pair.churn(ctx)
            orderbook.orders.append(pair)

    # Save to persistent storage
    orderbook.write_fs()

    # Wait for next check
    time.sleep(RETRY_SLEEP)
