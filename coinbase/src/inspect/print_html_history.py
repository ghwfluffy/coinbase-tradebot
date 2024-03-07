import json
from utils.maths import floor_usd, floor_btc

with open("historical.json", "r") as fp:
    orders = json.loads(fp.read())

cols = ["Time", "Tranche", "Buy BTC", "Sell BTC", "Buy Price", "Sell Price", "Buy USD", "Sell USD", "Fees", "Net"]

print("<TABLE border=1 cellpadding=5><TR>")
for col in cols:
    print("<TH>{}</TH>".format(col))
print("</TR>")

keep = []
for order in orders:
    if order['buy']['status'] == 'Canceled':
        continue

    if order['sell']['status'] == 'Canceled':
        continue

    keep.append(order)

keep.sort(key=lambda x:x['sell']['final_time'], reverse=True)
orders = keep

for order in orders:
    buy = float(order['buy']['usd'])
    sell = float(order['sell']['usd'])
    fees = float(order['buy']['final_fees']) + float(order['sell']['final_fees'])

    print("<TR>")
    print("<TD>{}</TD>".format(order['sell']['final_time']))
    print("<TD>{}</TD>".format(order['tranche']))
    print("<TD>{:.8f}</TD>".format(floor_btc(order['buy']['btc'])))
    print("<TD>{:.8f}</TD>".format(floor_btc(order['sell']['btc'])))
    print("<TD>{:.2f}</TD>".format(floor_usd(order['buy']['final_market'])))
    print("<TD>{:.2f}</TD>".format(floor_usd(order['sell']['final_market'])))
    print("<TD>{:.2f}</TD>".format(floor_usd(buy)))
    print("<TD>{:.2f}</TD>".format(floor_usd(sell)))
    print("<TD>{:.2f}</TD>".format(floor_usd(fees)))
    print("<TD>{:.2f}</TD>".format(floor_usd(sell - buy - fees)))
    print("</TR>")

print("</TABLE>")
