from gtb.core.api import CoinbaseApi
from gtb.market.prices import MarketPrices

from typing import Any

class Context():
    api: CoinbaseApi
    current_market: MarketPrices
    smooth_market: MarketPrices
    data: dict[str, Any]
    is_running: bool

    def __init__(self):
        self.api = CoinbaseApi()
        self.current_market = MarketPrices()
        self.smooth_market = MarketPrices()
        self.data = {}
        self.is_running = False
