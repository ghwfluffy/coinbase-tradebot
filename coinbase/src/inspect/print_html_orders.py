import json
from context import Context
from market.current import CurrentMarket
from utils.maths import floor_usd, floor_btc

ctx: Context = Context()
market: CurrentMarket | None = CurrentMarket.get(ctx)
if not market:
    print("Failed to get market data.")
    exit(1)

with open("orderbook.json", "r") as fp:
    data = json.loads(fp.read())

cols = ["Status", "Market", "Buy", "Sell", "Tranche"]

print("<TABLE border=1 cellpadding=5><TR>")
for col in cols:
    print("<TH>{}</TH>".format(col))
print("</TR>")

def sort_price(x):
    if x['status'] == 'PendingSell':
        return float(x['sell']['market'])
    else:
        buy = floor_usd(x['buy']['final_market'] if x['buy']['final_market'] else x['buy']['market'])
        return buy

data['orders'].sort(key=sort_price)

def print_market():
    print("<TR>")
    print("<TD>{}</TD>".format("Market"))
    print("<TD><B>{:.2f}</B></TD>".format(floor_usd(market.split)))
    print("<TD>{:.2f}</TD>".format(floor_usd(market.bid)))
    print("<TD>{:.2f}</TD>".format(floor_usd(market.ask)))
    print("<TD>{}</TD>".format("&nbsp;"))
    print("</TR>")

market_printed = False
for order in data['orders']:
    buy = floor_usd(order['buy']['final_market'] if order['buy']['final_market'] else order['buy']['market'])

    if not market_printed and sort_price(order) > market.bid:
        market_printed = True
        print_market()

    print("<TR>")
    print("<TD>{}</TD>".format(order['status']))
    print("<TD>{:.2f}</TD>".format(floor_usd(order['event_price'])))
    print("<TD>{:.2f}</TD>".format(buy))
    print("<TD>{:.2f}</TD>".format(floor_usd(order['sell']['market'])))
    print("<TD>{}</TD>".format(order['tranche']))
    print("</TR>")

if not market_printed:
    print_market()

print("</TABLE>")
