import time
from datetime import datetime
from dateutil.relativedelta import relativedelta

from context import Context
from orders.order_book import OrderBook
from market.current import CurrentMarket
from algorithm.tranche import Tranche
from algorithm.version3 import check_tranche
from utils.logging import Log

tranches = [
    #Tranche(
    #    "UberLow",
    #    usd=5,
    #    spread=0.0005,
    #    qty=2,
    #),
    Tranche(
        "Low",
        usd=50,
        spread=0.003,
        qty=4,
    ),
    Tranche(
        "Mid",
        usd=100,
        spread=0.005,
        qty=4,
    ),
    Tranche(
        "High",
        usd=500,
        spread=0.006,
        qty=4,
    ),
]

# Process every 1 second
RETRY_SLEEP_SECONDS: float = 1.0

# Print market to console very minute
PRINT_FREQUENCY: relativedelta = relativedelta(minutes=1)

# New Coinbase connection context
ctx: Context = Context()

# Get the active orders
orderbook: OrderBook = OrderBook.read_fs("orderbook.json")

# Loop variables
churn_retries = 0
next_print: datetime = datetime.now()

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

    # Make sure our tranches look how we want
    for tranche in tranches:
        check_tranche(ctx, orderbook, tranche)

    # Save to persistent storage
    orderbook.write_fs("orderbook.json")

    # Wait for next run
    time.sleep(RETRY_SLEEP_SECONDS)
