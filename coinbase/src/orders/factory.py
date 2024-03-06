from orders.target import TargetState
from orders.order import Order
from orders.order_pair import OrderPair
from market.current import CurrentMarket
from market.smooth import SmoothMarket
from algorithm.tranche import Tranche

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
    Log.debug("Market price ${:.2f} triggering pair order ({:.8f} BTC: ${:.2f} -> ${:.2f}).".format(
        market.split, num_bitcoins, target.wager, sell_price))
    return OrderPair(target, market.split, buy, sell)

def create_tranched_pair(market: SmoothMarket, tranche: Tranche, lowball: bool = True) -> OrderPair:
    if lowball:
        delta: float = market.split * tranche.spread
        buy_at: float = floor_usd(market.split - delta)
        sell_at: float = floor_usd(market.split)
    else:
        delta: float = (market.split * tranche.spread) / 2
        buy_at: float = floor_usd(market.split - delta)
        sell_at: float = floor_usd(market.split + delta)

    # What is the buy-price equivalent of our wager
    num_bitcoins = floor_btc(tranche.usd / buy_at)
    # What is the sell-price for that many bitcoins at our sell_at
    sell_price = floor_usd(num_bitcoins * sell_at)

    buy = Order(Order.Type.Buy, num_bitcoins, tranche.usd)
    sell = Order(Order.Type.Sell, num_bitcoins, sell_price)
    Log.debug("Market price ${:.2f} triggering {} order ({:.8f} BTC: ${:.2f} -> ${:.2f}).".format(
        market.split, tranche.name, num_bitcoins, tranche.usd, sell_price))
    return OrderPair(tranche.name, market.split, buy, sell)

def create_phased_pair(market: CurrentMarket, usd: float) -> OrderPair:
    buy_at: float = floor_usd(market.ask)

    # What is the buy-price equivalent of our wager
    num_bitcoins = floor_btc(usd / buy_at)
    # What is the sell-price for that many bitcoins at our sell_at

    buy = Order(Order.Type.Buy, num_bitcoins, usd)
    buy.tranched = False
    buy.maker_buffer = 5.0
    sell = None
    Log.debug("Market phase at price ${:.2f} triggering phased order ({:.8f} BTC: ${:.2f}).".format(
        market.ask, num_bitcoins, usd))
    return OrderPair("Phased", market.ask, buy, sell)
