from datetime import datetime

from context import Context
from utils.logging import Log

from coinbase.rest import market_data

class MarketWindow():
    low: float
    high: float

    def __init__(self, low: float, high: float):
        self.low = low
        self.high = high

    @classmethod
    def get(cls, ctx: Context, start_time: datetime, end_time: datetime) -> 'MarketWindow':
        try:
            diff = (end_time - start_time).total_seconds() / 60
            if diff < 60:
                granularity = 'ONE_MINUTE'
            else:
                granularity = 'THIRTY_MINUTE'
            result = market_data.get_candles(
                ctx,
                product_id='BTC-USD',
                start=str(int(start_time.timestamp())),
                end=str(int(end_time.timestamp())),
                granularity=granularity,
            )
            if 'candles' in result:
                low: float = None
                high: float = None
                for candle in result['candles']:
                    cur_low: float = float(candle['low'])
                    cur_high: float = float(candle['high'])
                    if not low or cur_low < low:
                        low = cur_low
                    if not high or cur_high > high:
                        high = cur_high
                if low and high:
                    return cls(low, high)
                else:
                    Log.error("No candles in market history.")
            else:
                Log.error("Failed to get market history: {}", result['error_response']['error'])
        except Exception as e:
            Log.error("Failed to get market history: {}".format(str(e)))

        return None
