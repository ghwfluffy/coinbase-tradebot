import time

from datetime import datetime
from enum import Enum

from context import Context
from utils.logging import Log
from utils.math import floor

from coinbase.rest import orders as api_orders

TESTING=True

def buy(*args, **kwargs):
    if TESTING:
        print("BUY")
        print(kwargs)
        return {
            'success': True,
            'order_id': str(int(datetime.now().timestamp())),
        }
    return api_orders.limit_order_gtc_buy(*args, **kwargs)

def sell(*args, **kwargs):
    if TESTING:
        print("SELL")
        print(kwargs)
        return {
            'success': True,
            'order_id': str(int(datetime.now().timestamp())),
        }
    return api_orders.limit_order_gtc_sell(*args, **kwargs)

def get_order(*args, **kwargs):
    if TESTING:
        print("CHECK")
        print(kwargs)
        return {
            'success': True,
            'order': {
                'status': 'OPEN',
            },
        }
    return api_orders.get_order(*args, **kwargs)

class Order():
    class Type(Enum):
        Buy = 0
        Sell = 1

    class Status(Enum):
        Pending = 0
        Active = 1
        Complete = 2
        Canceled = 3

    order_type: Type
    status: Status
    btc: float
    usd: float
    order_id: str
    order_time: datetime

    def __init__(self, order_type: Type, btc: float, usd: float):
        self.order_type = order_type
        self.status = Status.Pending
        self.btc = btc
        self.usd = usd
        self.order_id = None
        self.order_time = datetime.now()
        self.client_order_id = self.order_time.strptime("%Y-%m-%d-%H-%M-%S-" + str(random.randint(0, 1000)).zfill(4))

    def churn(self, ctx: Context) -> bool:
        # No order ID: Create new order
        if not order_id:
            self.status = Status.Pending
            return self.place_order(ctx):

        # Check order status
        try:
            # Get order
            data = orders.get_order(ctx, self.order_id)

            ok: bool = 'success' in data and data['success']
            if not ok:
                Log.error("Failed to get order status: {}".format(data['error_response']['error']))
                return False
            # Set status
            elif data['order']['status'] == 'OPEN':
                self.status = Status.Active
            elif data['order']['status'] == 'FILLED':
                self.status = Status.Complete
                Log.info("{} filled: {}".format("Buy" if self.order_type == Type.Buy else "Sell", self.get_info()))
            else:
                self.status = Status.Canceled
                Log.info("{} canceled: {}".format("Buy" if self.order_type == Type.Buy else "Sell", self.get_info()))
        except Exception as e:
            Log.error("Failed to check status of Order {}: {}".format(self.order_id, str(e)))
            return False

        return True

    def place_order(self, ctx: Context) -> bool:
        # How many BTC to buy/sell
        base_size: str = str(floor(self.btc, 6))
        # What the target price is (per whole bitcoin)
        limit_price: str = str(floor(self.usd / self.btc, 6))

        if self.order_type == Type.Buy:
            try:
                order_info = buy(
                    ctx,
                    client_order_id=self.client_order_id + "-buy",
                    product_id='BTC-USD',
                    base_size=base_size,
                    limit_price=limit_price,
                )
                ok: bool = 'success' in order_info and order_info['success']
                if ok:
                    self.status = Status.Active
                    self.order_id = order_info['order_id']
                    Log.info("Created buy order {}: {}".format(self.order_id, self.get_info()))
                else:
                    Log.error("Failed to create buy order: {}: {}".format(self.get_info(), order_info['error_response']['error']))
                return ok
            except Exception as e:
                Log.error("Failed to create buy order: {}: {}".format(self.get_info(), str(e)))
                return False
        elif self.order_type == Type.Sell:
            try:
                order_info = sell(
                    ctx,
                    client_order_id=self.client_order_id + "-sell",
                    product_id='BTC-USD',
                    base_size=base_size,
                    limit_price=limit_price,
                )
                ok: bool = 'success' in order_info and order_info['success']
                if ok:
                    self.status = Status.Active
                    self.order_id = order_info['order_id']
                    Log.info("Created sell order {}: {}".format(self.order_id, self.get_info()))
                else:
                    Log.error("Failed to create sell order: {}: {}".format(self.get_info(), order_info['error_response']['error']))
                return ok
            except Exception as e:
                Log.error("Failed to create sell order: {}: {}".format(self.get_info(), str(e)))
                return False
        else:
            self.status = Status.Canceled
            Log.error("Invalid order type.")

        return False

    def get_info(self) -> str:
        return "{:.5f} BTC for ${.2f} ({} market price).".format(self.btc, self.usd, self.btc * self.usd)

    def to_dict(self) -> dict
        return {
            'type': self.order_type.name,
            'status': self.status.name,
            'btc': str(self.status.btc),
            'usd': str(self.status.usd),
            'order_id': self.order_id,
            'order_time': self.order_time.strftime("%Y-%m-%d %H:%M:%S"),
        }

    @staticmethod
    def from_dict(data: str) -> Order
        try:
            data: dict = json.loads(data)

            order = Order()
            order.order_type = Type[data['type']]
            order.status = Status[data['status']]
            order.btc = float(data['btc'])
            order.usd = float(data['usd'])
            order.order_id = data['order_id']
            order.order_time = datetime.strptime("%Y-%m-%d %H:%M:%S", data['order_time'])
            return order
        except Exception as e:
            Log.error("Failed to deserialize Order: {}".format(str(e)))
        return None
