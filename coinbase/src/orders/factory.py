from orders.target import TargetState
from orders.order_pair import OrderPair
from orders.market import CurrentMarket

from utils.math import floor

def create_order(market: CurrentMarket, target: TargetState) -> OrderPair:
    # Difference from market price for buy/sell
    delta: float = (market.split * target.spread) / 2
    buy_at: float = market.split - delta
    sell_at: float = market.split + delta

    # What is the buy-price equivalent of our wager
    num_bitcoins = floor(target.wager / buy_at, 6)
    # What is the sell-price for that many bitcoins at our sell_at
    sell_price = floor(num_bitcoins * sell_at, 2)

    buy = Order(Order.Buy, num_bitcoins, target.wager)
    sell = Order(Order.Sell, num_bitcoins, sell_price)
    return OrderPair(buy, sell)
