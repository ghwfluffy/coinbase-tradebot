import json

from datetime import datetime
from dateutil.relativedelta import relativedelta

from gtb.core.thread import BotThread
from gtb.core.context import Context
from gtb.utils.logging import Log
from gtb.utils.maths import floor_usd

from coinbase.rest import products

VOLUME_WINDOW = 3000.0

class MarketVolume():
    pricebook: dict

    def __init__(self, pricebook: dict):
        self.pricebook = pricebook

    def debug_print(self) -> None:
        print(json.dumps(self.pricebook, indent=2))

    def get_bids(self, window: float = 1000.0) -> float:
        return self.get_window(float(self.pricebook['asks'][0]['price']), self.pricebook['bids'], window)

    def get_asks(self, window: float = 1000.0) -> float:
        return self.get_window(float(self.pricebook['bids'][0]['price']), self.pricebook['asks'], window)

    def get_window(self, mid: float, orders: dict, window: float) -> float:
        ret: float = 0.0
        for order in orders:
            delta: float = abs(mid - float(order['price']))
            # Ignore anything more than $1000 away
            if delta > window:
                continue
            # Scale from 0 to 1 based on how close it is to the mid price
            scale: float = (window - delta) / window
            ret += float(order['size']) * scale
        return ret

def get_volume_orders(ctx: Context) -> MarketVolume:
    # BTC-USD: Base BTC, Quote USD
    data = products.get_product_book(ctx.api, product_id="BTC-USD")
    return MarketVolume({
            'bids': data['pricebook']['bids'],
            'asks': data['pricebook']['asks'],
        })

class TrackVolumeThread(BotThread):
    file: str = "data/volume.csv"
    next_write: datetime

    def __init__(self, ctx: Context) -> None:
        super().__init__(ctx, sleep_seconds = 1)
        self.next_write = datetime.now()

    def init(self) -> None:
        pass

    def think(self) -> None:
        pass
