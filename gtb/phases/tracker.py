from datetime import datetime
from dateutil.relativedelta import relativedelta

from typing import List, Tuple

from gtb.core.thread import BotThread
from gtb.core.context import Context
from gtb.market.prices import MarketPrices
from gtb.phases.calculations import PhaseCalculations
from gtb.phases.phase import Phase
from gtb.utils.logging import Log

# Keep track of the current market bid/ask
class PhaseTracker(BotThread):
    history: List[Tuple[datetime, float]]
    diffs: List[Phase]

    def __init__(self, ctx: Context) -> None:
        super().__init__(ctx)
        self.history = []

    def init(self) -> None:
        pass

    def think(self) -> None:
        # Get latest market
        market: MarketPrices = self.ctx.smooth_market
        # No update
        if len(self.history) > 0 and self.history[-1][0] == market.updated:
            return None

        # Save update
        self.history.append((market.updated, market.split))
        if len(self.history) == 1:
            return None

        extended_ago: datetime = datetime.now() - relativedelta(hours=3)
        long_ago: datetime = datetime.now() - relativedelta(minutes=30)
        mid_ago: datetime = datetime.now() - relativedelta(minutes=10)
        short_ago: datetime = datetime.now() - relativedelta(minutes=5)
        acute_ago: datetime = datetime.now() - relativedelta(minutes=1)

        extended_index: int = -1
        long_index: int = -1
        mid_index: int = -1
        short_index: int = -1
        acute_index: int = -1
        for index in range(0, len(self.history)):
            cur: Tuple[datetime, float] = self.history[index]
            if cur[0] < extended_ago:
                extended_index = index
            if cur[0] < long_ago:
                long_index = index
            if cur[0] < mid_ago:
                mid_index = index
            if cur[0] < short_ago:
                short_index = index
            if cur[0] < acute_ago:
                acute_index = index
            else:
                break

        calc: PhaseCalculations = self.ctx.phases
        if acute_index >= 0:
            calc.acute = self.calc_phase(acute_index, 20.0)

        if short_index >= 0:
            calc.short = self.calc_phase(short_index, 30.0)

        if mid_index >= 0:
            calc.mid = self.calc_phase(mid_index, 50.0)

        if long_index >= 0:
            calc.long = self.calc_phase(long_index, 100.0)

        if extended_index >= 0:
            calc.extended = self.calc_phase(extended_index, 800.0)

            # Trim history
            self.history = self.history[extended_index:]

        Log.trace("Phases: [ {} | {} | {} | {} | {} ]".format(
            calc.extended.name,
            calc.long.name,
            calc.mid.name,
            calc.short.name,
            calc.acute.name,
        ))

    def calc_phase(self, index: int, min_delta: float) -> Phase:
        before: float = self.history[index][1]
        after: float = self.history[-1][1]
        if abs(after - before) < min_delta:
            return Phase.Plateau
        elif after > before:
            return Phase.Waxing
        elif after < before:
            return Phase.Waning
        return Phase.Unknown
