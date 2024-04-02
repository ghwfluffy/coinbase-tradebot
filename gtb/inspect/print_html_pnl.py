# Print profits and losses by algorithm as an HTML table

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
ctx.history.read_fs()
ctx.history.prune("2024-03-26 15:30:00")

market: MarketPrices = get_current_market(ctx)

# Table Header
print("<TABLE border=1 cellpadding=5>")
print("  <TR>")
for col in ["Algorithm", "Finalized", "Pending", "Total"]:
    print("    <TH>{}</TH>".format(col))
print("  </TR>")

hodl_usd: float = 0.0
hodl_btc: float = 0.0

spread_finalized: float = 0.0

# Read in finalized orders
for pair in ctx.history.order_pairs:
    if pair.status != OrderPair.Status.Complete:
        continue

    assert pair.buy.info is not None
    assert pair.buy.info.final_usd is not None
    assert pair.buy.info.final_fees is not None

    algorithm = pair.algorithm.split("-")[0]
    if algorithm == "HODL":
        hodl_usd += pair.buy.info.final_usd
        #hodl_usd -= pair.buy.info.final_fees
        hodl_btc += pair.buy.btc
        continue

    assert pair.sell is not None
    assert pair.sell.info is not None
    assert pair.sell.info.final_usd is not None
    assert pair.sell.info.final_fees is not None

    if algorithm == "Spread":
        spread_finalized -= pair.buy.info.final_usd
        #spread_finalized -= pair.buy.info.final_fees
        spread_finalized += pair.sell.info.final_usd
        spread_finalized -= pair.sell.info.final_fees

# Read in pending orders
spread_pending_btc: float = 0.0
spread_pending_usd: float = 0.0
for pair in ctx.order_book.order_pairs:
    if not pair.status in [
        OrderPair.Status.OnHoldSell,
        OrderPair.Status.PendingSell,
        OrderPair.Status.ActiveSell,
    ]:
        continue

    assert pair.buy.info is not None
    assert pair.buy.info.final_usd is not None
    assert pair.buy.info.final_fees is not None

    algorithm = pair.algorithm.split("-")[0]
    if algorithm == "Spread":
        spread_pending_btc += pair.buy.btc
        spread_pending_usd += pair.buy.info.final_usd
        #spread_pending_usd += pair.buy.info.final_fees

# Print HODL
hodl_current: float = hodl_btc * market.bid
hodl_fees: float = hodl_current * 0.0015
hodl_final: float = hodl_current - hodl_usd - hodl_fees
print("  <TR>")
print("    <TD>HODL</TD>")
print("    <TD>${:.2f}</TD>".format(hodl_final))
print("    <TD>&nbsp;</TD>")
print("    <TD>${:.2f}</TD>".format(hodl_final))
print("  </TR>")

# Print spread
spread_to_sell: float = spread_pending_btc * market.bid
spread_to_sell_fees: float = spread_to_sell * 0.0015
spread_pending: float = spread_to_sell - spread_pending_usd - spread_to_sell_fees
print("  <TR>")
print("    <TD>Spread</TD>")
print("    <TD>${:.2f}</TD>".format(spread_finalized))
print("    <TD>${:.2f}</TD>".format(spread_pending))
print("    <TD>${:.2f}</TD>".format(spread_finalized + spread_pending))
print("  </TR>")

print("  <TR>")
print("    <TD>Total</TD>")
print("    <TD>${:.2f}</TD>".format(hodl_final + spread_finalized))
print("    <TD>${:.2f}</TD>".format(spread_pending))
print("    <TD><B>${:.2f}</B></TD>".format(hodl_final + spread_finalized + spread_pending))
print("  </TR>")


# End table
print("</TABLE>")
