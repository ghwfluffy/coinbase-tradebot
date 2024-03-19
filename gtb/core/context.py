from gtb.core.api import CoinbaseApi
from gtb.market.prices import MarketPrices
from gtb.phases.calculations import PhaseCalculations
from gtb.orders.history import OrderHistory
from gtb.orders.order_book import OrderBook
from gtb.noti.queue import NotificationQueue

from typing import Any

class Context():
    api: CoinbaseApi
    current_market: MarketPrices
    smooth_market: MarketPrices
    phases: PhaseCalculations
    history: OrderHistory
    order_book: OrderBook
    notify: NotificationQueue
    is_running: bool

    def __init__(self) -> None:
        self.api = CoinbaseApi()
        self.current_market = MarketPrices()
        self.smooth_market = MarketPrices()
        self.phases = PhaseCalculations()
        self.history = OrderHistory()
        self.order_book = OrderBook()
        self.notify = NotificationQueue()
        self.is_running = False
