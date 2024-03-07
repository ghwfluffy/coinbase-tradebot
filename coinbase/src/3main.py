import time
from datetime import datetime

from context import Context
from settings import Settings
from orders.order_book import OrderBook
from market.current import CurrentMarket
from algorithm.version3 import check_tranche
from algorithm.phased import update_phases
from algorithm.phase_tracker import PhaseTracker
from utils.wallet import get_wallet
from utils.logging import Log

Log.debug("Started")

# New Coinbase connection context
ctx: Context = Context()

# Get the active orders
orderbook: OrderBook = OrderBook.read_fs("orderbook.json")
phases: PhaseTracker = PhaseTracker.read_fs()

# Loop variables
churn_retries = 0
next_print: datetime = datetime.now()

ctx.tranches = Settings.TRANCHES

#orderbook.clear_pending(ctx)

# Main loop
while True:
    # Update status of active orders
    if not orderbook.churn(ctx):
        time.sleep(Settings.RETRY_SLEEP_SECONDS)
        continue

    # Every minute print the market data
    if next_print <= datetime.now():
        # Get the current market spread
        market: CurrentMarket = CurrentMarket.get(ctx)
        if market == None:
            time.sleep(Settings.RETRY_SLEEP_SECONDS)
            continue

        Log.debug("Market: ${:.2f} (${:.2f}) ${:.2f}".format(market.bid, market.split, market.ask))

        ctx.smooth.update_market(market)
        Log.debug("Smooth: ${:.2f} (${:.2f}) ${:.2f}".format(ctx.smooth.bid, ctx.smooth.split, ctx.smooth.ask))

        next_print = datetime.now() + Settings.PRINT_FREQUENCY

    # Fail safe that wallet isn't getting drained due to a bug
    wallet: dict = get_wallet(ctx)
    if wallet and wallet['all']['total'] < Settings.MIN_WALLET:
        Log.error("Minimum wallet requirements not met: {}/{}.".format(wallet['all']['total'], Settings.MIN_WALLET))
        exit(0)

    # Make sure our tranches look how we want
    for tranche in ctx.tranches:
        check_tranche(ctx, orderbook, tranche)

    # Update our phase tracking and phased wager
    #update_phases(ctx, orderbook, phases)

    # Save to persistent storage
    orderbook.write_fs("orderbook.json")
    phases.write_fs()

    # Wait for next run
    time.sleep(Settings.RETRY_SLEEP_SECONDS)
