import json

from comm.rest import RestIfx

ifx = RestIfx()
rsp = ifx.post('get_ticker', { "pairs": ["BTC-USDC"] })
print(rsp)

rsp = ifx.post('get_order_book', {"pair": "BTC-USDC"})
print("")
print(json.dumps(rsp, indent=2))
