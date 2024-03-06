from market.current import CurrentMarket
from utils.logging import Log
from utils.math import floor_usd
from datetime import datetime

class SmoothMarket():
    bid: float
    ask: float
    split: float
    last_update: datetime
    max_change_per_minute: float

    def __init__(self, max_change_per_minute: float = 0.0002):
        self.bid = None
        self.ask = None
        self.split = None
        self.last_update = None
        self.max_change_per_minute = max_change_per_minute

    def update_market(self, market: CurrentMarket) -> None:
        if not self.bid:
            self.bid = market.bid
            self.ask = market.ask
            self.split = market.split
            self.last_update = datetime.now()
            return None

        # Blend old and new data
        old_bid: float = self.bid
        old_ask: float = self.ask
        self.bid = self.blend(self.bid, market.bid)
        self.ask = self.blend(self.ask, market.ask)

        # Normalize bid/ask relation
        if self.ask < self.bid:
            self.ask = self.bid
        if self.bid > self.ask:
            self.bid = self.ask

        # Calculate split
        self.split = (self.ask + self.bid) / 2

        # Record when updated
        self.last_update = datetime.now()

        #Log().debug("SMOOTH: [{}|{}]={} : [{}|{}]={}".format(
        #    floor_usd(old_bid), floor_usd(market.bid), floor_usd(self.bid),
        #    floor_usd(old_ask), floor_usd(market.ask), floor_usd(self.ask)))

    def blend(self, current: float, new: float) -> float:
        positive: float = 1 if new > current else -1
        delta: float = abs((current - new) / current)

        # Maximum change is 0.02% per minute
        seconds: float = (datetime.now() - self.last_update).total_seconds()
        max_delta = self.max_change_per_minute * (seconds / 60)
        if delta > max_delta:
            delta = max_delta

        return current + (current * delta * positive)
