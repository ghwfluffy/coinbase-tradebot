from coinbase.rest import products
from utils.logging import Log

class CurrentMarket():
    bid: float
    ask: float
    split: float

    @staticmethod
    def get(ctx) -> Market:
        try:
            # BTC-USD: Base BTC, Quote USD
            data = products.get_best_bid_ask(ctx, product_ids=["BTC-USD"])

            ret = Market()
            # Buy price
            ret.bid = float(data['pricebooks'][0]['bids'][0]['price'])
            # Sell price
            ret.ask = float(data['pricebooks'][0]['asks'][0]['price'])
            # Half point
            ret.split = math.round((bid + ask) / 2, 16)
            return ret
        except Exception as e:
            Log.error("Failed to get market price {}".format(str(e)))
            return None
