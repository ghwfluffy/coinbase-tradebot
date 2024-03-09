import os
import json
from datetime import datetime

from dateutil.relativedelta import relativedelta

from algorithm.phase import Phase
from orders.order_pair import OrderPair
from utils.logging import Log
from market.smooth import SmoothMarket

from settings import Settings

class PhaseTracker():
    phases: list[Phase]
    current_phase: Phase | None
    current_order: OrderPair | None
    current_max: float | None
    smooth: SmoothMarket

    def __init__(self):
        self.phases = []
        self.current_phase = None
        self.current_order = None
        self.current_max = None
        self.smooth = SmoothMarket(0.001)

    def current(self) -> Phase:
        if not self.current_phase:
            self.current_phase = Phase()
            self.phases.append(self.current_phase)
        return self.current_phase

    def new_phase(self, phase_type: Phase.Type = Phase.Type.Unknown) -> Phase:
        prev_phase = self.current_phase
        self.current_phase = Phase()
        self.current_phase.phase_type = phase_type
        self.phases.append(self.current_phase)
        if prev_phase:
            for i in range(0, len(prev_phase.points)):
                if prev_phase.points[i][0] > (datetime.now() - relativedelta(seconds=Settings.INITIAL_FRAME_SECONDS)):
                    self.current_phase.add_point(prev_phase.points[i][1], prev_phase.points[i][0])
        return self.current_phase

    @staticmethod
    def read_fs(file: str = "phases.json") -> 'PhaseTracker':
        tracker = PhaseTracker()
        # No saved state
        if not os.path.exists(file):
            return tracker

        # Read in file
        file_data: str
        with open(file, "r") as fp:
            file_data = fp.read()

        # Deserialize JSON string
        data: dict = json.loads(file_data)

        # Check version
        if not 'version' in data or data['version'] != 1:
            Log.error("PhaseTracker JSON file version mismatch.")
        else:
            # Deserialize each order pair
            for phase_data in data['phases']:
                phase = Phase.from_dict(phase_data)
                if phase:
                    tracker.phases.append(phase)

        return tracker

    def write_fs(self, file: str = "phases.json") -> None:
        # Serialize
        phases = []
        for phase in self.phases:
            phases.append(phase.to_dict())
        data = {
            'version': 1,
            'phases': phases,
        }

        # Write to file
        try:
            with open(file, "w") as fp:
                fp.write(json.dumps(data))
        except Exception as e:
            Log.error("Failed to write phases state: {}.".format(str(e)))
