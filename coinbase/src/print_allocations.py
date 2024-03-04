import json

from context import Context
from utils.wallet import get_wallet
from utils.math import floor_btc

# New Coinbase connection context
ctx: Context = Context()
# Get wallet info
wallet = get_wallet(ctx)

# Get orderbook
with open("orderbook.json", "r") as fp:
    data = json.loads(fp.read())

wallet_btc = wallet['BTC']['total']
pending_sell_btc = 0
for order in data['orders']:
    if order['status'] == 'PendingSell':
        pending_sell_btc += float(order['sell']['btc'])

print("Wallet BTC: {}".format(floor_btc(wallet_btc)))
print("Pending Sell: {}".format(floor_btc(pending_sell_btc)))
print("Orphaned: {}".format(floor_btc(wallet_btc - pending_sell_btc)))
