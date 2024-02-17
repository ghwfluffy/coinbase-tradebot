from coinbase.rest import products

def get_market_price(ctx):
    # BTC-USD: Base BTC, Quote USD
    data = products.get_best_bid_ask(ctx, product_ids=["BTC-USD"])
    # Buy price
    bid = float(data['pricebooks'][0]['bids'][0]['price'])
    # Sell price
    ask = float(data['pricebooks'][0]['asks'][0]['price'])
    return {
        'bid': bid,
        'ask': ask,
    }
