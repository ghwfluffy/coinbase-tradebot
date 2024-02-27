from context import Context
from orders.order_book import OrderBook
from orders.order_pair import OrderPair
from orders.factory import create_tranched_pair
from algorithm.tranche import Tranche
from market.current import CurrentMarket
from utils.logging import Log
from utils.math import floor_btc

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

        # Cancel this wager
        Log.info("Cancel pair for tranche {}.".format(tranche.name))
        furthest_pair.cancel(ctx)

    # Place a new order
    Log.info("Making new pair for tranche {}.".format(tranche.name))
    pair = create_tranched_pair(market, tranche)
    if pair:
        pair.churn(ctx, market)
        orderbook.orders.append(pair)
