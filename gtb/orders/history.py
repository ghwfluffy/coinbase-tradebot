import os
import json

from typing import List
from threading import RLock
from datetime import datetime

from gtb.orders.order_pair import OrderPair
from gtb.utils.logging import Log

class OrderHistory():
    file: str = "data/historical.json"

    order_pairs: List[OrderPair]
    mtx: RLock

    def __init__(self) -> None:
        self.order_pairs = []
        self.mtx = RLock()

    def read_fs(self) -> None:
        with self.mtx:
            if not os.path.exists(OrderHistory.file):
                Log.info("No historical data.")
                return None

            # Read file
            str_data: str
            with open(OrderHistory.file, "r") as fp:
                str_data = fp.read()

            # JSON deserialize
            data: list = json.loads(str_data)

            # Interpret
            for order in data:
                cur: OrderPair | None = OrderPair.from_dict(order)
                if cur:
                    self.order_pairs.append(cur)

            Log.info("Read {} historical order pairs.".format(len(self.order_pairs)))

    def write_fs(self) -> None:
        with self.mtx:
            # Serialize to dictionary
            data: list = []
            for order in self.order_pairs:
                data.append(order.to_dict())
            # Serialize to string
            str_data: str = json.dumps(data)

            # Create directory
            if not os.path.exists(os.path.dirname(OrderHistory.file)):
                os.makedirs(os.path.dirname(OrderHistory.file))

            # Write
            with open(OrderHistory.file, "w") as fp:
                fp.write(str_data)

    def append(self, order: OrderPair) -> None:
        with self.mtx:
            self.order_pairs.append(order)
            self.write_fs()

    def prune(self, oldest: str) -> None:
        with self.mtx:
            oldest_date: datetime = datetime.strptime(oldest, "%Y-%m-%d %H:%M:%S")
            idx = 0
            while idx < len(self.order_pairs):
                if self.order_pairs[idx].event_time < oldest_date:
                    self.order_pairs.pop(idx)
                else:
                    idx += 1
