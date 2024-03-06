from datetime import datetime

from context import Context
from algorithm.phase import Phase
from algorithm.phase_tracker import PhaseTracker
from orders.order_book import OrderBook
from orders.factory import create_phased_pair
from orders.order import Order
from orders.order_pair import OrderPair
from market.current import CurrentMarket
from utils.logging import Log

def update_phases(ctx: Context, order_book: OrderBook, phases: PhaseTracker) -> None:
    # Get most recent phase
    phase: Phase = phases.current()

    # Get current market value
    market: CurrentMarket = CurrentMarket.get(ctx)
    if market == None:
        return None

    # Update smooth market value tracking
    phases.smooth.update_market(market)

    # Tell phase to record current market value
    phase.add_point(phases.smooth.bid)

    print("")
    print("Phase: {}".format(phase.phase.name))
    print("Phase time: {}".format(phase.phase_seconds()))
    print("Phase minmax: ({} | {})".format(phase.min_score, phase.max_score))
    print("Phase current: {}".format(
        phase.current_score,
    ))
    tail = phase.get_score_tail_seconds(120)
    print("Phase tail: {}".format(tail))

    # Calculate percent
    #max_percent = 0
    #if phase.max_score > 0:
    #    max_percent = (phase.current_score / phase.max_score) * 100.0
    #min_percent = 0
    #if phase.min_score < 0:
    #    min_percent = (phase.current_score / phase.min_score) * 100.0
    #print("Percents: ({} | {})".format(min_percent, max_percent))

    # Don't do anything within minute of phase
    if phase.phase_seconds() < 60:
        return None

    # Unknown: After 30 seconds of data determine what phase we are in
    if phase.phase in [Phase.Type.Unknown, Phase.Type.Plateau]:
        if phase.current_score > 20 and tail > 20:
            Log.info("Waxing phase start at {} BTC.".format(market.bid))
            if phase.phase == Phase.Type.Plateau:
                phase = phases.new_phase(Phase.Type.Waxing)
            else:
                phase.phase = Phase.Type.Waxing
        elif phase.current_score < -20 or tail < -20:
            Log.info("Waning phase start at {} BTC.".format(market.bid))
            if phase.phase == Phase.Type.Plateau:
                phase = phases.new_phase(Phase.Type.Waning)
            else:
                phase.phase = Phase.Type.Waning
        else:
            phase.phase = Phase.Type.Plateau

    usd = 20.0 # TODO: Percent of wallet by settings
    # Waxing: Create buy if it doesn't exist
    if phase.phase == Phase.Type.Waxing and not phases.current_order:
        phases.current_order = create_phased_pair(market, usd)
        order_book.orders.append(phases.current_order)
        phases.current_order.churn(ctx, market)
    elif phases.current_order and phases.current_order.buy and phases.current_order.buy.status in [Order.Status.Pending, Order.Status.Active]:
        if (datetime.now() - phases.current_order.buy.order_time).total_seconds() > 5:
            Log.info("Requeue waxing phase starting at {} BTC.".format(market.bid))
            if phases.current_order.buy.cancel(ctx, "Requeue start waxing"):
                phases.current_order = create_phased_pair(market, usd)
                order_book.orders.append(phases.current_order)
                phases.current_order.churn(ctx, market)

    # Waxing: No longer climbing, time to sell
    if phase.phase == Phase.Type.Waxing and tail < 5:
        if phases.current_order and phases.current_order.buy.status in [Order.Status.Pending, Order.Status.Active]:
            if phases.current_order.cancel(ctx, "Done waxing"):
                phases.current_order = None
        if phases.current_order and phases.current_order.buy.status == Order.Status.Complete:
            Log.info("Waxing phase ending at {} BTC.".format(market.bid))
            btc = phases.current_order.buy.btc
            usd = market.bid * btc
            phases.current_order.sell = Order(Order.Type.Sell, btc, usd)
            phases.current_order.sell.maker_buffer = 5.0
            phases.current_order.churn(ctx, market)

        elif phases.current_order and phases.current_order.sell and phases.current_order.sell.status != Order.Status.Complete:
            if (datetime.now() - phases.current_order.sell.order_time).total_seconds() > 5:
                Log.info("Requeue waxing phase ending at {} BTC.".format(market.bid))
                phases.current_order.sell.cancel(ctx, "Requeue done waxing")
                btc = phases.current_order.buy.btc
                usd = market.bid * btc
                phases.current_order.sell = Order(Order.Type.Sell, btc, usd)
                phases.current_order.sell.maker_buffer = 5.0
                phases.current_order.churn(ctx, market)

        # Reset phase
        if not phases.current_order or phases.current_order.status in [OrderPair.Status.Complete, OrderPair.Status.Canceled]:
            Log.info("Waxing phase ended.")
            phases.new_phase()

    if phase.phase == Phase.Type.Waning and tail > -10:
        Log.info("Waning phase end at {} BTC.".format(market.bid))
        phases.new_phase()
