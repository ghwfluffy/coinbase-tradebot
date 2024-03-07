from datetime import datetime
from dateutil.relativedelta import relativedelta

from context import Context
from orders.order import Order
from orders.order_book import OrderBook
from orders.order_pair import OrderPair
from orders.factory import create_tranched_pair
from algorithm.tranche import Tranche
from market.current import CurrentMarket
from utils.logging import Log
from utils.maths import floor_btc

# 0.1% above market is our price for sales we want to sell at a discount
DISCOUNTED_SELL: float = 0.001

# How much more we value sells (less likely to cancel)
SELL_WEIGHT: float = 4

# How long a discount will make us pause making new buys
DISCOUNT_PAUSE = relativedelta(minutes=20)

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

    # Update smooth market calculation
    # We don't want to make buying and selling decisions based on rapid sudden unsustained market movements
    ctx.smooth.update_market(market)

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

    # If market is going down, we will pause trading for a while
    discount_mode: bool = False
    if tranche.last_discount and tranche.last_discount + DISCOUNT_PAUSE > datetime.now() and market and market.ask and tranche.discount_price and market.ask < tranche.discount_price:
        discount_mode = True

    # We need to cancel something to make a new bet
    # Or if the market is going down we will see if we should cut our losses on a sale
    if discount_mode or len(wagers) >= tranche.qty:
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
            Log.info("Cancel buy for tranche {}.".format(tranche.name))
            furthest_pair.cancel(ctx, "buy low")
        # Discount sell
        elif furthest_pair.status == OrderPair.Status.PendingSell:
            Log.info("Cancel sell for tranche {}.".format(tranche.name))
            sell: Order | None = furthest_pair.sell
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
                Log.info("Tranche {} entering discount sale mode for {} minutes.".format(tranche.name, DISCOUNT_PAUSE.minutes))

            return None
        else:
            #Log.debug("Furthest pair for tranche {} is in active state and can't cancel.".format(tranche.name))
            return None

    # If market is going down, pause buying for a few minutes
    if discount_mode:
        #Log.debug("New buys paused due to discounted sell state.")
        return None

    # Place a new order
    Log.info("Making new pair for tranche {}.".format(tranche.name))
    pair = create_tranched_pair(ctx.smooth, tranche)
    if pair:
        pair.churn(ctx, market)
        orderbook.orders.append(pair)
