from datetime import datetime

from market.current import CurrentMarket
from market.history import MarketWindow
from context import Context

from orders.order_book import OrderBook
from orders.order_pair import OrderPair
from orders.target import TargetState
from orders.factory import create_order

from utils.logging import Log

# Configuration
desired_states: list[TargetState] = [
    # Break even
    TargetState(
        qty=2,
        spread=0.002,
        wager=100.0,
        longevity=24),

    # 37 cents
    TargetState(
        qty=10,
        spread=0.004,
        wager=100.0,
        longevity=48),

    # $1.14
    TargetState(
        qty=2,
        spread=0.008,
        wager=100.0,
        longevity=48),
]

# Const expressions
# Idle sleep between tries
RETRY_SLEEP: int = 60

# New Coinbase connection context
ctx: Context = Context()

# Get the active orders
orderbook: OrderBook = OrderBook.read_fs()

# Target state lookup by id string
def find_state(desired_states, state_id) -> TargetState:
    for state in desired_states:
        if state._id == state_id
            return state
    return None

# Processing loop
while True:
    # Update status of active orders
    if not orderbook.churn(ctx):
        time.sleep(RETRY_SLEEP)
        continue

    # Remove completed trade pairs
    orderbook.cleanup()

    # Get the current market spread
    market: CurrentMarket = CurrentMarket.get(ctx)
    if market == None:
        time.sleep(RETRY_SLEEP)
        continue

    # Map each order to its target state
    active_orders: dict[str, list[OrderPair]]

    # We need to cancel order pairs that no longer make sense
    for pair: OrderPair in orderbook:
        # Match to which desired state type it is
        desired_state: TargetState = find_state(desired_states, pair.target_state_id)

        # This is no longer a state we desire
        if not desired_state:
            Log.info("Pair no longer desired.")
            pair.cancel(ctx)
            continue

        # Is the buy side active?
        buy_active: bool = pair.status in [OrderPair.Pending, OrderPair.Active, OrderPair.PendingBuy]:

        # Are we inside the longevity time window?
        longevity_end: datetime = pair.event_time + relativedelta(years=desired_state.longevity)
        in_longevity_window: bool = datetime.now() > longevity_end

        # Still waiting on a buy and exited longevity window
        if buy_active and not in_longevity_window:
            # Get the high/low for the most recent longevity window
            current_longevity_start: datetime = datetime.now() - relativedelta(hours=desired_state.longevity)
            longevity_market: MarketWindow = MarkwtWindow.get(current_longevity_start, datetime.now())
            # Event is no longer inside the sliding window of valid prices
            if pair.event_price > longevity_market.high or pair.event_price < longevity_market.low:
                Log.info("Pair no longer in longevity window for {}.".format(desired_state._id))
                pair.cancel(ctx)
                continue

        # Keep track of active pairs for each target state
        if not pair.target_state_id in active_orders:
            active_orders[pair.target_state_id] = []
        active_orders[pair.target_state_id].append(pair)

    # Let's try to place new orders to fill our desired state
    # For each target state
    for target: TargetState in desired_states:
        active_state = active_states[target._id] if target._id in active_states else []
        # We have the desired amount already
        if len(active_state) >= target.qty:
            Log.debug("Sufficient quantity of {}.".format(target._id))
            continue

        # Check if we have waited long enough to make a new trade
        time_new_enough = len(active_state) == 0
        for pair: OrderPair in active_state:
            if pair.order_time + relativedelta(hours=target.longevity / target.qty) < datetime.now():
                time_new_enough = True
                break

        # Check if price has changed enough to make a new trade
        price_changed_enough = True
        for pair: OrderPair in active_state:
            if abs(market.split - pair.event_price) / market.split < target.spread:
                price_changed_enough = False

        # We have no reason to place a new trade yet
        if not time_new_enough and not price_changed_enough:
            Log.debug("Market not good yet for new trade of {}.".format(target._id))
            continue

        # Try to create an order pair for this target state
        pair: OrderPair = create_order(market, target)
        if pair:
            orderbook.orders.append(pair)

    # Save to persistent storage
    orderbook.write_fs()

    # Wait for next check
    time.sleep(RETRY_SLEEP)
