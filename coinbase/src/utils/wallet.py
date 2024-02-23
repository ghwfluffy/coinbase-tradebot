import copy

from coinbase.rest import accounts
from market.current import CurrentMarket
from utils.logging import Log
from context import Context

def get_wallet(ctx: Context):
    try:
        # Get my account info
        info = accounts.get_accounts(ctx)

        # Simplify the data into my wallet struct
        wallet = {}
        for currency in ['BTC', 'USD', 'all']:
            wallet[currency] = {
                'hold': 0,
                'available': 0.0,
                'total': 0.0,
            }
        for account in info['accounts']:
            currency = account['currency']
            if currency in ['BTC', 'USD']:
                wallet[currency]['hold'] += float(account['hold']['value'])
                wallet[currency]['available'] += float(account['available_balance']['value'])
        for currency in wallet:
            w = wallet[currency]
            w['total'] += w['hold'] + w['available']

        # Current price
        bid = CurrentMarket.get(ctx).bid

        # Get totals in USD
        for x in ['hold', 'available', 'total']:
            wallet['BTC'][x + "$"] = wallet['BTC'][x] * bid
            wallet['all'][x] = wallet['BTC'][x + "$"] + wallet['USD'][x]

        return wallet
    except Exception as e:
        Log.error("Failed to get wallet: {}".format(str(e)))
        return None

def wallets_equal(lhs, rhs):
    LHS = copy.deepcopy(lhs)
    RHS = copy.deepcopy(rhs)
    try:
        del LHS['all']
    except:
        pass
    try:
        del RHS['all']
    except:
        pass
    for x in ['hold', 'available', 'total']:
        try:
            del LHS['BTC'][x + '$']
        except:
            pass
        try:
            del RHS['BTC'][x + '$']
        except:
            pass
    return LHS == RHS
