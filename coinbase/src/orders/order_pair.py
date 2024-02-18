import json

from datetime import datetime

from enum import Enum

from context import Context
from orders.order import Order

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
    status: Status
    # When the order pair was created
    event_time: datetime
    # Price of BTC at time of order pair creation
    event_price: float
    # Matches a TargetState
    target_state_id: str

    def __init__(self, buy: Order, sell: Order):
        self.buy = buy
        self.sell = sell
        self.status = Status.Pending
        self.event_time = datetime.now()

    def churn(self, ctx: Context) -> bool:
        # Place buy
        ret = self.buy.churn(ctx)

        # Successful buy, place/check sale
        if self.buy.status == Order.Status.Active:
            ret &= self.sell.churn(ctx)

        # Update pair status
        if ret:
            if self.buy.status == Order.Status.Canceled or self.sell.status == Order.Status.Canceled:
                self.status = Status.Canceled
            elif self.buy.status == Order.Status.Complete and self.sell.status == Order.Status.Complete:
                self.status = Status.Complete
            elif self.buy.status == Order.Status.Pending:
                self.status = Status.Pending
            elif self.buy.status == Order.Status.Active:
                self.status = Status.Active
            else:
                self.status = Status.PendingSell

        return ret

    def to_dict(self) -> dict
        return {
            'buy': self.buy.to_dict(),
            'sell': self.sell.to_dict(),
            'status': self.status.name,
            'event_time': self.event_time.strftime("%Y-%m-%d %H:%M:%S"),
            'event_price': str(self.event_price),
            'target_state_id': self.target_state_id,
        }

    @staticmethod
    def from_dict(data: str) -> OrderPair
        try:
            data: dict = json.loads(data)

            pair = OrderPair()
            pair.buy = Order.from_dict(data['buy'])
            if not pair.buy:
                Log.error("Failed to deserialize buy Order.")
                return None
            pair.sell = Order.from_dict(data['sell'])
            if not pair.sell:
                Log.error("Failed to deserialize sell Order.")
                return None
            pair.status = Status[data['status']]
            pair.event_time = datetime.strptime("%Y-%m-%d %H:%M:%S", data['event_time'])
            pair.event_price = float(data['event_price'])
            pair.target_state_id = data['target_state_id']
            return pair
        except Exception as e:
            Log.error("Failed to deserialize OrderPair: {}".format(str(e)))
        return None
