# Print profits and losses by algorithm as an HTML table

from gtb.core.context import Context
from gtb.orders.history import OrderHistory
from gtb.orders.order_pair import OrderPair
from gtb.utils.logging import Log

from typing import Dict

Log.INFO = False

ctx: Context = Context()
ctx.history.read_fs()
#ctx.history.prune("2024-03-26 15:30:00")

# Table Header
print("<TABLE border=1 cellpadding=5>")
print("  <TR>")
for col in ["Spread", "Wins", "Losses", "Total"]:#, "Average"]:
    print("    <TH>{}</TH>".format(col))
print("  </TR>")

class Stats():
    wins: int
    wins_delta: float
    losses: int
    losses_delta: float

    def __init__(self) -> None:
        self.wins_delta = 0.0
        self.wins = 0
        self.losses_delta = 0.0
        self.losses = 0

spread_stats: Dict[str, Stats] = {}

def add_stats(spread: str, total: float) -> None:
    if not spread in spread_stats:
        spread_stats[spread] = Stats()
    if total > 0:
        spread_stats[spread].wins += 1
        spread_stats[spread].wins_delta += total
    else:
        spread_stats[spread].losses += 1
        spread_stats[spread].losses_delta += total

# Read in finalized orders
for pair in ctx.history.order_pairs:
    if pair.status != OrderPair.Status.Complete:
        continue

    fields = pair.algorithm.split("-")
    if fields[0] != "Spread":
        continue

    assert pair.buy.info is not None
    assert pair.buy.info.final_usd is not None
    assert pair.buy.info.final_fees is not None
    assert pair.sell is not None
    assert pair.sell.info is not None
    assert pair.sell.info.final_usd is not None
    assert pair.sell.info.final_fees is not None

    total: float = pair.sell.info.final_usd - pair.sell.info.final_fees - pair.buy.info.final_usd - pair.buy.info.final_fees
    add_stats(fields[1], total)
    if len(fields) > 2:
        add_stats("-".join(fields[1:3]), total)

keys: list[str] = list(spread_stats.keys())
keys.sort()
#keys.sort(key=lambda x:(spread_stats[x].wins + spread_stats[x].losses) * -1)

for spread in keys:
    stats: Stats = spread_stats[spread]
    #if stats.wins < 2:
    #    continue
    total_count: int = stats.wins + stats.losses
    total_delta: float = stats.wins_delta + stats.losses_delta
    #average: float = total_delta / stats.wins
    #average: float = total_delta / total_count
    print(f"  <TR>")
    print(f"    <TD>{spread}</TD>")
    print(f"    <TD>${stats.wins_delta:.2f} ({stats.wins})</TD>")
    print(f"    <TD>${stats.losses_delta:.2f} ({stats.losses})</TD>")
    print(f"    <TD>${total_delta:.2f} ({total_count})</TD>")
    #print(f"    <TD>${average:.2f}</TD>")
    print(f"  </TR>")

# End table
print("</TABLE>")
