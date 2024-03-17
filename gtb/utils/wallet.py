from typing import Optional

from gtb.utils.logging import Log
from gtb.core.context import Context

from coinbase.rest import accounts as account_api

class Wallet():
    usd_hold: float
    usd_available: float
    btc_hold: float
    btc_available: float

    def __init__(self) -> None:
        self.usd_hold = 0.0
        self.usd_available = 0.0
        self.btc_hold = 0.0
        self.btc_available = 0.0

    @staticmethod
    def get(ctx: Context) -> Optional['Wallet']:
        try:
            ret: Wallet = Wallet()
            info: dict = account_api.get_accounts(ctx.api)
            for account in info['accounts']:
                currency = account['currency']
                if currency == "BTC":
                    ret.btc_hold += float(account['hold']['value'])
                    ret.btc_available += float(account['available_balance']['value'])
                elif currency == "USD":
                    ret.usd_hold += float(account['hold']['value'])
                    ret.usd_available += float(account['available_balance']['value'])
            return ret
        except Exception as e:
            Log.exception("Failed to get wallet", e)
        return None
