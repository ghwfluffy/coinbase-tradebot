from datetime import datetime

from context import Context
from settings import Settings

from algorithm.phase import Phase
from algorithm.phase_tracker import PhaseTracker
from orders.order_book import OrderBook
from orders.factory import create_phased_pair
from orders.order import Order
from orders.order_pair import OrderPair
from market.current import CurrentMarket
from utils.logging import Log
from utils.maths import floor_percent, floor_seconds, floor_usd, floor

class Calculations:
    score: float
    tilt: float
    tail_score: float
    tail_tilt: float
    # Percent of profit compared to all time high of phase
    usd_percent: float
    # Highest percent change from buy
    market_movement: float

def do_calculations(phases: PhaseTracker, phase: Phase) -> Calculations:
    calc: Calculations = Calculations()
    calc.score = phase.get_score()
    calc.tilt = phase.get_avg_tilt()
    calc.tail_score = phase.get_score(Settings.TAIL_PERCENT)
    calc.tail_tilt = phase.get_tilt(Settings.TAIL_PERCENT)
    calc.usd_percent = 1.0
    calc.market_movement = 0.0

    # If we have an active order, calculate the percent of the maximum height we've reached
    if (phases.current_order and
        phases.current_order.status == OrderPair.Status.PendingSell
        ):
        if not phases.current_max or phases.smooth.split > phases.current_max:
            phases.current_max = phases.smooth.split
        base_price: float = phases.current_order.buy.usd / phases.current_order.buy.btc
        calc.usd_percent = ((phases.smooth.split - base_price) / (phases.current_max - base_price))
        calc.market_movement = (phases.current_max - base_price) / base_price

    return calc

def is_still_waxing(calc: Calculations) -> bool:
    return calc.score > 0
    #return calc.tilt > 0

def is_start_waxing(calc: Calculations) -> bool:
    return calc.score >= 1.0
    #return calc.tilt >= Settings.GOOD_TILT

def is_still_waning(calc: Calculations) -> bool:
    if calc.score < 0:
        return True
    return False

def is_start_waning(calc: Calculations) -> bool:
    # Going up
    if calc.score >= 0:
        return False
    # Taking a loss
    if calc.usd_percent < 0:
        return True
    # Bad tilt
    if calc.score <= -1.0:
        return True
    # Market has moved significantly and we've lost 20% of the max
    if calc.market_movement >= 0.004 and calc.usd_percent < 0.8: # TODO: Magic numbers
        return True
    return False

def buy_active(phases: PhaseTracker) -> bool:
    if (phases.current_order and
        phases.current_order.buy and
        phases.current_order.buy.status in [Order.Status.Pending, Order.Status.Active]
    ):
        return True
    return False

def sell_active(phases: PhaseTracker) -> bool:
    if (phases.current_order and
        phases.current_order.sell and
        phases.current_order.sell.status in [Order.Status.Pending, Order.Status.Active]):
        return True
    return False

def buy_longevity(phases: PhaseTracker) -> float:
    if not phases.current_order:
        return 0
    assert phases.current_order is not None

    if not phases.current_order.buy:
        return 0
    assert phases.current_order.buy is not None

    return (datetime.now() - phases.current_order.buy.order_time).total_seconds() > 5

def sell_longevity(phases: PhaseTracker) -> float:
    if not phases.current_order:
        return 0
    assert phases.current_order is not None

    if not phases.current_order.sell:
        return 0
    assert phases.current_order.sell is not None

    return (datetime.now() - phases.current_order.sell.order_time).total_seconds() > 5

def cancel_buy(ctx: Context, phases: PhaseTracker) -> bool:
    if not buy_active(phases):
        return False
    assert phases.current_order is not None

    return phases.current_order.buy.cancel(ctx, "Requeue phased buy")

def cancel_sell(ctx: Context, phases: PhaseTracker) -> bool:
    if not sell_active(phases):
        return False
    assert phases.current_order is not None
    assert phases.current_order.sell is not None

    if phases.current_order.sell.cancel(ctx, "Requeue phased sell"):
        phases.current_order.sell = None
        return True
    return False

