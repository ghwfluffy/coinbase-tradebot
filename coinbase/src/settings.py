from algorithm.tranche import Tranche

from dateutil.relativedelta import relativedelta

class Settings():
    TRANCHES = [
        Tranche(
            "Low",
            usd=500,
            spread=0.003,
            qty=2,
        ),
        Tranche(
            "Mid",
            usd=500,
            spread=0.0045,
            qty=2,
        ),
        Tranche(
            "High",
            usd=200,
            spread=0.006,
            qty=2,
        ),
    ]

    # Don't use more than 50% of wallet funds on tranched bets
    TRANCHE_WALLET_POOL: float = 1.0 #0.5

    # Don't use more than 50% of wallet funds on phased bets
    PHASED_WALLET_POOL: float = 0.5

    # Fail safe wallet amount
    MIN_WALLET: float = 1000.0

    # Process every 1 second
    RETRY_SLEEP_SECONDS: float = 0.1

    # Print market to console very minute
    PRINT_FREQUENCY: relativedelta = relativedelta(minutes=5)
