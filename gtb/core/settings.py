from typing import Dict

class Settings():
    allocations: Dict[str, float] = {
        "HODL": 1.0,
        "Tranche": 0.9,
        "Phased": 0.005,
    }

    # HODL Algorithm #
    ##################

    # How often to create new hold position
    HODL_FREQUENCY_MINUTES: float = 240

    # How much to buy in our hold position
    HODL_BTC: float = 0.00002