def active_position(phases: PhaseTracker) -> bool:
    if phases.current_order and phases.current_order.status == OrderPair.Status.PendingSell:
        return True
    return False

def create_buy(
        ctx: Context,
        order_book:
        OrderBook,
        phases: PhaseTracker,
        market: CurrentMarket,
    ) -> bool:

    # Sell active don't need to buy
    if sell_active(phases):
        return False

    # Buy active, cancel it
    if buy_active(phases) and not cancel_buy(ctx, phases):
        return False

    # How much to wager?
    usd: float | None = ctx.get_phased_wager(ctx)
    if not usd:
        return False
    assert usd is not None

    # Queue
    phases.current_order = create_phased_pair(market, usd)
    phases.current_max = 0.0
    order_book.orders.append(phases.current_order)
    phases.current_order.churn(ctx, market)
    return True

def create_sell(
        ctx: Context,
        order_book:
        OrderBook,
        phases: PhaseTracker,
        market: CurrentMarket,
    ) -> bool:

    # Sell already active
    if sell_active(phases):
        return False

    # Never bought anything
    if not active_position(phases):
        return False
    assert phases.current_order is not None
    assert phases.current_order.buy is not None

    # Queue
    btc: float = phases.current_order.buy.btc
    usd: float = market.split * btc
    phases.current_order.sell = Order(Order.Type.Sell, btc, usd)
    phases.current_order.sell.maker_buffer = Settings.PHASED_MAKER_BUFFER
    phases.current_order.churn(ctx, market)
    return True

def print_debug(
        phases: PhaseTracker,
        market: CurrentMarket,
        phase: Phase,
        calc: Calculations,
    ):
    if False:
        print("")
        print("Market: {} | {}".format(floor_usd(phases.smooth.split), floor_usd(market.split)))
        print("Phase: {}".format(phase.phase_type.name))
        print("Status: {}".format(phases.current_order.status.name if phases.current_order else "None"))
        print("Phase time: {}".format(floor_seconds(phase.phase_seconds())))
        print("Score: {}".format(floor_percent(calc.score)))
        print("Tilt: {}".format(floor_percent(calc.tilt)))
        print("Tail Score: {}".format(floor_percent(calc.tail_score)))
        print("Tail Tilt: {}".format(floor_percent(calc.tail_tilt)))
        print("Of Max: {}".format(floor_percent(calc.usd_percent)))
        print("Movement: {}".format(calc.market_movement))
    elif True: # False:
        print("{:.2f}: {} : {:.6f} | {:.6f} (MM {:.6f})".format(
            floor_usd(phases.smooth.split),
            phase.phase_type.name,
            calc.score,
            calc.tilt,
            calc.market_movement,
        ))

    with open("phase_data-v5.csv", "a") as fp:
        fp.write("{},{},{},{},{},{},{},{},{},{},{},{}\n".format(
            datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f"),
            phase.phase_type.name,
            phases.current_order.status.name if phases.current_order else "None",
            floor_usd(market.split),
            floor_usd(phases.smooth.split),
            floor_seconds(phase.phase_seconds()),
            floor_percent(calc.score),
            floor_percent(calc.tilt),
            floor_percent(calc.tail_score),
            floor_percent(calc.tail_tilt),
            floor_percent(calc.usd_percent),
            floor(calc.market_movement, 6),
        ))


# Unknown
def phase_unknown(
        ctx: Context,
        order_book: OrderBook,
        phases: PhaseTracker,
        market: CurrentMarket,
        phase: Phase,
        calc: Calculations,
    ):

    # Are we in a new phase?
    if is_start_waxing(calc):
        Log.info("Waxing phase start at {} BTC.".format(market.split))
        phase.phase_type = Phase.Type.Waxing
    elif is_start_waning(calc):
        Log.info("Waning phase start at {} BTC.".format(market.split))
        phase.phase_type = Phase.Type.Waning
    else:
        phase.phase_type = Phase.Type.Plateau

