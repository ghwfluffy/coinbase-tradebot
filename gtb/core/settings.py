from typing import Dict, List

class Spread():
    name: str
    usd: float
    spread: float

    def __init__(self, name: str, usd: float, spread: float) -> None:
        self.name = name
        self.usd = usd
        self.spread = spread

class Settings():
    allocations: Dict[str, float] = {
        "HODL": 1.0,
        "Spread": 0.9,
        "Phased": 0.005,
    }


    # HODL Algorithm #
    ##################

    # How often to create new hold position
    HODL_FREQUENCY_MINUTES: float = 240

    # How much to buy in our hold position
    HODL_BTC: float = 0.00002


    # Spread Algorithm #
    ####################

    spreads: List[Spread] = [
        Spread(
            name="Min",
            usd=80,
            spread=0.0032,
        ),
        Spread(
            name="Low",
            usd=300,
            spread=0.0036,
        ),
        Spread(
            name="Mid",
            usd=450,
            spread=0.0046,
        ),
        Spread(
            name="High",
            usd=500,
            spread=0.0052,
        ),
        Spread(
            name="Max",
            usd=500,
            spread=0.01,
        ),
    ]
