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
            name="Low",
            usd=20,
            #usd=500,
            spread=0.003,
        ),
        Spread(
            name="Mid",
            usd=20,
            #usd=500,
            spread=0.0045,
        ),
        Spread(
            name="High",
            usd=20,
            #usd=500,
            spread=0.006,
        ),
    ]
