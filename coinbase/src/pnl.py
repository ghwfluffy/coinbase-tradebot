import json
from datetime import datetime

def floor(x):
    return math.floor(float(x))

def parse_date(x):
    return datetime.strptime(x, "%Y-%m-%d %H:%M:%S")

with open("historical.json", "r") as fp:
    data = json.loads(fp.read())

for order in data:
    if order['status'] != "Complete":
        continue
    start_time = order['buy']['order_time']
    end_time = order['sell']['final_time'] if 'final_time' in order['sell'] else order['sell']['order_time']

    delta_time = (parse_date(end_time) - parse_date(start_time)).total_seconds() // 60

    sell_usd = float(order['sell']['usd'])
    buy_usd = float(order['buy']['usd'])

    fee = (sell_usd * 0.001) + (buy_usd * 0.001)
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
