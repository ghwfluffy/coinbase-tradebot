from datetime import datetime
from enum import Enum

from market.smooth import SmoothMarket

class Phase():
    class Type(Enum):
        Unknown = 0
        Waxing = 1
        Waning = 2
        Plateau = 3

    phase: Type
    when: datetime
    price: float
    points: []

    def __init__(self):
        self.phase = Type.Unknown
        self.when = datetime.now()
        self.price = None
        self.points = []

    def add_point(self, bid: float) -> None:
        self.points.append((datetime.now(), bid))

    def to_dict(self):
        return {
            'phase': self.phase.Name,
            'when': self.when.strftime("%Y-%m-%d %H:%M:%S"),
            'price': self.price,
            'points': self.points,
        }

    def from_dict(self, data: dict):
        self.phase = Phase.Type[data['phase']]
        self.when = datetime.strptime(data['when'], "%Y-%m-%d %H:%M:%S")
        self.price = float(data['price'])
        self.points = data['points']
