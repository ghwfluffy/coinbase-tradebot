from orders.target import TargetState
from orders.order import Order
from orders.order_pair import OrderPair
from market.current import CurrentMarket

from utils.logging import Log
from utils.math import floor_btc, floor_usd

def create_order(market: CurrentMarket, target: TargetState, buy_now = False) -> OrderPair:
    # Difference from market price for buy/sell
    if buy_now or target.autofill:
        delta: float = market.split * target.spread
        buy_at: float = floor_usd(market.split)
        sell_at: float = floor_usd(market.split + delta)
    else:
        delta: float = (market.split * target.spread) / 2
        buy_at: float = floor_usd(market.split - delta)
        sell_at: float = floor_usd(market.split + delta)

    # What is the buy-price equivalent of our wager
    num_bitcoins = floor_btc(target.wager / buy_at)
    # What is the sell-price for that many bitcoins at our sell_at
    sell_price = floor_usd(num_bitcoins * sell_at)

    buy = Order(Order.Type.Buy, num_bitcoins, target.wager)
    sell = Order(Order.Type.Sell, num_bitcoins, sell_price)
    Log.debug("Market price ${:.2f} triggering pair order ({} BTC: ${:.2f} -> ${:.2f}).".format(
        market.split, num_bitcoins, target.wager, sell_price))
    return OrderPair(target, market.split, buy, sell)
