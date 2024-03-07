from datetime import datetime

from context import Context
from settings import Settings

from algorithm.phase import Phase
from algorithm.limits import get_phased_wager
from algorithm.phase_tracker import PhaseTracker
from orders.order_book import OrderBook
from orders.factory import create_phased_pair
from orders.order import Order
from orders.order_pair import OrderPair
from market.current import CurrentMarket
from utils.logging import Log
from utils.maths import floor_percent, floor_seconds

def update_phases(ctx: Context, order_book: OrderBook, phases: PhaseTracker) -> None:
    # Get most recent phase
    phase: Phase = phases.current()

    # Get current order after restart
    if not phases.current_order:
        for order in order_book:
            if order.tranche == Settings.PHASED_TRANCHE_NAME:
                phases.current_order = order

    # Get current market value
    market: CurrentMarket | None = CurrentMarket.get(ctx)
    if market == None:
        return None
    assert market is not None

    # Update smooth market value tracking
    phases.smooth.update_market(market)

    # Tell phase to record current market value
    phase.add_point(phases.smooth.bid)

    # Score for most recent tail of frame
    tail = phase.get_score(Settings.TAIL_PERCENT * phase.get_score_frame_percent())

    # Calculate percent
    percent: float = 0.0
    if abs(phase.current_score - tail) > 1:
        percent = phase.current_score / (phase.current_score - tail)
    else:
        percent = 100

    # Print debug
    if False:
        print("")
        print("Phase: {}".format(phase.phase_type.name))
        print("Phase time: {}".format(floor_seconds(phase.phase_seconds())))
        print("Phase current: {}".format(
            floor_percent(phase.current_score),
        ))
        print("Phase tail: {}".format(floor_percent(tail)))
        print("Percent: {}".format(floor_percent(percent)))

    # Don't do anything within minute of phase
    if phase.phase_seconds() < Settings.INITIAL_FRAME_SECONDS:
        return None

    # Unknown: After 30 seconds of data determine what phase we are in
    if phase.phase_type in [Phase.Type.Unknown, Phase.Type.Plateau]:
        if tail >= Settings.GOOD_SCORE:
            Log.info("Waxing phase start at {} BTC.".format(market.bid))
            if phase.phase_type == Phase.Type.Plateau:
                phase = phases.new_phase(Phase.Type.Waxing)
            else:
                phase.phase_type = Phase.Type.Waxing
        elif phase.current_score <= Settings.BAD_SCORE or tail <= Settings.BAD_SCORE:
            Log.info("Waning phase start at {} BTC.".format(market.bid))
            if phase.phase_type == Phase.Type.Plateau:
                phase = phases.new_phase(Phase.Type.Waning)
            else:
                phase.phase_type = Phase.Type.Waning
        else:
            phase.phase_type = Phase.Type.Plateau

    # Waxing: Create buy if it doesn't exist
    if phase.phase_type == Phase.Type.Waxing and not phases.current_order:
        usd = get_phased_wager(ctx)
        phases.current_order = create_phased_pair(market, usd)
        order_book.orders.append(phases.current_order)
        phases.current_order.churn(ctx, market)
    # Waxing: Been waxing for 5 seconds and buy still pending, we'll requeue at new price
    elif phases.current_order and phases.current_order.buy and phases.current_order.buy.status in [Order.Status.Pending, Order.Status.Active]:
        if (datetime.now() - phases.current_order.buy.order_time).total_seconds() > 5:
            Log.info("Requeue waxing phase starting at {} BTC.".format(market.bid))
            if phases.current_order.buy.cancel(ctx, "Requeue start waxing"):
                phases.current_order = create_phased_pair(market, phases.current_order.buy.usd)
                order_book.orders.append(phases.current_order)
                phases.current_order.churn(ctx, market)

    # Waxing: No longer climbing, move to next phase
    if phase.phase_type == Phase.Type.Waxing and percent < Settings.TAPER_PERCENT:
        # Cancel pending buy
        if phases.current_order and phases.current_order.buy.status in [Order.Status.Pending, Order.Status.Active]:
            if phases.current_order.cancel(ctx, "Done waxing"):
                phases.current_order = None
                phase = phases.new_phase()
        # Move to next phase, we may be back
        else:
            phase = phases.new_phase()

    # Waning: Time to sell
    if phase.phase_type == Phase.Type.Waning and phases.current_order:
        # Create new sell order
        if phases.current_order and phases.current_order.buy.status == Order.Status.Complete and not phases.current_order.sell:
            Log.info("Waxing phase ending at {} BTC.".format(market.bid))
            btc = phases.current_order.buy.btc
            usd = market.bid * btc
            phases.current_order.sell = Order(Order.Type.Sell, btc, usd)
            phases.current_order.sell.maker_buffer = 5.0
            phases.current_order.churn(ctx, market)
        # Still waning after 5 seconds and not sold, requeue cheaper
        elif phases.current_order and phases.current_order.sell and phases.current_order.sell.status != Order.Status.Complete:
            if (datetime.now() - phases.current_order.sell.order_time).total_seconds() > 5:
                Log.info("Requeue waxing phase ending at {} BTC.".format(market.bid))
                phases.current_order.sell.cancel(ctx, "Requeue done waxing")
                btc = phases.current_order.buy.btc
                usd = market.bid * btc
                phases.current_order.sell = Order(Order.Type.Sell, btc, usd)
                phases.current_order.sell.maker_buffer = 5.0
                phases.current_order.churn(ctx, market)

    # No longer waning
    if phase.phase_type == Phase.Type.Waning and tail > 0:
        Log.info("Waning phase end at {} BTC.".format(market.bid))
        phase = phases.new_phase()

    # Done with current order
    if phases.current_order and phases.current_order.status in [Order.Status.Complete, Order.Status.Complete]:
        phases.current_order = None
