import time
from datetime import datetime
from dateutil.relativedelta import relativedelta

from context import Context
from orders.order_book import OrderBook
from market.current import CurrentMarket
from algorithm.tranche import Tranche
from algorithm.version3 import check_tranche
from utils.wallet import get_wallet
from utils.logging import Log

Log.debug("Started")

tranches = [
    #Tranche(
    #    "UberLow",
    #    usd=5,
    #    spread=0.0005,
    #    qty=2,
    #),
    Tranche(
        "Low-50",
        usd=50,
        spread=0.003,
        qty=8,
    ),
    Tranche(
        "Low-100",
        usd=100,
        spread=0.004,
        qty=4,
    ),
    Tranche(
        "Mid-100",
        usd=100,
        spread=0.005,
        qty=4,
    ),
    Tranche(
        "High-500",
        usd=500,
        spread=0.006,
        qty=4,
    ),
]

tranches = [
    Tranche(
        "Low",
        usd=800,
        spread=0.003,
        qty=3,
    ),
    Tranche(
        "Mid",
        usd=50,
        spread=0.0035,
        qty=4,
    ),
    Tranche(
        "High",
        usd=40,
        spread=0.004,
        qty=3,
    ),
]

# Fail safe wallet amount
MIN_WALLET: float = 1000.0

# Process every 1 second
RETRY_SLEEP_SECONDS: float = 0.1

# Print market to console very minute
PRINT_FREQUENCY: relativedelta = relativedelta(minutes=5)

# New Coinbase connection context
ctx: Context = Context()

# Get the active orders
orderbook: OrderBook = OrderBook.read_fs("orderbook.json")

# Loop variables
churn_retries = 0
next_print: datetime = datetime.now()

#orderbook.clear_pending(ctx)

# Main loop
while True:
    # Update status of active orders
    if not orderbook.churn(ctx):
        time.sleep(RETRY_SLEEP_SECONDS)
        continue

    # Every minute print the market data
    if next_print <= datetime.now():
        # Get the current market spread
        market: CurrentMarket = CurrentMarket.get(ctx)
        if market == None:
            time.sleep(RETRY_SLEEP_SECONDS)
            continue

        Log.debug("Market: ${:.2f} (${:.2f}) ${:.2f}".format(market.bid, market.split, market.ask))
        next_print = datetime.now() + PRINT_FREQUENCY

    # Fail safe that wallet isn't getting drained due to a bug
    wallet: dict = get_wallet(ctx)
    if wallet and wallet['all']['total'] < MIN_WALLET:
        Log.error("Minimum wallet requirements not met: {}/{}.".format(wallet['all']['total'], MIN_WALLET))
        exit(0)

    # Make sure our tranches look how we want
    for tranche in tranches:
        check_tranche(ctx, orderbook, tranche)

    # Save to persistent storage
    orderbook.write_fs("orderbook.json")

    # Wait for next run
    time.sleep(RETRY_SLEEP_SECONDS)
