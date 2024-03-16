import json
from datetime import datetime
from dateutil.relativedelta import relativedelta

from context import Context
from settings import Settings

from orders.order import Order
from orders.order_book import OrderBook
from orders.order_pair import OrderPair

from market.current import CurrentMarket
from utils.maths import floor_usd
from utils.logging import Log

class HodlHistory():
    last_hodl: datetime | None = None
    current_hodl: OrderPair | None = None

def create_buy(ctx: Context, orderbook: OrderBook, hodl_history: HodlHistory) -> None:
    market: CurrentMarket | None = CurrentMarket.get(ctx)
    if not market:
        return None

    buy_at: float = floor_usd(market.ask)
    num_bitcoins: float = Settings.HODL_BTC
    usd: float = floor_usd(buy_at * num_bitcoins)

    buy: Order = Order(Order.Type.Buy, num_bitcoins, usd)
    buy.tranched = False
    buy.maker_buffer = Settings.PHASED_MAKER_BUFFER
    sell: Order | None = None
    Log.debug("Triggering HODL purchase at ${:.2f} ({:.8f} BTC: ${:.2f}).".format(
        market.ask, num_bitcoins, usd))
    order: OrderPair = OrderPair("HODL", market.ask, buy, sell)
    orderbook.orders.append(order)
    hodl_history.current_hodl = order
    hodl_history.last_hodl = datetime.now()

def check_hodl(ctx: Context, orderbook: OrderBook, hodl_history: HodlHistory) -> None:
    # Initialize the last hold position
    # Check orderbook
    if not hodl_history.last_hodl:
        for order in orderbook:
            if order.tranche == "HODL":
                hodl_history.current_hodl = order
                hodl_history.last_hodl = order.event_time
                break
    # Check history
    if not hodl_history.last_hodl:
        with open("historical.json", "r") as fp:
            orders = json.loads(fp.read())
        for order in reversed(orders):
            if order['tranche'] == "HODL":
                hodl_history.current_hodl = None
                hodl_history.last_hodl = datetime.strptime(order['event_time'], "%Y-%m-%d %H:%M:%S")
                break

    # Check active buy
    if hodl_history.current_hodl:
        # No sell in HODL, mark as complete
        if hodl_history.current_hodl.status == OrderPair.Status.PendingSell:
            hodl_history.current_hodl.status = OrderPair.Status.Complete
            Log.info("HODL purchase complete.")

        # Stop tracking current hodl
        if hodl_history.current_hodl.status in [
                OrderPair.Status.PendingSell,
                OrderPair.Status.Complete,
                OrderPair.Status.Canceled]:
            hodl_history.current_hodl = None
        # Requeue at better price if it's been queued too long
        elif (hodl_history.current_hodl.event_time + relativedelta(minutes=2)) < datetime.now():
            if hodl_history.current_hodl.buy.cancel(ctx, "Requeue HODL buy"):
                create_buy(ctx, orderbook, hodl_history)

    # Time for next hold position buy
    elif not hodl_history.last_hodl or (
            datetime.now() >= (hodl_history.last_hodl + relativedelta(minutes=Settings.HODL_FREQUENCY_MINUTES))):
        create_buy(ctx, orderbook, hodl_history)
