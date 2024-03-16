from algorithm.tranche import Tranche

from dateutil.relativedelta import relativedelta

class Settings():
    #                 #
    # Global Settings #

    # Fail safe wallet amount
    MIN_WALLET: float = 1000.0

    # Process every 1 second
    RETRY_SLEEP_SECONDS: float = 0.1

    # Print market to console very minute
    PRINT_FREQUENCY: relativedelta = relativedelta(minutes=5)


    #                    #
    # Wallet Allocations #

    # Don't use more than 50% of wallet funds on tranched bets
    TRANCHED_WALLET_POOL: float = 0.9

    # Don't use more than 50% of wallet funds on phased bets
    PHASED_WALLET_POOL: float = 0.4


    #          #
    # Tranched #
    TRANCHES = [
        Tranche(
            "Low",
            usd=500,
            spread=0.003,
            qty=3,
        ),
        Tranche(
            "Mid",
            usd=500,
            spread=0.0045,
            qty=3,
        ),
        Tranche(
            "High",
            usd=500,
            spread=0.006,
            qty=3,
        ),
    ]


    #        #
    # Phased #

    # How many seconds from previous phase is included in initial frame
    INITIAL_FRAME_SECONDS = 30
    WANING_MINIMUM_SECONDS = INITIAL_FRAME_SECONDS + 10

    # How much of the frame do we compare against the tail
    MAX_FRAME_SECONDS = 120

    # What percent of the phase is considered the tail of the phase
    TAIL_PERCENT = 0.2
    # If the tail causes slope to change by 20% of what it was, we're tapering up/down
    TAPER_PERCENT = 0.2

    # How big of an up slope in the tail determines if it's an up curve
    GOOD_TILT = 1.0
    # What sort of down curve determines a definite sell
    BAD_TILT = -1.0

    # String identifier for phased tranche
    PHASED_TRANCHE_NAME = "Phased"

    # How much USD above/below the bid/ask price to ensure we're a maker not a taker
    PHASED_MAKER_BUFFER = 2.0


    #      #
    # HODL #

    # How often to create new hold position
    HODL_FREQUENCY_MINUTES: float = 240

    # How much to buy in our hold position
    HODL_BTC: float = 0.00002
