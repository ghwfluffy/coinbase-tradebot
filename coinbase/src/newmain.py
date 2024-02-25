import time
from datetime import datetime
from dateutil.relativedelta import relativedelta

from context import Context
from market.current import CurrentMarket
from market.history import MarketWindow

from orders.order_book import OrderBook
from orders.order_pair import OrderPair
from orders.target import TargetState
from orders.factory import create_order

from utils.logging import Log
from utils.math import floor_usd
from utils.wallet import get_wallet, has_liquidity

# XXX: New Logic
# Every 5 minutes:
# If you have less than $50 available USD: find the lowest unfilled Active buy bid that is more than 1 hour old and more than 0.15% away from market and cancel it
# If still have less than $50 available USD: find the highest sell bid that is more than 6 hours old and more than 1% away from market and canel it and move it down to 0.05% above market price (reset age)
# If you have $50 USD
#   Create a new $50 order pair with 0.3% spread

# Const expressions
# Idle sleep between tries
RETRY_SLEEP: int = 1

# How much to bet
WAGER_SIZE: float = 250.0
# How often (minutes) to try and place a new trade
TRADE_FREQUENCY: float = 5.0 #XXX

# Fail safe don't ever have a wallet value less than this
MIN_WALLET: float = 2000.0

# 0.3% difference is a good spread
GOOD_SPREAD: float = 0.003

# 1.0% above market value is a bad sale
BAD_SALE_DELTA: float = 0.01
# 6 hours is a long time for a sale
SELL_EXPIRATION: float = 6.0
# 0.1% above market is our price for sales we want to sell at a discount
DISCOUNTED_SELL: float = 0.001
LONGEVITY: float = SELL_EXPIRATION
# Don't do more than 1 discount every 2 minutes
DISCOUNT_MIN_IDLE: float = 2.0

# If the buy is alive longer than an hour it's less likely to happen
BUY_EXPIRATION: float = 1.0
# More than .15% below market value is a bad buy
BAD_BUY_DELTA: float = GOOD_SPREAD / 2

# New Coinbase connection context
ctx: Context = Context()

# Get the active orders
orderbook: OrderBook = OrderBook.read_fs("orderbook-v2.json")

# Loop variables
churn_retries = 0
next_print: datetime = datetime.now()
next_bid: datetime = datetime.now()

# Clear pending orders from a previous run
#orderbook.clear_pending(ctx)

# Don't rebid too fast on restart
if len(orderbook.orders) > 0:
    last_order = orderbook.orders[-1]
    next_bid = last_order.event_time + relativedelta(minutes=TRADE_FREQUENCY)

def cancel_worst_buy(ctx: Context, orderbook: OrderBook, market: CurrentMarket) -> bool:
    Log.debug("Try to cancel worst buy.")
    # Find all Active buys that are more than BUY_EXPIRATION old
    buys: list[Order] = []
    for order in orderbook.orders:
        if order.status != OrderPair.Status.Active:
            continue
        if (datetime.now() - order.event_time).total_seconds() < (BUY_EXPIRATION * 60):
            continue
        buys.append(order.buy)
    # Sort them by bid ascending
    buys.sort(key=lambda order:order.final_market)
    if len(buys) == 0:
        return False

    # If the top one is more than BAD_BUY_DELTA away from market, cancel it and return True (return None on failed cancel)
    buy = buys[0]
    # This can't happen right?
    if market.bid < buy.get_price():
        Log.error("Our buy is above market bid?")
        return False
    # Worst buy is within acceptable delta
    if ((market.bid - buy.get_price()) / market.bid) < BAD_BUY_DELTA:
        return False

    # Cancel bid
    if not buy.cancel(ctx):
        return None # Failed to cancel

    # Canceled
    Log.info("Canceled bad buy.")
    return True

def discount_worst_sell(ctx: Context, orderbook: OrderBook, market: CurrentMarket) -> bool:
    Log.debug("Try to discount worst sale.")

    # Find all PendingSell sells that are more than SELL_EXPIRATION old
    sells: list[Order] = []
    for order in orderbook.orders:
        if order.status != OrderPair.Status.PendingSell:
            continue
        if (datetime.now() - order.sell.order_time).total_seconds() < (SELL_EXPIRATION * 60):
            continue
        sells.append(order.sell)
    # Sort them by bid descending
    sells.sort(key=lambda order:order.get_price() * -1)
    if len(sells) == 0:
        return False

    # If the top one is more than BAD_SALE_DELTA away from market, cancel the current sale, and make a new sale that is DISCOUNTED_SALE above market and return True
    sell = sells[0]
    # This can't happen right?
    if market.ask > sell.get_price():
        Log.error("Our sell is below market ask?")
        return False
    # Worst sell is within acceptable delta
    if ((market.ask - sell.get_price()) / market.ask) < BAD_SALE_DELTA:
        return False

    # Cancel current sell
    if not sell.cancel(ctx):
        return None # Failed to cancel

    # Requeue at DISCOUNTED_SALE rate
    sell.usd = (market.bid + (market.bid * DISCOUNTED_SALE)) * sell.btc
    sell.status = Order.Status.Pending
    sell.order_time = datetime.now()

    # Requeued
    Log.info("Discounted bad sale.")
    return True

while True:
    # Update status of active orders
    if not orderbook.churn(ctx):
        churn_retries += 1
        if churn_retries >= 10:
            time.sleep(RETRY_SLEEP)
            continue
    churn_retries = 0

    # Fail safe that wallet isn't getting drained due to a bug
    wallet: dict = get_wallet(ctx)
    if wallet and wallet['all']['total'] < MIN_WALLET:
        Log.error("Minimum wallet requirements not met: {}/{}.".format(wallet['all']['total'], MIN_WALLET))
        exit(0)

    # Remove completed trade pairs
    orderbook.cleanup(ctx)

    # Get the current market spread
    market: CurrentMarket = CurrentMarket.get(ctx)
    if market == None:
        time.sleep(RETRY_SLEEP)
        continue

    # Every minute print the market data
    if next_print <= datetime.now():
        Log.debug("Market: ${:.2f} (${:.2f}) ${:.2f}".format(market.bid, market.split, market.ask))
        next_print = datetime.now() + relativedelta(minutes=1)

    # Time to make next bid
    if next_bid <= datetime.now():
        Log.debug("Time for new trade.")

        # Our coffers are empty, try to cleanup unlikely buys
        all_buys_canceled = False
        while has_liquidity(ctx, WAGER_SIZE) == False:
            ret = cancel_worst_buy(ctx, orderbook, market)
            if ret == None:
                break
            if ret == False:
                all_buys_canceled = True
                break

        # Our coffers are still empty, see if we can sell something for cheaper than we were hoping
        if all_buys_canceled and has_liquidity(ctx, WAGER_SIZE) == False:
            discount_worst_sell(ctx, orderbook, market)
            # Don't discount more often than once every couple minutes
            next_bid = datetime.now() + relativedelta(minutes=DISCOUNT_MIN_IDLE)

        # We have money to make a new wager
        if has_liquidity(ctx, WAGER_SIZE):
            target: TargetState = TargetState(qty=1, spread=GOOD_SPREAD, wager=WAGER_SIZE, longevity=LONGEVITY)
            pair: OrderPair = create_order(market, target)
            if pair:
                pair.churn(ctx, market)
                orderbook.orders.append(pair)
                # Make a new bid every 5 minutes
                next_bid = datetime.now() + relativedelta(minutes=TRADE_FREQUENCY)

    # Save to persistent storage
    orderbook.write_fs("orderbook-v2.json")

    # Wait for next check
    time.sleep(RETRY_SLEEP)
