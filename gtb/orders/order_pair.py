from enum import Enum
from datetime import datetime
from threading import Lock
from typing import Optional

from gtb.orders.order import Order
from gtb.utils.logging import Log

class OrderPair():
    class Status(Enum):
        # Waiting for a trigger
        OnHold = 0
        # Buy not filed
        Pending = 1
        # Buy order open
        Active = 2
        # Bought bitcoin but sale on hold
        OnHoldSell = 3
        # Bought bitcoin but sale pending
        PendingSell = 4
        # Bought bitcoin and sale is open
        ActiveSell = 5
        # All transactions complete
        Complete = 6
        # At least one of the sides was canceled
        Canceled = 7

    buy: Order
    sell: Order | None
    status: 'OrderPair.Status'
    algorithm: str
    event_time: datetime
    event_price: float
    buy_only: bool
    mtx: Lock

    def __init__(self, algorithm: str, buy: Order, sell: Order | None = None) -> None:
        self.buy = buy
        self.sell = sell
        self.status = OrderPair.Status.Pending
        self.algorithm = algorithm
        self.event_time = datetime.now()
        self.event_price = buy.get_limit_price()
        self.buy_only = False
        self.mtx = Lock()

        self.update_status()

    # Setup pair status from buy/sell status
    def update_status(self) -> None:
        if self.buy.status == Order.Status.OnHold:
            self.status = OrderPair.Status.OnHold
        elif self.buy.status == Order.Status.Pending:
            self.status = OrderPair.Status.Pending
        elif self.buy.status == Order.Status.Active:
            self.status = OrderPair.Status.Active
        elif self.buy.status == Order.Status.Canceled:
            self.status = OrderPair.Status.Canceled
        elif not self.sell:
            if self.buy_only:
                self.status = OrderPair.Status.Complete
            else:
                self.status = OrderPair.Status.OnHoldSell
        elif self.sell.status == Order.Status.OnHold:
            self.status = OrderPair.Status.OnHoldSell
        elif self.sell.status == Order.Status.Pending:
            self.status = OrderPair.Status.PendingSell
        elif self.sell.status == Order.Status.Active:
            self.status = OrderPair.Status.ActiveSell
        elif self.sell.status == Order.Status.Canceled:
            self.status = OrderPair.Status.Canceled
        elif self.sell.status == Order.Status.Complete:
            self.status = OrderPair.Status.Complete

    def to_dict(self) -> dict:
        return {
            'status': self.status.name,
            'algorithm': self.algorithm,
            'event_time': self.event_time.strftime("%Y-%m-%d %H:%M:%S"),
            'event_price': "{:0.2f}".format(self.event_price),
            'buy': self.buy.to_dict(),
            'sell': self.sell.to_dict() if self.sell else None,
            'buy_only': self.buy_only,
        }

    @classmethod
    def from_dict(cls, data: dict) -> Optional['OrderPair']:
        try:
            status: OrderPair.Status = OrderPair.Status[data['status']]
            algorithm: str = data['algorithm']

            buy: Order | None = Order.from_dict(data['buy'])
            assert buy is not None

            sell: Order | None = None
            if data['sell']:
                sell = Order.from_dict(data['sell'])

            ret = cls(algorithm, buy, sell)
            ret.status = status
            ret.event_time = datetime.strptime(data['event_time'], "%Y-%m-%d %H:%M:%S")
            if 'event_price' in data:
                ret.event_price = float(data['event_price'])
            ret.buy_only = data['buy_only']
            return ret
        except Exception as e:
            Log.exception("Failed to deserialize OrderPair", e)
            return None
