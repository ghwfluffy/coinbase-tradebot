import json

from datetime import datetime
from typing import List, Tuple

from gtb.market.volume import MarketVolume

market_demand: List[Tuple[str, float]] = []
with open("data/volume.jsonl") as fp:
    for line in fp:
        cur_volume: MarketVolume = MarketVolume(json.loads(line))
        demand_pos: int = 1
        while True:
            try:
                demand_value: float = cur_volume.get_bids(demand_pos) / cur_volume.get_asks(demand_pos)
                break
            except:
                demand_pos += 1
        below: bool = demand_value < 1
        while (below and demand_value < 1) or (not below and demand_value > 1):
            demand_pos += 5
            asks: float = cur_volume.get_asks(demand_pos)
            if asks == 0:
                break
            demand_value = cur_volume.get_bids(demand_pos) / asks
            if demand_pos > 2000:
                break
        if below:
            demand_pos *= -1
        print("{},{}".format(cur_volume.pricebook['time'], demand_pos))
        market_demand.append((cur_volume.pricebook['time'], demand_pos))

with open("data/demand.csv", "w") as fp:
    for demand in market_demand:
        fp.write("{},{}\n".format(demand[0], demand[1]))
