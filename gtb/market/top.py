from datetime import datetime

class MarketTop():
    price: float
    last_update: datetime

    def __init__(self) -> None:
        self.price = 0.0
        self.last_update = datetime.now()
