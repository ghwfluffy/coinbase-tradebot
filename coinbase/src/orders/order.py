import time
import json
import random

from datetime import datetime
from enum import Enum

from context import Context
from utils.logging import Log
from utils.math import floor_btc, floor_usd

from coinbase.rest import orders as api_orders

TESTING=False
#TESTING=True

def buy(*args, **kwargs):
    if TESTING:
        print("BUY")
        print(json.dumps(kwargs, indent=4))
        return {
            'success': True,
            'order_id': str(int(datetime.now().timestamp())),
        }
    return api_orders.limit_order_gtc_buy(*args, **kwargs)

def sell(*args, **kwargs):
    if TESTING:
        print("SELL")
        print(json.dumps(kwargs, indent=4))
        return {
            'success': True,
            'order_id': str(int(datetime.now().timestamp())),
        }
    return api_orders.limit_order_gtc_sell(*args, **kwargs)

def get_order(*args, **kwargs):
    if TESTING:
        print("CHECK: ", end='')
        print(kwargs)
        return {
            'success': True,
            'order': {
                'status': 'OPEN',
            },
        }
    return api_orders.get_order(*args, **kwargs)

def cancel(*args, **kwargs):
    if TESTING:
        print("CANCEL: ", end='')
        print(kwargs)
        return {
            'results': [
                {'success': True},
            ]
        }
    return api_orders.cancel_orders(*args, **kwargs)

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
    status: 'Order.Status'
    btc: float
    usd: float
    order_id: str
    order_time: datetime

    def __init__(self, order_type: Type, btc: float, usd: float, client_order_id: str = None, order_id: str = None, order_time: datetime=None, status: 'Order.Status' = None):
        self.order_type = order_type
        self.status = status if status else Order.Status.Pending
        self.btc = btc
        self.usd = usd
        self.order_id = order_id
        self.order_time = order_time if order_time else datetime.now()
        self.client_order_id = client_order_id if client_order_id else self.order_time.strftime("%Y-%m-%d-%H-%M-%S-") + str(random.randint(0, 1000)).zfill(4)

    def churn(self, ctx: Context) -> bool:
        # XXX: Cancel test
        if TESTING and self.status == Order.Status.Canceled:
            return True

        # No order ID: Create new order
        if not self.order_id:
            self.status = Order.Status.Pending
            return self.place_order(ctx)

        # Check order status
        try:
            # Get order
            data = get_order(ctx, order_id=self.order_id)

            ok: bool = 'order' in data
            if not ok:
                Log.error("Failed to get order status: {}".format(data['error_response']['error']))
                return False
            # Set status
            elif data['order']['status'] == 'OPEN':
                self.status = Order.Status.Active
            elif data['order']['status'] == 'FILLED':
                if self.status != Order.Status.Complete:
                    Log.info("{} filled: {}".format("Buy" if self.order_type == Order.Type.Buy else "Sell", self.get_info()))
                self.status = Order.Status.Complete
            else:
                self.status = Order.Status.Canceled
                Log.info("{} canceled: {}".format("Buy" if self.order_type == Order.Type.Buy else "Sell", self.get_info()))
        except Exception as e:
            Log.error("Failed to check status of Order {}: {}".format(self.order_id, str(e)))
            return False

        return True

    def place_order(self, ctx: Context) -> bool:
        # How many BTC to buy/sell
        base_size: str = str(floor_btc(self.btc))
        # What the target price is (per whole bitcoin)
        limit_price: str = str(floor_usd(self.usd / self.btc))

        if self.order_type == Order.Type.Buy:
            print(limit_price)
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
                    self.status = Order.Status.Active
                    self.order_id = order_info['order_id']
                    Log.info("Created buy order {}: {}".format(self.order_id, self.get_info()))
                else:
                    Log.error("Failed to create buy order: {}: {}".format(self.get_info(), order_info['error_response']['error']))
                return ok
            except Exception as e:
                Log.error("Failed to create buy order: {}: {}".format(self.get_info(), str(e)))
                return False
        elif self.order_type == Order.Type.Sell:
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
                    self.status = Order.Status.Active
                    self.order_id = order_info['order_id']
                    Log.info("Created sell order {}: {}".format(self.order_id, self.get_info()))
                else:
                    Log.error("Failed to create sell order: {}: {}".format(self.get_info(), order_info['error_response']['error']))
                return ok
            except Exception as e:
                Log.error("Failed to create sell order: {}: {}".format(self.get_info(), str(e)))
                return False
        else:
            self.status = Order.Status.Canceled
            Log.error("Invalid order type.")

        return False

    def get_info(self) -> str:
        return "{:.5f} BTC for ${:.2f} (${:.2f} market price).".format(self.btc, self.usd, self.usd / self.btc)

    def cancel(self, ctx: Context) -> bool:
        if self.order_id and not self.status in [Order.Status.Canceled, Order.Status.Complete]:
            try:
                finish = cancel(
                    ctx,
                    order_ids=[self.order_id],
                )
                ok = finish['results'][0]['success']
                if ok:
                    self.status = Order.Status.Canceled
                    Log.info("Canceled {} {} {}".format(
                        "buy" if self.order_type == Order.Type.Buy else "sell",
                        self.order_id,
                        self.get_info()))
                else:
                    Log.error("Failed to cancel {} {} {}".format(
                        "buy" if self.order_type == Order.Type.Buy else "sell",
                        self.order_id,
                        self.get_info()))
                return ok
            except Exception as e:
                Log.error("Failed to cancel {} {} {}: {}".format(
                    "buy" if self.order_type == Order.Type.Buy else "sell",
                    self.order_id,
                    self.get_info(),
                    str(e)))
                return False
        return True

    def to_dict(self) -> dict:
        return {
            'type': self.order_type.name,
            'status': self.status.name,
            'btc': str(self.btc),
            'usd': str(self.usd),
            'market': str(floor_usd(self.usd / self.btc)),
            'order_id': self.order_id,
            'order_time': self.order_time.strftime("%Y-%m-%d %H:%M:%S"),
        }

    @classmethod
    def from_dict(cls, data: dict) -> 'Order':
        try:
            order_type = Order.Type[data['type']]
            status = Order.Status[data['status']]
            btc = float(data['btc'])
            usd = float(data['usd'])
            order_id = data['order_id']
            order_time = datetime.strptime(data['order_time'], "%Y-%m-%d %H:%M:%S")
            order = cls(
                order_type=order_type,
                btc=btc,
                usd=usd,
                order_id=order_id,
                order_time=order_time,
                status=status,
            )
            return order
        except Exception as e:
            Log.error("Failed to deserialize Order: {}".format(str(e)))
        return None