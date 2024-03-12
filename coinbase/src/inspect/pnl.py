#!/usr/bin/env python3

import json
import math
from datetime import datetime

from context import Context
from market.current import CurrentMarket

ctx: Context = Context()
market: CurrentMarket | None = CurrentMarket.get(ctx)
assert market is not None

START_TIME="2024-02-26 00:00:00"
#START_TIME=None

def parse_date(x):
    return datetime.strptime(x, "%Y-%m-%d %H:%M:%S")

with open("historical.json", "r") as fp:
    data = json.loads(fp.read())

if START_TIME:
    START_TIME=parse_date(START_TIME)

results = []
tranched_profit: float = 0
phased_profit: float = 0
hodl_profit: float = 0
for order in data:
    if order['status'] != "Complete":
        continue

    start_time = order['buy']['order_time']
    if START_TIME and parse_date(start_time) < START_TIME:
        continue

    total: float = 0

    # HODL we will calculate sell price using current market price
    if order['tranche'] == "HODL":
        buy_usd = float(order['buy']['usd'])
        buy_btc = float(order['buy']['btc'])
        current_usd = buy_btc * market.split
        fee = order['buy']['final_fees'] if 'final_fees' in order['buy'] else (buy_usd * 0.001)
        total = (current_usd - buy_usd) - fee
        hodl_profit += total

        results.append("{}: ${} - {} min - ${:.2f} profit".format(
            start_time,
            buy_usd,
            0,
            total,
        ))

        break

    end_time = order['sell']['final_time'] if 'final_time' in order['sell'] else order['sell']['order_time']
    if START_TIME and parse_date(end_time) < START_TIME:
        continue

    delta_time = (parse_date(end_time) - parse_date(start_time)).total_seconds() // 60

    sell_usd = float(order['sell']['usd'])
    buy_usd = float(order['buy']['usd'])

    fee = 0
    fee += order['sell']['final_fees'] if 'final_fees' in order['sell'] else (sell_usd * 0.001)
    fee += order['buy']['final_fees'] if 'final_fees' in order['buy'] else (buy_usd * 0.001)
    sub_total = sell_usd - buy_usd
    total = sub_total - fee

    results.append("{}: ${} - {} min - ${:.2f} profit".format(
        end_time,
        buy_usd,
        int(delta_time),
        total,
    ))

    # Phased or tranched?
    if order['tranche'] == "Phased":
        phased_profit += total
    else:
        tranched_profit += total

# Show most recent 10
while len(results) > 10:
    results.pop(0)

total_profit: float = tranched_profit + phased_profit + hodl_profit
print("Total profit   : {:.2f}".format(total_profit))
print("Tranched profit: {:.2f}".format(tranched_profit))
print("Phased profit  : {:.2f}".format(phased_profit))
print("HODL profit    : {:.2f}".format(hodl_profit))
print("")
results.reverse()
for r in results:
    print(r)
