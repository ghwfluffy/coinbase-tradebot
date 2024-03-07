import json
from utils.maths import floor_usd, floor_btc

with open("historical.json", "r") as fp:
    orders = json.loads(fp.read())

bad_orders = []
for order in orders:
    if order['buy']['status'] != 'Canceled' and order['sell']['status'] == 'Canceled':
        bad_orders.append(order)

print(json.dumps(bad_orders))
