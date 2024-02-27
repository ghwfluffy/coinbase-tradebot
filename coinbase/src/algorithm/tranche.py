class Tranche:
    name: str
    usd: float
    spread: float
    qty: int

    def __init__(self,
        name: str,
        usd: float,
        spread: float,
        qty: int):

        self.name = name
        self.usd = usd
        self.spread = spread
        self.qty = qty

    def to_dict(self) -> None:
        return {
            'name': self.name,
            'usd': self.usd,
            'spread': self.spread,
            'qty': self.qty,
        }

    def from_dict(self, data: dict) -> None:
        self.name = data.get('name')
        self.usd = data.get('usd')
        self.spread = data.get('spread')
        self.qty = data.get('qty')
