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

def buy_btc(ctx, amount, bid_price):
    client_order_id = gen_client_order_id('buy')

    bid_price = floor(bid_price, 2)
    usd_quantity = floor(amount * bid_price, 2)

    try:
        #order_info = fake_order(
        order_info = orders.stop_limit_order_gtc_buy(
            ctx,
            client_order_id=client_order_id,
            product_id='BTC-USD',
            base_size=str(amount), # Amount of BTC to buy
            stop_price=str(bid_price - 10), # Price to trigger at (USD)
            limit_price=str(bid_price), # Don't buy anything more expensive than this (USD) (Higher than stop price)
            stop_direction='STOP_DIRECTION_STOP_DOWN', # Trigger when below stop_price
        )
    except Exception as e:
        print(e)
        order_info = {'success': False}

    order_id = None
    print("[{}] [BUY]  ".format(get_now_string()), end='')
    if not 'success' in order_info or not order_info['success']:
        print("[ERROR] [{}] ".format(order_info['error_response']['error']), end='')
    else:
        order_id = order_info['order_id']

    print("{:.5f} bitcoin for ${:.2f} (${:.2f} BTC).".format(amount, usd_quantity, bid_price))

    return order_id

def sell_btc(ctx, btc_quantity, ask_price):
    client_order_id = gen_client_order_id('sell')

    ask_price = floor(ask_price, 2)
    btc_quantity = floor(btc_quantity, 5)

    try:
        #order_info = fake_order(
        order_info = orders.stop_limit_order_gtc_sell(
            ctx,
            client_order_id=client_order_id,
            product_id='BTC-USD',
            base_size=str(btc_quantity), # Amount of BTC to sell
            stop_price=str(ask_price + 10), # Price to trigger at (USD)
            limit_price=str(ask_price), # Don't sell anything cheaper than this (USD) (Lower than stop price)
            stop_direction='STOP_DIRECTION_STOP_UP', # Trigger when above stop_price
        )
    except Exception as e:
        print(e)
        order_info = {'success': False}

    usd_price = floor(btc_quantity * ask_price, 2)

    order_id = None
    print("[{}] [SELL] ".format(get_now_string()), end='')
    if not 'success' in order_info or not order_info['success']:
        print("[ERROR] [{}] ".format(order_info['error_response']['error']), end='')
    else:
        order_id = order_info['order_id']

    print("{:.5f} bitcoin for ${:.2f} (${:.2f} BTC).".format(btc_quantity, usd_price, ask_price))

    return order_id

def is_order_complete(ctx, order_id):
    # No order
    if not order_id:
        return True

    try:
        # Get order
        data = orders.get_order(ctx, order_id)
        # Is complete for any state but OPEN
        return data['order']['status'] != 'OPEN'
    except Exception as e:
        print(e)
        print("[{}] [CHECK] [ERROR] Failed to get order {} status".format(get_now_string(), order_id))
        # Failed: Assume still open
        pass

    return False

def cancel_order(ctx, cancel_type, order_id):
    try:
        finish = orders.cancel_orders(
            ctx,
            order_ids=[order_id],
        )
        success = finish['results'][0]['success']
        print("[{}] [CANCEL: {}] {}".format(get_now_string(), cancel_type, "Success" if success else "Fail"))
        return True
    except Exception as e:
        print(e)
        print("[{}] [CANCEL: {}] [ERROR]".format(get_now_string(), cancel_type))
        pass

    return False
