import json

from coinbase.rest import rest_base, accounts, products, orders

CREDENTIAL="secrets/coinbase_cloud_api_key.json"

def print_json(data):
    print(json.dumps(data, indent=4))

# REST API instance
ctx = rest_base.RESTBase(key_file=CREDENTIAL)

# Get accounts
info = accounts.get_accounts(ctx)
print_json(info)

# BTC-USD: Base BTC, Quote USD
data = products.get_best_bid_ask(ctx, product_ids=["BTC-USD"])
print_json(data)

# Buy BTC
if False:
    order_info = orders.stop_limit_order_gtc_buy(
        ctx,
        client_order_id='FirstBuy',
        product_id='BTC-USD',
        base_size='0.0016', # Amount of BTC to buy
        stop_price='49500.00', # Price to trigger at (USD)
        limit_price='49766.00', # Don't buy anything more expensive than this (USD) (Higher than stop price)
        stop_direction='STOP_DIRECTION_STOP_DOWN', # Trigger when below stop_price
    )

    print_json(order_info)

    order_id = order_info['order_id']

# Sell BTC
if False:
    order_info = orders.stop_limit_order_gtc_sell(
        ctx,
        client_order_id='SecondSell',
        product_id='BTC-USD',
        base_size='0.0016', # Amount of BTC to sell
        stop_price='50200.00', # Price to trigger at (USD)
        limit_price='50100.00', # Don't sell anything cheaper than this (USD) (Lower than stop price)
        stop_direction='STOP_DIRECTION_STOP_UP', # Trigger when above stop_price
    )

    print_json(order_info)

    order_id = order_info['order_id']

# Cancel order
if False:
    finish = orders.cancel_orders(
        ctx,
        order_ids=[order_id],
    )

    print_json(finish)
