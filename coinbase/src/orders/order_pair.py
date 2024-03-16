import json

from enum import Enum
from datetime import datetime
from typing import Optional

from context import Context
from orders.order import Order
from market.current import CurrentMarket
from utils.logging import Log

class OrderPair():
    class Status(Enum):
        # Waiting for a trigger
        OnHold = 0
        # Buy not filed
        Pending = 1
        # Buy order open
        Active = 2
        # Bought bitcoin but sale pending
        PendingSell = 3
        # Sold and bought bitcoin
        Complete = 4
        # One of the sides was canceled
        Canceled = 5

    buy: Order
    sell: Order | None
    status: 'OrderPair.Status'
    # When the order pair was created
    event_time: datetime
    # Price of BTC at time of order pair creation
    event_price: float
    # Tranche we are fulfilling
    tranche: str

    def __init__(self,
            tranche: str,
            event_price: float,
            buy: Order,
            sell: Order | None = None,
            event_time: datetime | None = None):
        self.buy = buy
        self.sell = sell
        self.status = OrderPair.Status.Pending
        self.event_time = event_time if event_time else datetime.now()
        self.event_price = event_price
        self.tranche = tranche

    def churn(self, ctx: Context, market: CurrentMarket) -> bool:
        # Marked complete for some reason
        if self.status in [OrderPair.Status.Canceled, OrderPair.Status.Complete]:
            return True

        # Place buy
        ret = self.buy.churn(ctx, market.split)

        # Successful buy, place/check sale
        if self.sell and self.buy.status == Order.Status.Complete:
            ret &= self.sell.churn(ctx, market.split)

        # Update pair status
        if ret:
            if self.buy.status == Order.Status.Canceled or (self.sell and self.sell.status == Order.Status.Canceled):
                self.status = OrderPair.Status.Canceled
            elif self.buy.status == Order.Status.Complete and self.sell and self.sell.status == Order.Status.Complete:
                self.status = OrderPair.Status.Complete
            elif self.buy.status == Order.Status.OnHold:
                self.status = OrderPair.Status.OnHold
            elif self.buy.status == Order.Status.Pending:
                self.status = OrderPair.Status.Pending
            elif self.buy.status == Order.Status.Active:
                self.status = OrderPair.Status.Active
            else:
                self.status = OrderPair.Status.PendingSell

        return ret

    def cancel(self, ctx: Context, reason: str) -> bool:
        ret = self.buy.cancel(ctx, reason)
        if self.sell:
            ret &= self.sell.cancel(ctx, reason)
        return ret

    def to_dict(self) -> dict:
        return {
            'buy': self.buy.to_dict(),
            'sell': self.sell.to_dict() if self.sell else None,
            'status': self.status.name,
            'event_time': self.event_time.strftime("%Y-%m-%d %H:%M:%S"),
            'event_price': str(self.event_price),
            'tranche': self.tranche,
        }

    @classmethod
    def from_dict(cls, data: dict) -> Optional['OrderPair']:
        try:
            buy = Order.from_dict(data['buy'])
            if not buy:
                Log.error("Failed to deserialize buy Order.")
                return None
            sell = None
            if data['sell']:
                sell = Order.from_dict(data['sell'])
                if not sell:
                    Log.error("Failed to deserialize sell Order.")
                    return None
            event_time = datetime.strptime(data['event_time'], "%Y-%m-%d %H:%M:%S")
            event_price = float(data['event_price'])

            tranche: str = str(data.get('tranche', ''))
            pair = cls(tranche, event_price, buy, sell, event_time)
            pair.status = OrderPair.Status[data['status']]
            return pair
        except Exception as e:
            Log.error("Failed to deserialize OrderPair: {}".format(str(e)))
        return None
