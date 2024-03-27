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
    # Last 90 minutes
    extended: Phase
    # Last 8 hours
    trend: Phase

    def __init__(self) -> None:
        self.acute = Phase.Unknown
        self.short = Phase.Unknown
        self.mid = Phase.Unknown
        self.long = Phase.Unknown
        self.extended = Phase.Unknown
        self.trend = Phase.Unknown
