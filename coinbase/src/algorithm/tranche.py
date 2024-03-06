from datetime import datetime

class Tranche:
    name: str
    usd: float
    spread: float
    qty: int
    last_discount: datetime
    discount_price: float

    def __init__(self,
        name: str,
        usd: float,
        spread: float,
        qty: int):

        self.name = name
        self.usd = usd
        self.spread = spread
        self.qty = qty
        self.last_discount = None
        self.discount_price = None

    def to_dict(self) -> None:
        return {
            'name': self.name,
            'usd': self.usd,
            'spread': self.spread,
            'qty': self.qty,
            #'last_discount': self.last_discount,
        }

    def from_dict(self, data: dict) -> None:
        self.name = data.get('name')
        self.usd = data.get('usd')
        self.spread = data.get('spread')
        self.qty = data.get('qty')
        #self.last_discount = data.get('last_discount')
