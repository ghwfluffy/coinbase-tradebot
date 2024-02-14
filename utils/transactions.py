import time
import math

from coinbase.rest import orders

from utils.debug import get_now_string

def fake_order(*args, **kwargs):
    #print(kwargs)
    return { 'success': True }

def floor(x, decimals):
    power = float(pow(10, decimals))
    return math.floor(x * power) / power

def gen_client_order_id(order_type):
    return str(int(time.time() * 1000)) + "-" + order_type

def buy_btc(ctx, usd_quantity, bid_price):
    client_order_id = gen_client_order_id('buy')

    bid_price = floor(bid_price, 2)
    usd_quantity = floor(usd_quantity, 2)
    amount = floor(usd_quantity / bid_price, 5)

    #order_info = fake_order(
    order_info = orders.limit_order_gtc_buy(
        ctx,
        client_order_id=client_order_id,
        product_id='BTC-USD',
        base_size=str(amount), # Amount of BTC to buy
        limit_price=str(bid_price), # Don't buy anything more expensive than this (USD) (Higher than stop price)
    )

    print("[{}] [BUY]  ".format(get_now_string()), end='')
    if not 'success' in order_info or not order_info['success']:
        print("[ERROR] ", end='')

    print("{:.5f} bitcoin for ${:.2f} (${:.2f} BTC).".format(amount, usd_quantity, bid_price))

def sell_btc(ctx, btc_quantity, ask_price):
    client_order_id = gen_client_order_id('sell')

    ask_price = floor(ask_price, 2)
    btc_quantity = floor(btc_quantity, 5)

    #order_info = fake_order(
    order_info = orders.limit_order_gtc_sell(
        ctx,
        client_order_id=client_order_id,
        product_id='BTC-USD',
        base_size=str(btc_quantity), # Amount of BTC to sell
        limit_price=str(ask_price), # Don't sell anything cheaper than this (USD) (Lower than stop price)
    )

    usd_price = floor(btc_quantity * ask_price, 2)

    print("[{}] [SELL] ".format(get_now_string()), end='')
    if not 'success' in order_info or not order_info['success']:
        print("[ERROR] ", end='')

    print("{:.5f} bitcoin for ${:.2f} (${:.2f} BTC).".format(btc_quantity, usd_price, ask_price))
