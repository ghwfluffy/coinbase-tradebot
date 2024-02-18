import json

from context import Context
from utils.wallet import get_wallet

# New Coinbase connection context
ctx: Context = Context()
wallet = get_wallet(ctx)
print(json.dumps(wallet, indent=2))
