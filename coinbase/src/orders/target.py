class TargetState():
    """
    Describes a spread between buys/sell pairs in percentage of bitcoin,
    how many of such pairs we want to maintain at any given time,
    how much each pair should be worth in USD, and
    how long of a window we should consider those pairs still good for.
    """

    # Number of order pairs
    qty: int
    # The percent difference in buy/sell of the pair (1=100%)
    spread: float
    # The amount of USD for each
    wager: float
    # Number of hours before trade can be considered invalidated
    longevity: int
    # If we buy immediately or wait till it goes down to buy
    autofill: bool

    # Hash of state
    _id: str

    def __init__(self, qty: int, spread: float, wager: float, longevity: int, name=None, autofill=False):
        self.qty = qty
        self.spread = spread
        self.wager = wager
        self.longevity = longevity
        self.autofill = autofill
        if name:
            self._id = name
        else:
            self.calc_id()

    def calc_id(self) -> None:
        self._id = "{}_{}_{}_{}".format(self.qty, self.spread, self.wager, self.longevity)
        return None
