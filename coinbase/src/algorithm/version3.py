from datetime import datetime
from dateutil.relativedelta import relativedelta

from context import Context
from settings import Settings
from orders.order import Order
from orders.order_book import OrderBook
from orders.order_pair import OrderPair
from orders.factory import create_tranched_pair
from algorithm.tranche import Tranche
from algorithm.phase import Phase
from market.current import CurrentMarket
from utils.logging import Log
from utils.maths import floor_btc

from typing import List

# 0.1% above market is our price for sales we want to sell at a discount
DISCOUNTED_SELL: float = 0.001

# How much more we value sells (less likely to cancel)
SELL_WEIGHT: float = 4

# How long a discount will make us pause making new buys
DISCOUNT_PAUSE = relativedelta(hours=2)

def requeue(ctx: Context, tranche: Tranche, pair: OrderPair) -> None:
    # Check sell has been active at least a minute before requeue
    if not pair.sell or not pair.sell.order_time or (pair.sell.order_time + relativedelta(minutes=1)) > datetime.now():
        return None
    Log.info("Cancel sell for tranche {}.".format(tranche.name))
    sell: Order | None = pair.sell
    # Cancel current sell
    if sell and sell.cancel(ctx, "sell high"):
        # Requeue at DISCOUNTED_SALE rate
        before = sell.usd
        after = (ctx.smooth.bid + (ctx.smooth.bid * DISCOUNTED_SELL)) * sell.btc
        Log.info("Requeue discounted sale (${:.2f} -> ${:.2f}).".format(before, after))
        sell.usd = after
        sell.status = Order.Status.Pending
        sell.order_time = datetime.now()
        tranche.last_discount = datetime.now()
        tranche.discount_price = before
        Log.info("Tranche {} entering discount sale mode.".format(tranche.name))

def clear_wagers(ctx: Context, tranche: Tranche, wagers: List[OrderPair]) -> None:
    for pair in wagers:
        if pair.status == OrderPair.Status.PendingSell:
            Log.info("Discounting sale for tranche {}.".format(tranche.name))
            requeue(ctx, tranche, pair)
        elif pair.status in [OrderPair.Status.Pending, OrderPair.Status.Active]:
            Log.info("Cancel buy for tranche {}.".format(tranche.name))
            pair.cancel(ctx, "discount cancels buy")


