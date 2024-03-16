import json

from context import Context
from utils.wallet import get_wallet
from utils.maths import floor_btc

# New Coinbase connection context
ctx: Context = Context()
# Get wallet info
wallet = get_wallet(ctx)

# Get orderbook
with open("orderbook.json", "r") as fp:
    data = json.loads(fp.read())
with open("historical.json", "r") as fp:
    history = json.loads(fp.read())

wallet_btc: float = wallet['BTC']['total']
pending_sell_btc: float = 0
for order in data['orders']:
    if order['status'] == 'PendingSell':
        if order['sell']:
            pending_sell_btc += float(order['sell']['btc'])
        else:
            pending_sell_btc += float(order['buy']['btc'])

hodl_btc: float = 0.0
for order in history:
    if order['tranche'] == "HODL":
        hodl_btc += float(order['buy']['btc'])

print("Wallet BTC: {}".format(floor_btc(wallet_btc)))
print("Pending Sell: {}".format(floor_btc(pending_sell_btc)))
print("HODL BTC: {}".format(floor_btc(hodl_btc)))
print("Orphaned: {}".format(floor_btc(wallet_btc - pending_sell_btc - hodl_btc)))
