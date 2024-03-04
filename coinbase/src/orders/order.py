import time
import json
import random

from datetime import datetime
from enum import Enum

from context import Context
from market.current import CurrentMarket
from utils.logging import Log
from utils.math import floor_btc, floor_usd

from coinbase.rest import orders as api_orders

# XXX: This is broken / does not mix with real orderbook
TESTING=False
#TESTING=True

# XXX: Safety precautions
MAX_PRICE=78000
MIN_PRICE=48000

def buy(*args, **kwargs):
    if float(kwargs['limit_price']) > MAX_PRICE:
        Log.error("MAX PRICE EXCEEDED: {}".format(kwargs['limit_price']))
        return False

    if TESTING:
        print("BUY")
        print(json.dumps(kwargs, indent=4))
        return {
            'success': True,
            'order_id': str(int(datetime.now().timestamp())),
        }
    return api_orders.limit_order_gtc_buy(*args, **kwargs)
    #kwargs['stop_direction'] = 'STOP_DIRECTION_STOP_DOWN'
    #kwargs['stop_price'] = str(float(kwargs['limit_price']) - 1)
    #return api_orders.stop_limit_order_gtc_buy(*args, **kwargs)

def sell(*args, **kwargs):
    if float(kwargs['limit_price']) < MIN_PRICE:
        Log.error("MIN PRICE EXCEEDED: {}".format(kwargs['limit_price']))
        return False

    if TESTING:
        print("SELL")
        print(json.dumps(kwargs, indent=4))
        return {
            'success': True,
            'order_id': str(int(datetime.now().timestamp())),
        }
    return api_orders.limit_order_gtc_sell(*args, **kwargs)
    #kwargs['stop_direction'] = 'STOP_DIRECTION_STOP_UP'
    #kwargs['stop_price'] = str(float(kwargs['limit_price']) + 1)
    #return api_orders.stop_limit_order_gtc_sell(*args, **kwargs)

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
    final_time: datetime
    final_market: float
    final_fees: float
    final_usd: float
    cancel_time: datetime
    cancel_reason: str

    def __init__(self, order_type: Type, btc: float, usd: float, client_order_id: str = None, order_id: str = None, order_time: datetime=None, final_time: datetime=None, status: 'Order.Status' = None, final_market=None, final_fees=None, final_usd=None, cancel_time: datetime=None, cancel_reason: str = None):
        self.order_type = order_type
        self.status = status if status else Order.Status.Pending
        self.btc = btc
        self.usd = usd
        self.order_id = order_id
        self.order_time = order_time if order_time else datetime.now()
        self.client_order_id = client_order_id if client_order_id else datetime.now().strftime("%Y-%m-%d-%H-%M-%S-") + str(random.randint(0, 1000)).zfill(4)
        self.final_time = final_time
        self.final_market = final_market
        self.final_fees = final_fees
        self.final_usd = final_usd
        self.insufficient_funds = False
        self.cancel_time = cancel_time
        self.cancel_reason = cancel_reason

    def is_good_market_value(self, current_price: float) -> bool:
        # Within $20 of target - Cheap enough to list our buy
        if self.order_type == Order.Type.Buy:
            return (current_price - 20.0) <= self.get_limit_price()
        # Within $20 of target - Expensive enough to list our sale
        if self.order_type == Order.Type.Sell:
            return (current_price + 20.0) >= self.get_limit_price()
        return False

    def is_good_market_conditions(self, current_price: float) -> bool:
        # There is at least a dollar gap between sell and buy bids
        # Really want to make sure we don't take the 'takers' fee
        return current_price.ask - current_price.bid >= 1.0

    def churn(self, ctx: Context, current_price: float) -> bool:
        # Already in canceled state
        if self.status == Order.Status.Canceled:
            return True

        # No order ID: Create new order
        if not self.order_id:
            self.status = Order.Status.Pending
            # Check cached current price
            if not self.is_good_market_value(current_price):
                return True
            # Check again with live current price
            current_market: CurrentMarket = CurrentMarket.get(ctx)
            if not current_market or not current_market.split or not self.is_good_market_value(current_market.split) or not self.is_good_market_conditions(current_market):
                return True
            # We are ready to place an order
            return self.place_order(ctx, current_market.split)

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
                # Make sure we're not holding onto trades that no longer reflect the market
                # XXX: Let newmain algorithm worry about this
                #self.check_longevity(ctx, current_price)
            elif data['order']['status'] == 'FILLED':
                if self.status != Order.Status.Complete:
                    Log.info("{} filled: {} (${} fee)".format("Buy" if self.order_type == Order.Type.Buy else "Sell", self.get_info(), data['order']['total_fees']))
                self.status = Order.Status.Complete
                if not self.final_time:
                    self.final_time = datetime.now()
                self.final_market = float(data['order']['average_filled_price'])
                self.final_fees = float(data['order']['total_fees'])
                self.final_usd = float(data['order']['filled_value'])
            else:
                self.status = Order.Status.Canceled
                if not self.final_time:
                    self.final_time = datetime.now()
                Log.info("{} canceled: {}".format("Buy" if self.order_type == Order.Type.Buy else "Sell", self.get_info()))
        except Exception as e:
            Log.error("Failed to check status of Order {}: {}".format(self.order_id, str(e)))
            return False

        return True

    def check_longevity(self, ctx: Context, current_price: float) -> None:
        # Less than 20 minutes old, we'll always keep the trade open
        if (datetime.now() - self.order_time).total_seconds() < (60 * 20):
            return None
        # We're pretty close to the price we asked, we'll keep the trade open
        if abs(current_price - self.final_market) < 20.0:
            return None
        # Cancel trade
        Log.debug("Sale longevity reached.")
        if self.cancel(ctx, "longevity"):
            # Go back to pending
            self.status = Order.Status.Pending

    def get_limit_price(self) -> float:
        return floor_usd(self.usd / self.btc)

    def get_price(self) -> float:
        if self.final_market:
            return self.final_market
        return self.get_limit_price()

    def get_order_price(self, current_price: float) -> float:
        # We will bid $20 different than what the market wants
        if self.order_type == Order.Type.Sell:
            return floor_usd(current_price + 20)
        elif self.order_type == Order.Type.Buy:
            return floor_usd(current_price - 20)
        return current_price

    def place_order(self, ctx: Context, current_price: float) -> bool:
        # How many BTC to buy/sell
        base_size: str = str(floor_btc(self.btc))

        # What the target price is (per whole bitcoin)
        limit_price: float = self.get_order_price(current_price)

        # New buy order
        if self.order_type == Order.Type.Buy:
            try:
                order_info = buy(
                    ctx,
                    client_order_id=self.client_order_id + "-buy",
                    product_id='BTC-USD',
                    base_size=base_size,
                    limit_price=str(limit_price),
                )
                ok: bool = 'success' in order_info and order_info['success']
                if ok:
                    self.order_time = datetime.now()
                    self.status = Order.Status.Active
                    self.order_id = order_info['order_id']
                    self.final_market = limit_price
                    Log.info("Created buy order {}: {}".format(self.order_id, self.get_info()))
                else:
                    if order_info['error_response']['error'] == 'INSUFFICIENT_FUND':
                        if self.insufficient_funds:
                            return False
                        self.insufficient_funds = True
                    Log.error("Failed to create buy order: {}: {}".format(self.get_info(), order_info['error_response']['error']))
                return ok
            except Exception as e:
                Log.error("Failed to create buy order: {}: {}".format(self.get_info(), str(e)))
                return False
        # New sell order
        elif self.order_type == Order.Type.Sell:
            try:
                order_info = sell(
                    ctx,
                    client_order_id=self.client_order_id + "-sell",
                    product_id='BTC-USD',
                    base_size=base_size,
                    limit_price=str(limit_price),
                )
                ok: bool = 'success' in order_info and order_info['success']
                if ok:
                    self.order_time = datetime.now()
                    self.status = Order.Status.Active
                    self.order_id = order_info['order_id']
                    self.final_market = limit_price
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

    def cancel(self, ctx: Context, reason: str) -> bool:
        # Cancel before ever executed
        if not self.order_id or self.status == Order.Status.Pending:
            self.status = Order.Status.Canceled
            self.cancel_reason = reason
            self.cancel_time = datetime.now()
        # Cancel active order
        elif self.order_id and not self.status in [Order.Status.Canceled, Order.Status.Complete]:
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
                    self.final_time = datetime.now()
                    self.order_id = None
                    self.cancel_reason = reason
                    self.cancel_time = datetime.now()
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
            'order_time': self.order_time.strftime("%Y-%m-%d %H:%M:%S") if self.order_time else None,
            'final_time': self.final_time.strftime("%Y-%m-%d %H:%M:%S") if self.final_time else None,
            'final_market': self.final_market,
            'final_usd': self.final_usd,
            'final_fees': self.final_fees,
            'cancel_time': self.cancel_time.strftime("%Y-%m-%d %H:%M:%S") if self.cancel_time else None,
            'cancel_reason': self.cancel_reason,
        }

    @classmethod
    def from_dict(cls, data: dict) -> 'Order':
        try:
            order_type = Order.Type[data['type']]
            status = Order.Status[data['status']]
            btc = float(data['btc'])
            usd = float(data['usd'])
            order_id = data['order_id']

            order_time = None
            try:
                order_time = datetime.strptime(data['order_time'], "%Y-%m-%d %H:%M:%S")
            except:
                pass

            final_time = None
            try:
                final_time = datetime.strptime(data['final_time'], "%Y-%m-%d %H:%M:%S")
            except:
                pass

            final_market = None
            final_fees = None
            final_usd = None
            try:
                final_market = float(data['final_market'])
                final_fees = float(data['final_fees'])
                final_usd = float(data['final_usd'])
            except:
                pass

            cancel_time = None
            try:
                cancel_time = datetime.strptime(data['cancel_time'], "%Y-%m-%d %H:%M:%S")
            except:
                pass
            cancel_reason = data.get('cancel_reason')

            order = cls(
                order_type=order_type,
                btc=btc,
                usd=usd,
                order_id=order_id,
                order_time=order_time,
                final_time=final_time,
                status=status,
                final_market=final_market,
                final_fees=final_fees,
                final_usd=final_usd,
                cancel_time=cancel_time,
                cancel_reason=cancel_reason,
            )
            return order
        except Exception as e:
            Log.error("Failed to deserialize Order: {}".format(str(e)))
        return None