def check_tranche(ctx: Context, orderbook: OrderBook, tranche: Tranche) -> None:
    # Get wagers associated with this tranche
    wagers = []
    for pair in orderbook:
        if pair.tranche == tranche.name:
            wagers.append(pair)

    # Get current market value
    market: CurrentMarket | None = CurrentMarket.get(ctx)
    if market == None:
        return None
    assert market is not None

    # Update smooth market calculation
    # We don't want to make buying and selling decisions based on rapid sudden unsustained market movements
    ctx.smooth.update_market(market)

    phase: Phase.Type = Phase.Type.Unknown
    # XXX: Use our own phase tracking
    if True: #False:
        # Tell phase to record current market value
        if not ctx.tranche_phase:
            ctx.tranche_phase = Phase(frame_size=30)
        ctx.tranche_phase.add_point(ctx.smooth.split)
        # Don't in effect have a memory leak
        while len(ctx.tranche_phase.points) > 500:
            ctx.tranche_phase.points.pop(0)
            ctx.tranche_phase.scores.pop(0)

        # Calculate the current phase
        # This phase is more discrete than the phased algorithm phase
        if ctx.tranche_phase.phase_seconds() < Settings.INITIAL_FRAME_SECONDS:
            phase = Phase.Type.Unknown
        elif ctx.tranche_phase.get_score() < -0.2:
            phase = Phase.Type.Waning
        elif ctx.tranche_phase.get_score() < 0.2:
            phase = Phase.Type.Plateau
        else:
            phase = Phase.Type.Waxing

        print("                              {}: {}".format(phase.name, ctx.tranche_phase.get_score()))
    # XXX: Set phase to phased algorithm phase
    elif len(ctx.phases.phases) > 0:
        phase = ctx.phases.phases[-1].phase_type

    # Enable buys that are on hold if the curve is going up
    # If the curve is going down, we're not going to buy stuff
    if len(ctx.phases.phases) > 0:
        for pair in wagers:
            # Going up, enable buying stuff
            if phase in [Phase.Type.Waxing, Phase.Type.Plateau]:
                # Too far away to care
                if (pair.buy.get_limit_price() - ctx.smooth.bid) > 100.0:
                    continue
                # We will catch the up swing and buy
                if pair.status == OrderPair.Status.OnHold:
                    pair.buy.status = Order.Status.Pending
                    pair.buy.maker_buffer = 20.0
            # Going down, put stuff on hold
            elif phase == Phase.Type.Waning:
                if pair.status == OrderPair.Status.Pending:
                    pair.buy.status = Order.Status.OnHold
                elif pair.status == OrderPair.Status.Active and pair.buy.cancel(ctx, "waning active"):
                    pair.buy.status = Order.Status.OnHold

            # Pause selling if we're going up
            if pair.status == OrderPair.Status.PendingSell:
                # Going up: pause
                #if phase == Phase.Type.Waxing:
                #    if pair.sell.status == Order.Status.Pending:
                #        pair.sell.status = Order.Status.OnHold
                # Going down: resume
                #else:
                #    if pair.sell.status == Order.Status.OnHold:
                #        pair.sell.status = Order.Status.Pending
                # XXX: Sell when we hit our target
                if pair.sell.status == Order.Status.OnHold:
                    pair.sell.status = Order.Status.Pending

    # If we fall significantly below a buy price we will get out of our positions
    discount_mode: bool = False
    for pair in wagers:
        if pair.status != OrderPair.Status.PendingSell:
            continue
        if (pair.buy.final_market - 50.0) > ctx.smooth.split:
            Log.info("Fell below buy price on tranche {}.".format(tranche.name))
            requeue(ctx, tranche, pair)
            discount_mode = True

    # If market is going down, we will pause trading for a while
    if tranche.last_discount and tranche.last_discount + DISCOUNT_PAUSE > datetime.now() and market and market.ask and tranche.discount_price and market.ask < tranche.discount_price:
        discount_mode = True

    # If market is going down, pause buying for a while
    if discount_mode:
        clear_wagers(ctx, tranche, wagers)
        return None

    # Find the closest wager to the current market price
    diff_spread: float
    min_spread: float | None = None
    for pair in wagers:
        diff_spread = abs((ctx.smooth.bid - pair.event_price) / ctx.smooth.bid)
        if not min_spread or diff_spread < min_spread:
            min_spread = diff_spread

    # Are we far enough away to make a new bet?
    if min_spread and min_spread < tranche.spread:
        #Log.debug("Min spread {:.8f} for tranche {} too low.".format(floor_btc(min_spread), tranche.name))
        return None

    # We need to cancel something to make a new bet
    # Or if the market is going down we will see if we should cut our losses on a sale
    if len(wagers) >= tranche.qty:
        max_spread: float | None = None
        furthest_pair: OrderPair | None = None
        for pair in wagers:
            diff_spread = abs((ctx.smooth.bid - pair.event_price) / ctx.smooth.bid)
            # Pending sells can be 4x as far
            if pair.status == OrderPair.Status.PendingSell:
                diff_spread = diff_spread / SELL_WEIGHT
            if not max_spread or diff_spread > max_spread:
                max_spread = diff_spread
                furthest_pair = pair

        if not furthest_pair or not max_spread or max_spread <= tranche.spread:
            #Log.debug("No valid orders to cancel for tranch {}.".format(tranche.name))
            return None

        # Cancel buy
        # If market is going down though we will keep our buys
        if not discount_mode and furthest_pair.status in [OrderPair.Status.Active, OrderPair.Status.Pending]:
            Log.info("Cancel low buy for tranche {}.".format(tranche.name))
            furthest_pair.cancel(ctx, "buy low")
        elif not discount_mode and furthest_pair.status in [OrderPair.Status.OnHold]:
            Log.info("Cancel high buy for tranche {}.".format(tranche.name))
            furthest_pair.cancel(ctx, "buy high")
        # Discount sell
        elif furthest_pair.status == OrderPair.Status.PendingSell:
            Log.info("Entering discount mode for tranche {}.".format(tranche.name))
            discount_mode = True
            return None
        else:
            #Log.debug("Furthest pair for tranche {} is in active state and can't cancel.".format(tranche.name))
            return None

    # Place a new order
    Log.info("Making new pair for tranche {}.".format(tranche.name))
    pair = create_tranched_pair(ctx.smooth, tranche)
    if pair:
        pair.churn(ctx, market)
        orderbook.orders.append(pair)
