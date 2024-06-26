from datetime import datetime

class Tranche:
    name: str
    usd: float
    spread: float
    qty: int
    last_discount: datetime | None
    discount_price: float | None

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

    def to_dict(self) -> dict:
        return {
            'name': self.name,
            'usd': self.usd,
            'spread': self.spread,
            'qty': self.qty,
            #'last_discount': self.last_discount,
        }

    def from_dict(self, data: dict) -> None:
        self.name = str(data.get('name', ''))
        self.usd = float(data.get('usd', 0))
        self.spread = float(data.get('spread', 0.0))
        self.qty = int(data.get('qty', 0))
        #self.last_discount = data.get('last_discount')
