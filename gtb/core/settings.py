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
        "Spread": 0.80,
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
        #Spread(
        #    name="SmallSpeed",
        #    usd=222,
        #    spread=0.001,
        #),
        #Spread(
        #    name="Speed",
        #    usd=1000,
        #    spread=0.001,
        #),
        Spread(
            name="Min",
            usd=20,
            #usd=200,
            spread=0.0014,
        ),
        Spread(
            name="LowSmall",
            usd=50,
            spread=0.0016,
        ),
        Spread(
            name="Low",
            usd=500,
            spread=0.0016,
        ),
        Spread(
            name="MidSmall",
            usd=50,
            spread=0.0022,
        ),
        Spread(
            name="Mid",
            usd=500,
            spread=0.0022,
        ),
        Spread(
            name="High",
            usd=500,
            spread=0.003,
        ),
        Spread(
            name="HighSmall",
            usd=50,
            spread=0.003,
        ),
    ]
