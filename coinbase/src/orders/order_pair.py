import json

from datetime import datetime

from enum import Enum

from context import Context
from orders.order import Order
from orders.target import TargetState
from market.current import CurrentMarket
from utils.logging import Log

class OrderPair():
    class Status(Enum):
        # Buy not filed
        Pending = 0
        # Buy and sell orders pending
        Active = 1
        # Bought bitcoin but sale pending
        PendingSell = 3
        # Sold and bought bitcoin
        Complete = 4
        # One of the sides was canceled
        Canceled = 5

    buy: Order
    sell: Order
    status: 'OrderPair.Status'
    # When the order pair was created
    event_time: datetime
    # Price of BTC at time of order pair creation
    event_price: float
    # TargetState we are fulfilling
    target_id: str
    target: TargetState

    def __init__(self, target: TargetState, event_price: float, buy: Order, sell: Order, event_time = None):
        self.buy = buy
        self.sell = sell
        self.status = OrderPair.Status.Pending
        self.event_time = event_time if event_time else datetime.now()
        self.event_price = event_price
        self.target = target
        self.target_id = target._id

    def churn(self, ctx: Context, market: CurrentMarket) -> bool:
        # Place buy
        ret = self.buy.churn(ctx, market.split)

        # Successful buy, place/check sale
        if self.buy.status == Order.Status.Complete:
            ret &= self.sell.churn(ctx, market.split)

        # Update pair status
        if ret:
            if self.buy.status == Order.Status.Canceled or self.sell.status == Order.Status.Canceled:
                self.status = OrderPair.Status.Canceled
            elif self.buy.status == Order.Status.Complete and self.sell.status == Order.Status.Complete:
                self.status = OrderPair.Status.Complete
            elif self.buy.status == Order.Status.Pending:
                self.status = OrderPair.Status.Pending
            elif self.buy.status == Order.Status.Active:
                self.status = OrderPair.Status.Active
            else:
                self.status = OrderPair.Status.PendingSell

        return ret

    def cancel(self, ctx: Context) -> None:
        ret = self.buy.cancel(ctx)
        ret &= self.sell.cancel(ctx)
        return ret

    def to_dict(self) -> dict:
        return {
            'buy': self.buy.to_dict(),
            'sell': self.sell.to_dict(),
            'status': self.status.name,
            'event_time': self.event_time.strftime("%Y-%m-%d %H:%M:%S"),
            'event_price': str(self.event_price),
            'target_id': self.target._id,
            'target_spread': self.target.spread,
            'target_wager': self.target.wager,
            'target_longevity': self.target.longevity,
            'target_qty': self.target.qty,
        }

    @classmethod
    def from_dict(cls, data: dict) -> 'OrderPair':
        try:
            buy = Order.from_dict(data['buy'])
            if not buy:
                Log.error("Failed to deserialize buy Order.")
                return None
            sell = Order.from_dict(data['sell'])
            if not sell:
                Log.error("Failed to deserialize sell Order.")
                return None
            event_time = datetime.strptime(data['event_time'], "%Y-%m-%d %H:%M:%S")
            event_price = float(data['event_price'])

            target_id = None
            target_spread = None
            target_wager = None
            target_longevity = None
            target_qty = None
            try:
                target_id = data['target_id']
                target_spread = data['target_spread']
                target_wager = data['target_wager']
                target_longevity = data['target_longevity']
                target_qty = data['target_qty']
            except:
                pass
            target: TargetState = TargetState(target_qty, target_spread, target_wager, target_longevity, target_id)
            pair = cls(target, event_price, buy, sell, event_time)
            pair.status = OrderPair.Status[data['status']]
            return pair
        except Exception as e:
            Log.error("Failed to deserialize OrderPair: {}".format(str(e)))
        return None