# Plateau
def phase_plateau(
        ctx: Context,
        order_book: OrderBook,
        phases: PhaseTracker,
        market: CurrentMarket,
        phase: Phase,
        calc: Calculations,
    ):

    # Are we in a new phase?
    if is_start_waxing(calc):
        Log.info("Waxing phase start at {} BTC.".format(market.split))
        phases.new_phase(Phase.Type.Waxing)
    elif is_start_waning(calc):
        Log.info("Waning phase start at {} BTC.".format(market.split))
        phases.new_phase(Phase.Type.Waning)

# Waxing
def phase_waxing(
        ctx: Context,
        order_book: OrderBook,
        phases: PhaseTracker,
        market: CurrentMarket,
        phase: Phase,
        calc: Calculations,
    ):

    # Still waxing?
    if not is_still_waxing(calc):
        Log.info("Waxing phase ending.")
        # Cancel buy if it wasn't already filled
        cancel_buy(ctx, phases)
        phases.new_phase(Phase.Type.Plateau)
        return None

    # Create buy if it doesn't exist
    if not phases.current_order:
        create_buy(ctx, order_book, phases, market)

    # Been waxing for 5 seconds and buy still pending, we'll requeue at new price
    if buy_longevity(phases) > 5.0:
        Log.info("Requeue waxing phase starting at {} BTC.".format(market.split))
        if cancel_buy(ctx, phases):
            create_buy(ctx, order_book, phases, ctx)

# Waning
def phase_waning(
        ctx: Context,
        order_book: OrderBook,
        phases: PhaseTracker,
        market: CurrentMarket,
        phase: Phase,
        calc: Calculations,
    ):

    # Still waning?
    if not is_still_waning(calc):
        Log.info("Waning phase ending.")
        # Cancel sell if it wasn't already filled
        cancel_sell(ctx, phases)
        phases.new_phase(Phase.Type.Plateau)
        return None

    # Don't want to have a buy
    if buy_active(phases):
        cancel_buy(ctx, phases)

    # Time to sell
    if active_position(phases) and not sell_active(phases):
        Log.info("Waning phase selling at {} BTC.".format(market.split))
        create_sell(ctx, order_book, phases, market)

    # Been waning for 5 seconds and sell still active, we'll requeue at new price
    if sell_longevity(phases) > 5.0:
        Log.info("Rqueue waning phase at {} BTC.".format(market.split))
        if cancel_sell(ctx, phases):
            create_sell(ctx, order_book, phases, market)


def update_phases(ctx: Context, order_book: OrderBook, phases: PhaseTracker) -> None:
    # Get most recent phase
    phase: Phase = phases.current()

    # Get current order after restart
    if not phases.current_order:
        for order in order_book:
            if order.tranche == Settings.PHASED_TRANCHE_NAME:
                Log.debug("Recovered active phased order from previous run.")
                phases.current_order = order

    # Get current market value
    market: CurrentMarket | None = CurrentMarket.get(ctx)
    if market == None:
        return None
    assert market is not None

    # Update smooth market value tracking
    phases.smooth.update_market(market)

    # Tell phase to record current market value
    phase.add_point(phases.smooth.split)

    # Make calculations
    calc: Calculations = do_calculations(phases, phase)
    if True:
        print_debug(phases, market, phase, calc)

    # Don't do anything within minute of phase
    if phase.phase_seconds() < Settings.INITIAL_FRAME_SECONDS:
        return None

    # Run state machine
    {
        Phase.Type.Unknown: phase_unknown,
        Phase.Type.Plateau: phase_plateau,
        Phase.Type.Waxing: phase_waxing,
        Phase.Type.Waning: phase_waning,
    }[phase.phase_type](ctx, order_book, phases, market, phase, calc)

    # Done with current order
    if phases.current_order and phases.current_order.status in [OrderPair.Status.Complete, OrderPair.Status.Canceled]:
        phases.current_order = None
