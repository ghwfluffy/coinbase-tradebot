# Print profits and losses by algorithm as an HTML table

from gtb.core.context import Context
from gtb.orders.order_book import OrderBook
from gtb.orders.order_pair import OrderPair
from gtb.market.current import get_current_market
from gtb.market.prices import MarketPrices
from gtb.utils.maths import floor_usd
from gtb.utils.logging import Log
from gtb.utils.wallet import Wallet

Log.INFO = False

ctx: Context = Context()
ctx.order_book.read_fs()
ctx.history.read_fs()

wallet: Wallet | None = Wallet.get(ctx)
assert wallet is not None

market: MarketPrices = get_current_market(ctx)

# Table Header
print("<TABLE border=1 cellpadding=5>")
print("  <TR>")
for col in ["Algorithm", "BTC", "BTC@USD", "USD", "Total"]:
    print("    <TH>{}</TH>".format(col))
print("  </TR>")

# Read in BTC we're holding permanently
hodl_btc: float = 0.0
for pair in ctx.history.order_pairs:
    if pair.status != OrderPair.Status.Complete:
        continue

    assert pair.buy.info is not None
    assert pair.buy.info.final_usd is not None
    assert pair.buy.info.final_fees is not None

    algorithm = pair.algorithm.split("-")[0]
    if algorithm == "HODL":
        hodl_btc += pair.buy.btc
        continue

# Amount of USD locked in active HODLs
hodl_usd: float = 0.0
# Amount of BTC pending sell for spreads
spread_btc: float = 0.0
spread_btc_hold: float = 0.0
# Amount of USD locked in active spreads
spread_usd: float = 0.0

# Read in active orders
for pair in ctx.order_book.order_pairs:
    # Bitcoins to be sold
    if pair.status in [
        OrderPair.Status.OnHoldSell,
        OrderPair.Status.PendingSell,
        OrderPair.Status.ActiveSell,
    ]:
        if algorithm == "Spread":
            spread_btc += pair.buy.btc
            if pair.status == OrderPair.Status.ActiveSell:
                spread_btc_hold += pair.buy.btc

    # USD locked in active trade
    elif pair.status in [
        OrderPair.Status.Active,
    ]:
        if algorithm == "Spread":
            spread_usd += pair.buy.usd
        elif algorithm == "HODL":
            hodl_usd += pair.buy.usd

# Spread
spread_btc_usd: float = floor_usd(spread_btc * market.bid)
print("  <TR>")
print("    <TD>Spread</TD>")
print("    <TD>{:.8f}</TD>".format(spread_btc))
print("    <TD>${:.2f}</TD>".format(spread_btc_usd))
print("    <TD>${:.2f}</TD>".format(spread_usd))
print("    <TD>${:.2f}</TD>".format(spread_usd + spread_btc_usd))
print("  </TR>")

# HODL
hodl_btc_usd: float = floor_usd(hodl_btc * market.bid)
print("  <TR>")
print("    <TD>HODL</TD>")
print("    <TD>{:.8f}</TD>".format(hodl_btc))
print("    <TD>${:.2f}</TD>".format(hodl_btc_usd))
print("    <TD>${:.2f}</TD>".format(hodl_usd))
print("    <TD>${:.2f}</TD>".format(hodl_usd + hodl_btc_usd))
print("  </TR>")

# Out-of-band
other_btc: float = wallet.btc_hold - spread_btc_hold
other_btc_usd: float = floor_usd(other_btc * market.bid)
other_usd: float = wallet.usd_hold - spread_usd - hodl_usd
print("  <TR>")
print("    <TD>Manual</TD>")
print("    <TD>{:.8f}</TD>".format(other_btc))
print("    <TD>${:.2f}</TD>".format(other_btc_usd))
print("    <TD>${:.2f}</TD>".format(other_usd))
print("    <TD>${:.2f}</TD>".format(other_usd + other_btc_usd))
print("  </TR>")

# Orphans
orphan_btc: float = wallet.btc_available - (spread_btc - spread_btc_hold) - hodl_btc
orphan_btc_usd: float = floor_usd(orphan_btc * market.bid)
orphan_usd: float = wallet.usd_available
print("  <TR>")
print("    <TD>Unallocated</TD>")
print("    <TD>{:.8f}</TD>".format(orphan_btc))
print("    <TD>${:.2f}</TD>".format(orphan_btc_usd))
print("    <TD>${:.2f}</TD>".format(orphan_usd))
print("    <TD>${:.2f}</TD>".format(orphan_usd + orphan_btc_usd))
print("  </TR>")

# Total
total_btc: float = wallet.btc_hold + wallet.btc_available
total_btc_usd: float = floor_usd(total_btc * market.bid)
total_usd: float = wallet.usd_hold + wallet.usd_available
print("  <TR>")
print("    <TD>Total</TD>")
print("    <TD>{:.8f}</TD>".format(total_btc))
print("    <TD>${:.2f}</TD>".format(total_btc_usd))
print("    <TD>${:.2f}</TD>".format(total_usd))
print("    <TD><B>${:.2f}</B></TD>".format(total_usd + total_btc_usd))
print("  </TR>")

# End table
print("</TABLE>")
