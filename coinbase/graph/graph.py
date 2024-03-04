#!/usr/bin/env python3

import math
import json
import numpy
from datetime import datetime

import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib

from scipy.interpolate import make_interp_spline

def create_plot(
    START_AT="2024-02-29 00:00:00",
    SHOW_ONLY_COMPLETE=False,
    SHOW_PENDING=True,
    ORDERBOOK_ONLY=False,
    backend='Agg'
    ):

    matplotlib.use(backend)

    # Get data
    data = []
    with open("orderbook.json", "r") as fp:
        data = json.loads(fp.read())
        data = data['orders']
    if not ORDERBOOK_ONLY:
        with open("historical.json", "r") as fp:
            data += json.loads(fp.read())
    data.sort(key=lambda x:datetime.strptime(x['event_time'], "%Y-%m-%d %H:%M:%S"))

    # Utils
    def floor(x):
        return math.floor(float(x))

    def parse_date(x):
        return datetime.strptime(x, "%Y-%m-%d %H:%M:%S")

    # Aggregated data
    MINMAX=[0,0]
    def minmax(MINMAX, value):
        if not MINMAX[0] or MINMAX[0] > value:
            MINMAX[0] = value
        if not MINMAX[1] or MINMAX[1] < value:
            MINMAX[1] = value
        return MINMAX

    market=[]

    if START_AT:
        START_AT = parse_date(START_AT)
    plt.figure(figsize=(10, 6))

    for order in data:
        if order['status'] == "Canceled" and not order['buy']['order_time'] and not order['sell']['order_time']:
            continue

        if not SHOW_PENDING and order['status'] == "Pending" and not order['buy']['order_time']:
            continue

        event_time = parse_date(order['event_time'])
        if START_AT and event_time < START_AT:
            continue

        # Fix old data
        if not 'final_time' in order['buy']:
            order['buy']['final_time'] = order['buy']['order_time']
        if not 'final_time' in order['sell']:
            order['sell']['final_time'] = order['sell']['order_time']
        if not 'final_market' in order['buy']:
            order['buy']['final_market'] = order['buy']['market']
        if not 'final_market' in order['sell']:
            order['sell']['final_market'] = order['sell']['market']

        if SHOW_ONLY_COMPLETE and not order['sell']['final_time']:
            continue

        # Plot the event point
        event_price = floor(order['event_price'])
        plt.scatter(event_time, event_price, color='blue')
        minmax(MINMAX, event_price)
        market.append((event_time, event_price))

        # Plot the buy order placed
        if order['buy']['order_time']:
            buy_order_time = parse_date(order['buy']['order_time'])
            buy_order_price = floor(order['buy']['market'])

            plt.scatter(buy_order_time, buy_order_price, color='red')
            plt.plot([event_time, buy_order_time], [event_price, buy_order_price], color='black', linestyle=':', linewidth=1)

            minmax(MINMAX, buy_order_price)
        elif SHOW_PENDING:
            buy_order_time = event_time
            buy_order_price = floor(order['buy']['market'])

            plt.scatter(buy_order_time, buy_order_price, color='red')
            plt.plot([event_time, buy_order_time], [event_price, buy_order_price], color='gray', linestyle='--', linewidth=1)

            minmax(MINMAX, buy_order_price)

        # Plot the buy order filled
        if order['buy']['status'] == "Complete" and order['buy']['final_time']:
            buy_final_time = parse_date(order['buy']['final_time'])
            buy_final_price = floor(order['buy']['final_market'])

            plt.scatter(buy_final_time, buy_final_price, color='red')
            plt.plot([buy_order_time, buy_final_time], [buy_order_price, buy_final_price], color='black', linestyle=':', linewidth=1)

            minmax(MINMAX, buy_final_price)
            market.append((buy_final_time, buy_final_price))

        # Plot the sell order placed
        #if order['sell']['order_time']:
        #    sell_order_time = parse_date(order['sell']['order_time'])
        #    sell_order_price = floor(order['sell']['market'])

        #    plt.scatter(sell_order_time, sell_order_price, color='green')
        #    plt.plot([buy_final_time, sell_order_time], [buy_final_price, sell_order_price], color='green')

        #    minmax(MINMAX, sell_order_price)
        #elif SHOW_PENDING:
        #    sell_order_time = event_time
        #    sell_order_price = floor(order['sell']['market'])

        #    plt.scatter(sell_order_time, sell_order_price, color='green')
        #    plt.plot([event_time, sell_order_time], [event_price, sell_order_price], color='gray', linestyle='--', linewidth=1)

        #    minmax(MINMAX, sell_order_price)

        # Plot the sell order filled
        if order['sell']['status'] == "Complete" and order['sell']['final_time']:
            sell_final_time = parse_date(order['sell']['final_time'])
            sell_final_price = floor(order['sell']['final_market'])

            plt.scatter(sell_final_time, sell_final_price, color='green')
            color = 'green' if sell_final_price >= buy_final_price else 'red'
            plt.plot([buy_final_time, sell_final_time], [buy_final_price, sell_final_price], color=color)

            minmax(MINMAX, sell_final_price)
            market.append((sell_final_time, sell_final_price))
        elif SHOW_PENDING:
            sell_order_time = event_time
            sell_order_price = floor(order['sell']['market'])

            plt.scatter(sell_order_time, sell_order_price, color='green')
            plt.plot([event_time, sell_order_time], [event_price, sell_order_price], color='gray', linestyle='--', linewidth=1)

            minmax(MINMAX, sell_order_price)

    if len(market) == 0:
        print("No matching data.")
        exit(0)

    # Setup y axis
    plt.ylim(bottom=MINMAX[0] - 10, top=MINMAX[1] + 10)

    # Make a yellow line for the market value
    market.sort(key=lambda x:x[0])
    x, y = zip(*market)
    plt.plot(x, y, color='yellow')

    # Setup labels
    plt.title('Trade History')
    plt.xlabel('Time')
    plt.ylabel('Price USD')
    plt.legend()

    return plt

if __name__ == '__main__':
    plt = create_plot()
    plt.show(backend='Qt5Agg')
