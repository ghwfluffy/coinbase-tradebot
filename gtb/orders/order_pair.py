from enum import Enum
from datetime import datetime

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
    status: OrderPair.Status
    algorithm: str
    event_time: datetime

    def __init__(self, algorithm: str, buy: Order, sell: Order | None = None) -> None:
        self.buy = buy
        self.sell = sell
        self.status = OrderPair.Status.Pending
        self.algorithm = algorithm
        self.event_time = datetime.now()

    def to_dict(self) -> dict:
        return {
            'status': self.status.name,
            'algorithm': self.algorithm,
            'event_time': self.event_time.strftime("%Y-%m-%d %H:%M:%S"),
            'buy': self.buy.to_dict(),
            'sell': self.sell.to_dict() if self.sell else None,
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
            return ret
        except Exception as e:
            Log.exception("Failed to deserialize OrderPair", e)
            return None
