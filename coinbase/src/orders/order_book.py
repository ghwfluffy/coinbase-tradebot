import os
import json
import shutil

from context import Context
from orders.order_pair import OrderPair
from market.current import CurrentMarket
from utils.logging import Log

class OrderBook():
    orders: list[OrderPair]
    iter_index: int = 0

    PERSISTENT_STORAGE = "./orderbook.json"

    def __init__(self):
        self.orders = []
        self.iter_index = 0

    @staticmethod
    def read_fs() -> 'OrderBook':
        book = OrderBook()
        # No saved state
        if not os.path.exists(OrderBook.PERSISTENT_STORAGE):
            return book

        # Read in file
        with open("orderbook.json", "r") as fp:
            data = fp.read()

        # Deserialize JSON string
        data = json.loads(data)

        # Check version
        if not 'version' in data or data['version'] != 1:
            Log.error("Orderbook JSON file version mismatch.")
        else:
            # Deserialize each order pair
            for order in data['orders']:
                new_order = OrderPair.from_dict(order)
                if new_order:
                    book.orders.append(new_order)

        return book

    def write_fs(self) -> None:
        # Serialize
        orders = []
        for order in self.orders:
            orders.append(order.to_dict())
        data = {
            'version': 1,
            'orders': orders,
        }

        # Write to file
        try:
            with open("orderbook.json", "w") as fp:
                fp.write(json.dumps(data))
        except Exception as e:
            Log.error("Failed to write orderbook state: {}.".format(str(e)))

    def __iter__(self):
        self.iter_index = 0
        return self
    
    def __next__(self) -> OrderPair:
        if self.iter_index < len(self.orders):
            order: Order = self.orders[self.iter_index]
            self.iter_index += 1
            return order
        raise StopIteration

    def churn(self, ctx: Context) -> bool:
        # Get the current market spread
        market: CurrentMarket = CurrentMarket.get(ctx)
        if market == None:
            return False

        ret = True
        for order in self.orders:
            ret &= order.churn(ctx, market)
        return ret

    def cleanup(self, ctx: Context) -> None:
        i = 0
        while i < len(self.orders):
            pair: OrderPair = self.orders[i]
            if pair.status == OrderPair.Status.Complete:
                Log.debug("Cleanup completed order pair from {}.".format(pair.target_id))
                self.orders.pop(i)
                self.write_historical(pair)
            elif pair.status == OrderPair.Status.Canceled and pair.cancel(ctx):
                Log.debug("Cleanup canceled order pair from {}.".format(pair.target_id))
                self.orders.pop(i)
                self.write_historical(pair)
            else:
                i += 1

    def write_historical(self, pair: OrderPair) -> None:
        try:
            # Read in file
            with open("historical.json", "r") as fp:
                data = fp.read()

            # Deserialize JSON string
            data = json.loads(data)

            # Append
            data.append(pair.to_dict())

            # Write
            with open("historical-updated.json", "w") as fp:
                fp.write(json.dumps(data))

            # Careful not to lose any data
            shutil.move("historical.json", "historical-bak.json")
            shutil.move("historical-updated.json", "historical.json")
        except Exception as e:
            Log.error("Failed to save historical data: {}".format(str(e)))
