import time
from datetime import datetime, timedelta, timezone

from coinbase.rest import rest_base, accounts, products, orders

from utils.wallet import get_wallet, wallets_equal
from utils.pricebook import get_market_price
from utils.debug import print_json, get_now_string
from utils.transactions import buy_btc, sell_btc

# How big of a spread to play with
SPREAD_SIZE = 100

CREDENTIAL="secrets/coinbase_cloud_api_key.json"

# REST API instance
ctx = rest_base.RESTBase(key_file=CREDENTIAL)

market_prices = []
last_wallet = {}
while True:
    # Get wallet
    wallet = get_wallet(ctx)
    reload = False

    # Current BTC price
    market_price = get_market_price(ctx)['bid']

    # Get the average price over the last two hours
    market_prices.append(market_price)
    average_price = sum(market_prices) / len(market_prices)

    # Keep 2 hours worth of data
    while len(market_prices) > 120:
        market_prices.pop(0)

    # We have money to spend
    if wallet['USD']['available'] > 10:
        # Buy below average value and current market value
        buy_price = average_price - SPREAD_SIZE
        if buy_price > market_price - SPREAD_SIZE:
            buy_price = market_price - SPREAD_SIZE
        buy_btc(ctx, wallet['USD']['available'] - 5, buy_price)
        reload = True

    # We have bitcoin to sell
    if wallet['BTC']['available'] > 0.0001:
        # Sell above average value and current market value
        sell_price = average_price + SPREAD_SIZE
        if sell_price < market_price + SPREAD_SIZE:
            sell_price = market_price + SPREAD_SIZE
        sell_btc(ctx, wallet['BTC']['available'], sell_price)
        reload = True

    # Cancel bids to buy bitcoin that are older than 2 hours
    cur_orders = orders.list_orders(ctx, order_status=['OPEN'])
    for order in cur_orders['orders']:
        if order['side'] != "BUY":
            continue
        created = datetime.strptime(order['created_time'], "%Y-%m-%dT%H:%M:%S.%fZ").replace(tzinfo=timezone.utc)
        now = datetime.now(timezone.utc)
        time_difference = now - created
        if time_difference > timedelta(hours=2):
            result = orders.cancel_orders(ctx, order_ids=[order['order_id']])
            success = False
            try:
                success = result['results'][0]['success']
            except:
                pass
            print("[{}] [CANCEL] ".format(get_now_string()), end='')
            if not success:
                print("[ERROR] ", end='')
            print("Order {} of {:.5f} BTC at ${:.2f}.".format(
                order['client_order_id'],
                float(order['order_configuration']['limit_limit_gtc']['base_size']),
                float(order['order_configuration']['limit_limit_gtc']['limit_price'])))
            reload = True

    # Show updated wallet info
    if reload:
        wallet = get_wallet(ctx)
    if not wallets_equal(last_wallet, wallet):
        print("[{}] Wallet:".format(get_now_string()))
        print_json(wallet)
        last_wallet = wallet

    # Sleep 1 minute
    time.sleep(60)
