import os
import json

from typing import List, Callable
from threading import RLock

from gtb.orders.order_pair import OrderPair
from gtb.utils.logging import Log

class OrderBook():
    file: str = "data/historical.json"

    order_pairs: List[OrderPair]
    mtx: RLock

    def __init__(self) -> None:
        self.order_pairs = []
        self.mtx = RLock()

    def read_fs(self) -> None:
        with self.mtx:
            if not os.path.exists(OrderBook.file):
                Log.info("No orderbook data.")
                return None

            # Read file
            str_data: str
            with open(OrderBook.file, "r") as fp:
                str_data = fp.read()

            # JSON deserialize
            data: list = json.loads(str_data)

            # Interpret
            for pair in data:
                cur: OrderPair | None = OrderPair.from_dict(pair)
                if cur:
                    self.order_pairs.append(cur)

            Log.info("Read {} orderbook pairs.".format(len(self.order_pairs)))

    def write_fs(self) -> None:
        with self.mtx:
            # Serialize to dictionary
            data: list = []
            for pair in self.order_pairs:
                data.append(pair.to_dict())
            # Serialize to string
            str_data: str = json.dumps(data)

            # Create directory
            if not os.path.exists(os.path.dirname(OrderBook.file)):
                os.makedirs(os.path.dirname(OrderBook.file))

            # Write
            with open(OrderBook.file, "w") as fp:
                fp.write(str_data)

    def append(self, order_pair: OrderPair) -> None:
        with self.mtx:
            self.order_pairs.append(order_pair)
            self.write_fs()

    # Remove completed/canceled pairs
    def cleanup(self) -> None:
        with self.mtx:
            i: int = 0
            while i < len(self.order_pairs):
                pair: OrderPair = self.order_pairs[i]
                if pair.status in [OrderPair.Status.Complete, OrderPair.Status.Canceled]:
                    Log.info("Clearing {} order pair for {}.".format(
                        pair.status.name,
                        pair.algorithm,
                    ))
                    self.order_pairs.pop(i)
                else:
                    i += 1

    def get_ids(self) -> List[int]:
        ret: List[int] = []
        with self.mtx:
            for pair in self.order_pairs:
                ret.append(id(pair))
        return ret

    def process_id(self, pair_id: int, func: Callable) -> bool:
        with self.mtx:
            for pair in self.order_pairs:
                if id(pair) == pair_id:
                    with pair.mtx:
                        func(pair)
                    return True
        return False
