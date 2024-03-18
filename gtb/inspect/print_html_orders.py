# Print active orderbook as an HTML table

from gtb.core.context import Context
from gtb.orders.order_book import OrderBook
from gtb.orders.order_pair import OrderPair
from gtb.market.current import get_current_market
from gtb.market.prices import MarketPrices
from gtb.utils.maths import floor_usd
from gtb.utils.logging import Log

Log.INFO = False

ctx: Context = Context()
ctx.order_book.read_fs()

market: MarketPrices = get_current_market(ctx)

# Table Header
print("<TABLE border=1 cellpadding=5>")
print("  <TR>")
for col in ["Status", "Buy", "Sell", "Algorithm", "Position"]:
    print("    <TH>{}</TH>".format(col))
print("  </TR>")

# Sort orders by price
def sort_price(pair: OrderPair) -> float:
    if pair.sell and pair.status in [
        OrderPair.Status.OnHoldSell,
        OrderPair.Status.PendingSell,
        OrderPair.Status.ActiveSell]:
        return pair.sell.get_limit_price()
    elif pair.buy.info and pair.buy.info.final_market:
        return pair.buy.info.final_market
    else:
        return pair.buy.get_limit_price()
ctx.order_book.order_pairs.sort(key=sort_price)

def print_market() -> None:
    print("  <TR>")
    print("    <TD><B>{}</B></TD>".format("Market"))
    print("    <TD>{:.2f}</TD>".format(floor_usd(market.bid)))
    print("    <TD>{:.2f}</TD>".format(floor_usd(market.ask)))
    print("    <TD>{}</TD>".format("&nbsp;"))
    print("    <TD>{}</TD>".format("&nbsp;"))
    print("  </TR>")

market_printed: bool = False
for pair in ctx.order_book.order_pairs:
    price: float = sort_price(pair)

    if not market_printed and price > market.split:
        market_printed = True
        print_market()

    print("  <TR>")
    print("    <TD>{}</TD>".format(pair.status.name))
    print("    <TD>{:.2f}</TD>".format(pair.buy.get_limit_price()))
    if pair.sell:
        print("    <TD>{:.2f}</TD>".format(pair.sell.get_limit_price()))
    else:
        print("    <TD>&nbsp;</TD>")
    print("    <TD>{}</TD>".format(pair.algorithm))
    print("    <TD>${:.2f}</TD>".format(pair.buy.usd))
    print("  </TR>")

if not market_printed:
    print_market()

print("</TABLE>")
