import time
import copy
from datetime import datetime

from gtb.market.prices import MarketPrices
from gtb.core.thread import BotThread
from gtb.core.context import Context
from gtb.utils.logging import Log
from gtb.utils.maths import floor_usd

from coinbase.rest import products

# Keep track of the current market bid/ask
class CurrentMarketThread(BotThread):
    max_change_per_minute: float

    def __init__(self, ctx: Context) -> None:
        super().__init__(ctx)
        # smooth = 0.1% change per minute max
        self.max_change_per_minute = 0.001

    def init(self) -> None:
        # Make sure we have the current market data before any other threads start
        while True:
            data: MarketPrices | None = self.retrieve()
            if not data:
                time.sleep(1)
                continue
            assert data is not None
            self.ctx.current_market = data
            self.ctx.smooth_market = copy.deepcopy(data)
            break
        Log.info("Current market initialized.")

    def think(self) -> None:
        # Get the current market data
        current_market: MarketPrices | None = self.retrieve()
        if not current_market:
            return None
        assert current_market is not None

        # Blend old and new data
        old_bid: float = self.ctx.smooth_market.bid
        old_ask: float = self.ctx.smooth_market.ask
        new_bid: float = self.blend(old_bid, current_market.bid)
        new_ask: float = self.blend(old_ask, current_market.ask)

        # Normalize bid/ask relation
        if new_ask < new_bid:
            new_ask = new_bid
        if new_bid > new_ask:
            new_bid = new_ask

        # Calculate split
        new_split: float = floor_usd((new_ask + new_bid) / 2)

        # Save update
        self.ctx.current_market = current_market
        self.ctx.smooth_market.bid = new_bid
        self.ctx.smooth_market.ask = new_ask
        self.ctx.smooth_market.split = new_split
        self.ctx.smooth_market.updated = current_market.updated

        # Log
        Log.trace("Market: [ {:.2f} | {:.2f} ] -> Smooth: [ {:.2f} | {:.2f} ]".format(
            floor_usd(self.ctx.current_market.bid),
            floor_usd(self.ctx.current_market.ask),
            floor_usd(self.ctx.smooth_market.bid),
            floor_usd(self.ctx.smooth_market.ask),
        ))

    def retrieve(self) -> MarketPrices | None:
        try:
            # BTC-USD: Base BTC, Quote USD
            data = products.get_best_bid_ask(self.ctx.api, product_ids=["BTC-USD"])

            ret = MarketPrices()
            ret.updated = datetime.now()
            # Buy price
            ret.bid = float(data['pricebooks'][0]['bids'][0]['price'])
            # Sell price
            ret.ask = float(data['pricebooks'][0]['asks'][0]['price'])
            # Half point
            ret.split = round((ret.bid + ret.ask) / 2, 16)
            return ret
        except Exception as e:
            Log.exception("Failed to get market price", e)
            return None

    def blend(self, current: float, new: float) -> float:
        positive: float = 1 if new > current else -1
        delta: float = abs((current - new) / current)

        # Don't change too fast
        seconds: float = (datetime.now() - self.ctx.smooth_market.updated).total_seconds()
        max_delta: float = self.max_change_per_minute * (seconds / 60)
        if delta > max_delta:
            delta = max_delta

        return current + (current * delta * positive)