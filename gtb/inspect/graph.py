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
from gtb.market.volume import MarketVolume

Log.INFO = False

# Utils
def floor(x):
    return math.floor(float(x))

def parse_date(x):
    return datetime.strptime(x, "%Y-%m-%d %H:%M:%S")

def create_plot(
    START_AT="2024-05-03 17:00:00",
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

    market_demand: List[Tuple[datetime, float]] = []
    with open("data/demand.csv") as fp:
        for line in fp:
            demand_fields: List[str] = line.split(",")
            market_demand.append((datetime.strptime(demand_fields[0], "%Y-%m-%d %H:%M:%S.%f"), float(demand_fields[1])))

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

        color: str
        buy_market: float
        buy_time: datetime
        if pair.algorithm == "HODL":
            color = 'pink'
            buy_time = pair.event_time
            if pair.buy.status == Order.Status.Complete:
                assert pair.buy.info is not None
                buy_time = pair.buy.info.order_time
                color = '#C71585'

            buy_market = pair.buy.get_limit_price()
            values.append(buy_market)
            times.append(buy_time)
            plt.scatter(buy_time, buy_market, color=color) # type: ignore
            continue

        # Buy
        if pair.buy.status == Order.Status.Active:
            assert pair.buy.info is not None
            color = 'orange'
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
            color = 'blue'
            buy_time = pair.event_time
            buy_market = pair.buy.get_limit_price()
        plt.scatter(buy_time, buy_market, color=color) # type: ignore
        times.append(buy_time)
        values.append(buy_market)

        # Draw from conception
        plt.scatter(pair.event_time, pair.event_price, color="black") # type: ignore
        plt.plot([pair.event_time, buy_time], [pair.event_price, buy_market], color="orange", zorder=1) # type: ignore

        if not pair.sell:
            # Shouldn't happen
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
            #sell_time = buy_time
            sell_market = pair.sell.get_limit_price()
        plt.scatter(sell_time, sell_market, color='green') # type: ignore
        times.append(sell_time)
        values.append(sell_market)

        if pair.sell.status == Order.Status.Complete:
            color = 'green' if sell_market >= buy_market else 'red'
            plt.plot([buy_time, sell_time], [buy_market, sell_market], color=color, zorder=1) # type: ignore
        else:
            plt.plot([buy_time, sell_time], [buy_market, sell_market], color='gray', linestyle='--', linewidth=1, zorder=1) # type: ignore
            plt.plot([sell_time, datetime.now()], [sell_market, sell_market], color='gray', linestyle='--', linewidth=1, zorder=1) # type: ignore

    if len(values) == 0:
        print("No matching data.")
        return None

    # Get min/max time
    times.sort()
    min_time: datetime = times[0]
    max_time: datetime = times[-1]

    # Make a yellow line for the market value
    market: List[Tuple[datetime, float]] = []
    demand: List[Tuple[datetime, float]] = []
    demand_smooth: float = 0
    demand_lastupdate: datetime
    volume_index: int = 0
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


        if volume_index == 0:
            demand_smooth = split
            demand_lastupdate = when

        #while (volume_index + 1) < len(market_volume):
        #    cur_volume: MarketVolume = market_volume[volume_index]
        #    if cur_volume.get_time() > when:
        #        break
        #    if cur_volume.get_time() <= when and market_volume[volume_index + 1].get_time() >= when:
        #        demand_pos: int = 10
        #        demand_value: float = cur_volume.get_bids(demand_pos) / cur_volume.get_asks(demand_pos)
        #        below: bool = demand_value < 1
        #        while (below and demand_value < 1) or (not below and demand_value > 1):
        #            demand_pos += 5
        #            demand_value = cur_volume.get_bids(demand_pos) / cur_volume.get_asks(demand_pos)
        #            if demand_pos > 2000:
        #                break

        #        positive: float = -1 if below else 1
        #        delta: float = abs(demand_pos / demand_smooth)

        #        # Don't change too fast
        #        demand_time_diff: float = (when - demand_lastupdate).total_seconds()
        #        max_demand_per_minute: float = 0.001
        #        max_delta: float = max_demand_per_minute * (demand_time_diff / 60)
        #        if delta > max_delta:
        #            delta = max_delta

        #        demand.append((when, demand_smooth + (demand_smooth * delta * positive)))
        #        break
        #    volume_index += 1
        start_index: int = volume_index
        while False and (volume_index + 1) < len(market_demand):
            start_demand: Tuple[datetime, float] = market_demand[start_index]
            if start_demand[0] > when:
                break
            end_demand: Tuple[datetime, float] = market_demand[volume_index]
            if end_demand[0] >= when:
                def blend_demand(avg: Tuple[datetime, float], cur: Tuple[datetime, float]) -> Tuple[datetime, float]:
                    demand_delta: float = cur[1] - avg[1]
                    positive: int = 1 if demand_delta > 0 else -1
                    time_delta: float = (cur[0] - avg[0]).total_seconds()
                    max_delta_per_minute: float = 200
                    max_delta: float = max_delta_per_minute * (time_delta / 60)
                    if abs(demand_delta) > max_delta:
                        demand_delta = max_delta * positive
                    return (cur[0], avg[1] + demand_delta)

                avg: Tuple[datetime, float] = (start_demand[0], 0)
                avg2: float = 0
                for index in range(start_index, volume_index):
                    cur_demand: Tuple[datetime, float] = market_demand[index]
                    avg = blend_demand(avg, cur_demand)

                    avg2 += cur_demand[1]
                avg2 /= (volume_index - start_index)

                demand_value: float = split + avg2
                #demand_value: float = split + avg[1]
                positive: float = 1 if demand_value >= demand_smooth else -1
                demand_time_diff: float = (when - demand_lastupdate).total_seconds()
                demand_delta: float = abs(demand_smooth - demand_value)
                if demand_delta > 50:
                    demand_delta = 50
                #max_delta_per_minute: float = 200
                #max_delta: float = max_delta_per_minute * (demand_time_diff / 60)
                #print("| {} | {}".format(demand_time_diff, demand_delta))
                #if demand_delta > max_delta:
                #    demand_delta = max_delta
                demand_lastupdate = when
                demand_smooth = demand_smooth + (demand_delta * positive)
                demand.append((when, demand_smooth))
                #demand_lastupdate = when
                #demand_smooth = demand_smooth + (demand_smooth * delta * positive)
                #delta: float = 1 if demand_smooth == 0 else abs(demand_smooth - demand_value) / demand_smooth







                #if avg2 > 0:
                #    demand.append((when, split + 100))
                #else:
                #    demand.append((when, split - 100))
                #demand.append((when, split + avg2))
                #demand.append((when, split + avg[1]))

#                demand_value: float = avg[1]
#                positive: float = 1 if demand_value >= 0 else -1
#                demand_time_diff: float = (when - demand_lastupdate).total_seconds()
#                demand_delta: float = abs(demand_value)
#                max_delta_per_minute: float = 10
#                max_delta: float = max_delta_per_minute * (demand_time_diff / 60)
#                print("{} | {} | {}".format(demand_time_diff, demand_delta, max_delta))
#                if demand_delta > max_delta:
#                    demand_delta = max_delta
#                demand_lastupdate = when
#                demand_smooth = demand_smooth + (demand_delta * positive)
                #demand_lastupdate = when
                #demand_smooth = demand_smooth + (demand_smooth * delta * positive)
                #delta: float = 1 if demand_smooth == 0 else abs(demand_smooth - demand_value) / demand_smooth

                # Don't change too fast
                #demand_time_diff: float = (when - demand_lastupdate).total_seconds()
                #max_demand_per_minute: float = 10
                #max_delta: float = max_demand_per_minute * (demand_time_diff / 60)
                #if delta > max_delta:
                #    delta = max_delta
                #print("{} | {} | {}".format(demand_value, demand_smooth, delta))

                #demand_smooth = demand_smooth + (demand_smooth * delta * positive)
#                demand.append((when, split + demand_smooth))


                break
            volume_index += 1

    # Setup y axis
    values.sort()
    min_value: float = values[0]
    max_value: float = values[-1]
    plt.ylim(bottom=min_value - 10, top=max_value + 10)

    market.sort(key=lambda x:x[0])
    x, y = zip(*market)
    plt.plot(x, y, color='yellow', zorder=10, label='market')

    #demand.sort(key=lambda x:x[0])
    #x, y = zip(*demand)
    #plt.plot(x, y, color='#9900cc', zorder=10, label='demand')

    # Setup labels
    plt.title('Trade History')
    plt.xlabel('Time')
    plt.ylabel('Price USD')
    plt.legend()

if __name__ == '__main__':
    #create_plot(backend='Qt5Agg')
    create_plot(backend='Agg')
    plt.show()
