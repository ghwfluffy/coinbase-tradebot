#!/usr/bin/env python3

import json
from datetime import datetime

START_TIME="2024-02-23 06:46:00"
#START_TIME=None

def floor(x):
    return math.floor(float(x))

def parse_date(x):
    return datetime.strptime(x, "%Y-%m-%d %H:%M:%S")

with open("historical.json", "r") as fp:
    data = json.loads(fp.read())

if START_TIME:
    START_TIME=parse_date(START_TIME)

total_profit = 0
for order in data:
    if order['status'] != "Complete":
        continue

    start_time = order['buy']['order_time']
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

    print("{}: ${} - {} min - ${:.2f} profit".format(
        end_time,
        buy_usd,
        int(delta_time),
        total,
        sell_usd,
        buy_usd,
    ))

    total_profit += total

print("Total profit: {:.2f}".format(total_profit))
