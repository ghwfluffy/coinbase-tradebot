from enum import Enum
from datetime import datetime

from typing import Optional

from gtb.utils.logging import Log
from gtb.utils.maths import floor_usd

class OrderInfo():
    order_id: str
    client_order_id: str
    order_time: datetime
    order_market: float
    final_time: datetime | None
    final_market: float | None
    final_fees: float | None
    final_usd: float | None
    cancel_time: datetime | None
    cancel_reason: str | None

    def __init__(self, order_id: str, client_order_id: str, order_time: datetime, order_market: float) -> None:
        self.order_id = order_id
        self.client_order_id = client_order_id
        self.order_time = order_time
        self.order_market = order_market
        self.final_time = None
        self.final_market = None
        self.final_fees = None
        self.final_usd = None
        self.cancel_time = None
        self.cancel_reason = None

    def to_dict(self) -> dict:
        return {
            'order_id': self.order_id,
            'client_order_id': self.client_order_id,
            'order_time': self.order_time.strftime("%Y-%m-%d %H:%M:%S") if self.order_time else None,
            'final_time': self.final_time.strftime("%Y-%m-%d %H:%M:%S") if self.final_time else None,
            'cancel_time': self.cancel_time.strftime("%Y-%m-%d %H:%M:%S") if self.cancel_time else None,
            'order_market': self.order_market,
            'final_market': self.final_market,
            'final_fees': self.final_fees,
            'final_usd': self.final_usd,
            'cancel_reason': self.cancel_reason,
        }

    @classmethod
    def from_dict(cls, data: dict) -> Optional['OrderInfo']:
        try:
            order_id: str = data['order_id']
            client_order_id: str = data['client_order_id']
            order_time: datetime = datetime.strptime(data['order_time'], "%Y-%m-%d %H:%M:%S")
            order_market: float = float(data['order_market'])
            ret = cls(order_id, client_order_id, order_time, order_market)
            try:
                ret.final_time = datetime.strptime(data['final_time'], "%Y-%m-%d %H:%M:%S")
            except:
                ret.final_time = None
                pass
            try:
                ret.cancel_time = datetime.strptime(data['cancel_time'], "%Y-%m-%d %H:%M:%S")
            except:
                ret.cancel_time = None
                pass
            ret.final_market = float(data['final_market'])
            ret.final_fees = float(data['final_fees'])
            ret.final_usd = float(data['final_usd'])
            ret.cancel_reason = data['cancel_reason']
            return ret
        except Exception as e:
            Log.exception("Failed to deserialize OrderInfo", e)
            return None

class Order():
    class Type(Enum):
        Buy = 0
        Sell = 1

    class Status(Enum):
        OnHold = 0
        Pending = 1
        Active = 2
        Complete = 3
        Canceled = 4

    order_type: Order.Type
    status: Order.Status
    btc: float
    usd: float
    info: OrderInfo | None
    insufficient_funds: bool

    def __init__(self, order_type: Type, btc: float, usd: float) -> None:
        self.order_type = order_type
        self.status = Order.Status.Pending
        self.btc = btc
        self.usd = usd
        self.info = None
        self.insufficient_funds = False

    def to_dict(self) -> dict:
        ret: dict = {
            'type': self.order_type.name,
            'status': self.status.name,
            'btc': self.btc,
            'usd': self.usd,
            'info': None,
        }
        if self.info:
            ret['info'] = self.info.to_dict()
        return ret

    @classmethod
    def from_dict(cls, data: dict) -> Optional['Order']:
        try:
            order_type: Order.Type = Order.Type[data['type']]
            btc: float = float(data['btc'])
            usd: float = float(data['usd'])
            ret = cls(order_type, btc, usd)
            if data['info']:
                ret.info = OrderInfo.from_dict(data['info'])
            return ret
        except Exception as e:
            Log.exception("Failed to deserialize Order", e)
            return None

    def get_limit_price(self) -> float:
        return floor_usd(self.usd / self.btc)
