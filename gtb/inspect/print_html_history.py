# Print active orderbook as an HTML table

from gtb.core.context import Context
from gtb.orders.order_book import OrderBook
from gtb.orders.order_pair import OrderPair
from gtb.market.current import get_current_market
from gtb.market.prices import MarketPrices
from gtb.utils.maths import floor_usd
from gtb.utils.logging import Log

MAX_TRANSACTIONS: int = 20

Log.INFO = False

ctx: Context = Context()
ctx.history.read_fs()

# Table Header
print("<TABLE border=1 cellpadding=5>")
print("  <TR>")
for col in ["Algorithm", "Buy", "Sell", "Fees", "Total"]:
    print("    <TH>{}</TH>".format(col))
print("  </TR>")

count = 0
for pair in reversed(ctx.history.order_pairs):
    if pair.status != OrderPair.Status.Complete:
        continue

    assert pair.buy.info is not None
    assert pair.buy.info.final_usd is not None
    assert pair.buy.info.final_fees is not None

    print("  <TR>")
    print("    <TD>{}</TD>".format(pair.algorithm))
    print("    <TD>${:.2f}</TD>".format(pair.buy.info.final_usd))
    if pair.sell and pair.sell.info and pair.sell.info.final_usd:
        assert pair.sell is not None
        assert pair.sell.info is not None
        assert pair.sell.info.final_usd is not None
        assert pair.sell.info.final_fees is not None
        fees: float = pair.sell.info.final_fees + pair.buy.info.final_fees
        total: float = pair.sell.info.final_usd - pair.buy.info.final_usd - fees
        print("    <TD>${:.2f}</TD>".format(pair.sell.info.final_usd))
        print("    <TD>${:.2f}</TD>".format(fees))
        print("    <TD>${:.2f}</TD>".format(total))
    else:
        print("    <TD>&nbsp;</TD>")
        print("    <TD>${:.2f}</TD>".format(pair.buy.info.final_fees))
        print("    <TD>&nbsp;</TD>")
    print("  </TR>")

    count += 1
    if count >= MAX_TRANSACTIONS:
        break

print("</TABLE>")
