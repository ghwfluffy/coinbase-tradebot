from datetime import datetime
from dateutil.relativedelta import relativedelta

from gtb.core.thread import BotThread
from gtb.core.context import Context
from gtb.core.settings import Settings
from gtb.orders.order import Order
from gtb.orders.order_pair import OrderPair
from gtb.utils.logging import Log

# Periodically buys bitcoins and does not sell
class DiamondHands(BotThread):
    ALGORITHM: str = "HODL"

    last_buy: datetime | None
    next_buy: datetime

    def __init__(self, ctx: Context) -> None:
        super().__init__(ctx)
        self.last_buy = None
        self.next_buy = datetime.now()

    def init(self) -> None:
        # Find most recent HODL: Order history
        for pair in self.ctx.history.order_pairs:
            if pair.status != OrderPair.Status.Complete:
                continue
            if pair.algorithm != DiamondHands.ALGORITHM:
                continue
            if self.last_buy and pair.event_time < self.last_buy:
                continue
            self.last_buy = pair.event_time

        # Find most recent HODL: Active orders
        for pair in self.ctx.order_book.order_pairs:
            if pair.status == OrderPair.Status.Canceled:
                continue
            if pair.algorithm != DiamondHands.ALGORITHM:
                continue
            if self.last_buy and pair.event_time < self.last_buy:
                continue
            self.last_buy = pair.event_time

        # Set next buy time
        if self.last_buy:
            self.next_buy = self.last_buy + relativedelta(minutes=Settings.HODL_FREQUENCY_MINUTES)

    def think(self) -> None:
        if datetime.now() < self.next_buy:
            return None

        # Create order
        Log.info("Creating new HODL order.")
        btc: float = Settings.HODL_BTC
        usd: float = self.ctx.smooth_market.split * btc
        order: Order = Order(Order.Type.Buy, btc, usd)
        pair: OrderPair = OrderPair(DiamondHands.ALGORITHM, order, None)
        pair.buy_only = True

        # Queue order
        self.ctx.order_book.append(pair)

        # Update next buy time
        self.next_buy = datetime.now() + relativedelta(minutes=Settings.HODL_FREQUENCY_MINUTES)
