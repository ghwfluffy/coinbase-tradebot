import json

from context import Context
from utils.wallet import get_wallet

# New Coinbase connection context
ctx: Context = Context()
wallet = get_wallet(ctx)
print(json.dumps(wallet, indent=2))

with open("orderbook-v2.json", "r") as fp:
    data = json.loads(fp.read())

to_be_sold = 0.0
total_queued = 0.0
sell_prices=[]
for order in data['orders']:
    if order['status'] == "PendingSell":
        to_be_sold += float(order['sell']['btc'])
        sell_prices.append(order['sell']['market'])
    total_queued += float(order['sell']['btc'])

print("BTC queued for selling:")
print("Matched: {}".format(to_be_sold))
print("Total: {}".format(total_queued))
print("Pending sale prices:")
sell_prices.sort()
for x in sell_prices:
    print(x)
