#!/usr/bin/env python3

import math
import json
import numpy
from datetime import datetime
from typing import List, Tuple

import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib

from scipy.interpolate import make_interp_spline

from gtb.orders.order_book import OrderBook
from gtb.orders.history import OrderHistory
from gtb.orders.order_pair import OrderPair
from gtb.orders.order import Order
from gtb.utils.logging import Log

Log.INFO = False

# Utils
def floor(x):
    return math.floor(float(x))

def parse_date(x):
    return datetime.strptime(x, "%Y-%m-%d %H:%M:%S")

def create_plot(
    START_AT="2024-03-20 08:00:00",
    backend='Agg'
    ) -> None:

    START_AT = parse_date(START_AT)
    matplotlib.use(backend)

    # Get data
    order_book: OrderBook = OrderBook()
    order_book.read_fs()
    history: OrderHistory = OrderHistory()
    history.read_fs()

    market_csv: List[str]
    with open("data/market.csv") as fp:
        market_csv = fp.readlines()

    # Aggregated data
    times: List[datetime] = []
    values: List[float] = []
    MINMAX=[0,0]
    def minmax(MINMAX, value):
        if not MINMAX[0] or MINMAX[0] > value:
            MINMAX[0] = value
        if not MINMAX[1] or MINMAX[1] < value:
            MINMAX[1] = value
        return MINMAX

    # Start graph
    plt.figure(figsize=(10, 6))

    # Add order pairs
    pairs = order_book.order_pairs
    pairs += history.order_pairs
    for pair in pairs:
        if pair.event_time < START_AT:
            continue

        if pair.status == OrderPair.Status.Canceled:
            continue

        if pair.algorithm == "HODL":
            values.append(pair.buy.get_limit_price())
            times.append(pair.event_time)
            plt.scatter(pair.event_time, pair.buy.get_limit_price(), color='pink') # type: ignore
            continue

        # Buy
        buy_market: float
        buy_time: datetime
        color: str = 'blue'
        if pair.buy.status == Order.Status.Active:
            assert pair.buy.info is not None
            buy_time = pair.buy.info.order_time
            buy_market = pair.buy.get_limit_price()
        elif pair.buy.status == Order.Status.Complete:
            assert pair.buy.info is not None
            assert pair.buy.info.final_time is not None
            assert pair.buy.info.final_market is not None
            buy_time = pair.buy.info.final_time
            buy_market = pair.buy.info.final_market
            color = 'red'
        else:
            buy_time = pair.event_time
            buy_market = pair.buy.get_limit_price()
        plt.scatter(buy_time, buy_market, color=color) # type: ignore
        times.append(buy_time)
        values.append(buy_market)

        if not pair.sell:
            continue

        # Sell
        sell_market: float
        sell_time: datetime
        if pair.sell.status == Order.Status.Active:
            assert pair.sell is not None
            assert pair.sell.info is not None
            sell_time = pair.sell.info.order_time
            sell_market = pair.sell.get_limit_price()
        elif pair.sell.status == Order.Status.Complete:
            assert pair.sell is not None
            assert pair.sell.info is not None
            assert pair.sell.info.final_time is not None
            assert pair.sell.info.final_market is not None
            sell_time = pair.sell.info.final_time
            sell_market = pair.sell.info.final_market
        else:
            sell_time = buy_time
            sell_market = pair.sell.get_limit_price()
        plt.scatter(sell_time, sell_market, color='green') # type: ignore
        times.append(sell_time)
        values.append(sell_market)

        if pair.sell.status == Order.Status.Complete:
            color = 'green' if sell_market >= buy_market else 'red'
            plt.plot([buy_time, sell_time], [buy_market, sell_market], color=color) # type: ignore
        else:
            plt.plot([buy_time, sell_time], [buy_market, sell_market], color='gray', linestyle='--', linewidth=1) # type: ignore

    if len(values) == 0:
        print("No matching data.")
        return None

    # Get min/max time
    times.sort()
    min_time: datetime = times[0]
    max_time: datetime = times[-1]

    # Make a yellow line for the market value
    market: List[Tuple[datetime, float]] = []
    for line in market_csv:
        fields: List[str] = line.split(",")
        when: datetime = parse_date(fields[0])
        if when < min_time:
            continue

        bid: float = float(fields[3])
        ask: float = float(fields[4])
        split: float = (bid + ask) / 2
        market.append((when, split))
        values.append(split)

    # Setup y axis
    values.sort()
    min_value: float = values[0]
    max_value: float = values[-1]
    plt.ylim(bottom=min_value - 10, top=max_value + 10)

    market.sort(key=lambda x:x[0])
    x, y = zip(*market)
    plt.plot(x, y, color='yellow')

    # Setup labels
    plt.title('Trade History')
    plt.xlabel('Time')
    plt.ylabel('Price USD')
    plt.legend()

if __name__ == '__main__':
    create_plot(backend='Qt5Agg')
    plt.show()
