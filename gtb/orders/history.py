import os
import json

from typing import List
from threading import RLock

from gtb.orders.order import Order
from gtb.utils.logging import Log

class OrderHistory():
    file: str = "data/historical.json"

    orders: List[Order]
    mtx: RLock

    def __init__(self):
        self.orders = []
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
                cur: Order | None = Order.from_dict(order)
                if cur:
                    self.orders.append(cur)

            Log.info("Read {} historical orders.".format(len(self.orders)))

    def write_fs(self) -> None:
        with self.mtx:
            # Serialize to dictionary
            data: list = []
            for order in self.orders:
                data.append(order.to_dict())
            # Serialize to string
            str_data: str = json.dumps(data)

            # Create directory
            if not os.path.exists(os.path.dirname(OrderHistory.file)):
                os.makedirs(os.path.dirname(OrderHistory.file))

            # Write
            with open(OrderHistory.file, "w") as fp:
                fp.write(str_data)

    def append(self, order: Order) -> None:
        with self.mtx:
            self.orders.append(order)
            self.write_fs()
