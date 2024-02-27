from context import Context
from orders.order_book import OrderBook
from orders.order_pair import OrderPair
from orders.factory import create_tranched_pair
from algorithm.tranche import Tranche
from market.current import CurrentMarket
from utils.logging import Log
from utils.math import floor_btc

# 0.1% above market is our price for sales we want to sell at a discount
DISCOUNTED_SELL: float = 0.001

def check_tranche(ctx: Context, orderbook: OrderBook, tranche: Tranche) -> None:
    # Get wagers associated with this tranche
    wagers = []
    for pair in orderbook:
        if pair.tranche == tranche.name:
            wagers.append(pair)

    # Get current market value
    market: CurrentMarket = CurrentMarket.get(ctx)
    if market == None:
        return None

    # Find the closest wager to the current market price
    min_spread: float = None
    for pair in wagers:
        diff_spread: float = abs((market.bid - pair.event_price) / market.bid)
        if not min_spread or diff_spread < min_spread:
            min_spread = diff_spread

    # Are we far enough away to make a new bet?
    if min_spread and min_spread < (tranche.spread / 2):
        #Log.debug("Min spread {:.8f} for tranche {} too low.".format(floor_btc(min_spread), tranche.name))
        return None

    # We need to cancel something to make a new bet
    if len(wagers) >= tranche.qty:
        max_spread: float = None
        furthest_pair: OrderPair = None
        for pair in wagers:
            diff_spread: float = abs((market.bid - pair.event_price) / market.bid)
            # Pending sells can be 4x as far
            if pair.status == OrderPair.Status.PendingSell:
                diff_spread = diff_spread / 4
            if not max_spread or diff_spread > max_spread:
                max_spread = diff_spread
                furthest_pair = pair

        if max_spread <= tranche.spread:
            Log.debug("No valid orders to cancel for tranch {}.".format(tranche.name))
            return None

        # Cancel buy
        if pair.status in [OrderPair.Status.Active, OrderPair.Status.Pending]:
            Log.info("Cancel buy for tranche {}.".format(tranche.name))
            furthest_pair.cancel(ctx)
        # Discount sell
        elif pair.status == OrderPair.Status.PendingSell:
            Log.info("Cancel sell for tranche {}.".format(tranche.name))
            sell: Order = pair.sell
            # Cancel current sell
            if sell.cancel(ctx):
                # Requeue at DISCOUNTED_SALE rate
                before = sell.usd
                after = (market.bid + (market.bid * DISCOUNTED_SELL)) * sell.btc
                Log.info("Requeu discounted sale (${:.2f} -> ${:.2f}).".format(before, after))
                sell.usd = after
                sell.status = Order.Status.Pending
                sell.order_time = datetime.now()

            return None

    # Place a new order
    Log.info("Making new pair for tranche {}.".format(tranche.name))
    pair = create_tranched_pair(market, tranche)
    if pair:
        pair.churn(ctx, market)
        orderbook.orders.append(pair)
