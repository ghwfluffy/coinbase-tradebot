import time
from datetime import datetime, timedelta, timezone

from coinbase.rest import rest_base, accounts, products, orders

from utils.wallet import get_wallet, wallets_equal
from utils.pricebook import get_market_price
from utils.debug import print_json, get_now_string
from utils.transactions import buy_btc, sell_btc, cancel_order, is_order_complete

CREDENTIAL="secrets/coinbase_cloud_api_key.json"

# REST API instance
ctx = rest_base.RESTBase(key_file=CREDENTIAL)

# Algorithm:
# Place bid/ask at market price +/- 500
# if sell BTC:
#   cancel buy
#   place new buy at -500
#   place new sell at +500
# if buy BTC:
#   place new buy at -500
SPREAD_SIZE = 500
WAGER_BTC = 0.0016

last_wallet = {}
sell_order = None
buy_order = None

while True:
    # Current BTC price
    try:
        market_price = get_market_price(ctx)['bid']
    except:
        print("[{}] Failed to get market price.".format(get_now_string()))
        time.sleep(60)
        continue

    # Sell order is complete: Cancel buy and place new sell
    if is_order_complete(ctx, sell_order):
        if is_order_complete(ctx, buy_order) or cancel_order(ctx, "BUY", buy_order):
            sell_order = sell_btc(ctx, WAGER_BTC, market_price + SPREAD_SIZE)
            print("[{}] Sell order ID: {}".format(get_now_string(), sell_order))

    # Buy order is complete: Place new buy order
    if is_order_complete(ctx, buy_order):
        buy_order = buy_btc(ctx, WAGER_BTC, market_price - SPREAD_SIZE)
        print("[{}] Buy order ID: {}".format(get_now_string(), buy_order))

    # Print wallet changes
    try:
        wallet = get_wallet(ctx)
        if not wallets_equal(last_wallet, wallet):
            print("[{}] Wallet:".format(get_now_string()))
            print_json(wallet)
            last_wallet = wallet
    except:
        print("[{}] Error loading wallet.".format(get_now_string()))
        time.sleep(60)
        continue

    # Sleep 1 minute
    time.sleep(6)
