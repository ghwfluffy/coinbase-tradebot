from gtb.phases.phase import Phase

class PhaseCalculations():
    # Last minute
    acute: Phase
    # Last 5 minutes
    short: Phase
    # Last 10 mnutes
    mid: Phase
    # Last 30 minutes
    long: Phase
    # Last 3 hours
    extended: Phase

    def __init__(self) -> None:
        self.acute = Phase.Unknown
        self.short = Phase.Unknown
        self.mid = Phase.Unknown
        self.long = Phase.Unknown
        self.extended = Phase.Unknown
