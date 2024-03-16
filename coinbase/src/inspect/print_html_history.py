import json
from datetime import datetime
from utils.maths import floor_usd, floor_btc

START_TIME="2024-03-15 03:30:00"

def parse_date(x):
    return datetime.strptime(x, "%Y-%m-%d %H:%M:%S")

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

    if order['sell'] and order['sell']['status'] == 'Canceled':
        continue

    keep.append(order)

keep.sort(key=lambda x:x['sell']['final_time'] if x['sell'] else x['buy']['final_time'], reverse=True)
orders = keep

for order in orders:
    if parse_date(START_TIME) > parse_date(order['event_time']):
        continue

    buy = float(order['buy']['usd'])
    sell: float = 0
    fees: float = 0
    fees += float(order['buy']['final_fees'])
    if order['sell']:
        sell = float(order['sell']['usd'])
        fees += float(order['sell']['final_fees'])

    print("<TR>")
    if order['sell']:
        print("<TD>{}</TD>".format(order['sell']['final_time']))
    else:
        print("<TD>{}</TD>".format(order['buy']['final_time']))
    print("<TD>{}</TD>".format(order['tranche']))
    print("<TD>{:.8f}</TD>".format(floor_btc(order['buy']['btc'])))
    if order['sell']:
        print("<TD>{:.8f}</TD>".format(floor_btc(order['sell']['btc'])))
    else:
        print("<TD>&nbsp;</TD>")
    print("<TD>{:.2f}</TD>".format(floor_usd(order['buy']['final_market'])))
    if sell > 0:
        print("<TD>{:.2f}</TD>".format(floor_usd(order['sell']['final_market'])))
    else:
        print("<TD>&nbsp;</TD>")
    print("<TD>{:.2f}</TD>".format(floor_usd(buy)))
    if sell > 0:
        print("<TD>{:.2f}</TD>".format(floor_usd(sell)))
    else:
        print("<TD>&nbsp;</TD>")
    print("<TD>{:.2f}</TD>".format(floor_usd(fees)))
    if sell > 0:
        print("<TD>{:.2f}</TD>".format(floor_usd(sell - buy - fees)))
    else:
        print("<TD>&nbsp;</TD>")
    print("</TR>")

print("</TABLE>")
