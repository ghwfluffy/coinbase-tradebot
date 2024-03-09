from datetime import datetime
from enum import Enum
from typing import List, Tuple

import numpy as np

from scipy.integrate import trapezoid
from scipy.interpolate import CubicSpline

from market.smooth import SmoothMarket

from settings import Settings

class Phase():
    class Type(Enum):
        Unknown = 0
        Waxing = 1
        Waning = 2
        Plateau = 3

    phase_type: 'Phase.Type'
    when: datetime
    price: float
    points: List[Tuple[datetime, float]]
    scores: List[float]

    def __init__(self):
        self.phase_type = Phase.Type.Unknown
        self.when = datetime.now()
        self.price = None
        self.points = []
        self.scores = []

    def get_score_frame_percent(self) -> float:
        # What percent of the frame is the score based on
        score_percent: float = 1
        longevity = self.phase_seconds()
        if longevity > Settings.MAX_FRAME_SECONDS:
            score_percent = Settings.MAX_FRAME_SECONDS / longevity
        return score_percent

    def add_point(self, bid: float, when: datetime | None = None) -> None:
        if not when:
            when = datetime.now()
        self.points.append((when, bid))
        if self.price == None:
            self.price = bid

        score_percent = self.get_score_frame_percent()
        score = self.get_score(score_percent)
        self.scores.append(score)

    def to_dict(self) -> dict:
        return {
            'phase': self.phase_type.name,
            'when': self.when.strftime("%Y-%m-%d %H:%M:%S.%f"),
            'price': self.price,
            'points': self.points_to_list(),
            'scores': self.scores,
        }

    @staticmethod
    def from_dict(data: dict) -> 'Phase':
        ret = Phase()
        ret.phase_type = Phase.Type[data['phase']]
        ret.when = datetime.strptime(data['when'], "%Y-%m-%d %H:%M:%S.%f")
        ret.price = float(data['price']) if data['price'] else 0.0
        ret.points_from_list(data['points'])
        ret.scores = data['scores']
        return ret

    def points_to_list(self) -> list:
        ret = []
        for point in self.points:
            ret.append([point[0].strftime("%Y-%m-%d %H:%M:%S.%f"), point[1]])
        return ret

    def points_from_list(self, points) -> None:
        for point in points:
            self.points.append((datetime.strptime(point[0], "%Y-%m-%d %H:%M:%S.%f"), float(point[1])))

    def phase_seconds(self) -> float:
        if len(self.points) < 2:
            return 0.0
        return (self.points[-1][0] - self.points[0][0]).total_seconds()

    def get_score(self, tail_percent = 1.0) -> float:
        if len(self.points) < 2:
            return 0.0

        # Split x and y
        datetimes = []
        values = []
        for x in self.points:
            datetimes.append(x[0])
            values.append(x[1])

        # Convert datetime to a numerical format (e.g., timestamp or ordinal)
        times_numeric = np.array([dt.timestamp() for dt in datetimes])

        # Sort the data based on time, just in case it's not sorted
        sorted_indices = np.argsort(times_numeric)
        times_numeric_sorted = times_numeric[sorted_indices]
        values_sorted = np.array(values)[sorted_indices]

        # Interpolate using a cubic spline
        cs = CubicSpline(times_numeric_sorted, values_sorted)

        # Generate a dense set of points for analysis
        dense_times = np.linspace(times_numeric_sorted.min(), times_numeric_sorted.max(), 1000)
        dense_values = cs(dense_times)

        # Calculate the first derivative (slope)
        second_derivative = cs.derivative(2)(dense_times)

        # Calculate the area under the curve for the first derivative for the tail section
        start_index = int(len(dense_times) * (1.0 - tail_percent))
        area_under_curve = trapezoid(second_derivative[start_index:], dense_times[start_index:])

        return area_under_curve

    def get_tilt(self, tail_percent = 1.0) -> float:
        if len(self.points) < 2:
            return 0.0

        # Where does tail start
        start_index: int = int(len(self.points) * (1.0 - tail_percent))
        if len(self.points) - start_index < 2:
            return 0.0

        # Get x and y coordinates
        datetimes: List[datetime] = []
        values: List[float] = []

        total_score: float = 0.0
        for i in range(start_index, len(self.points)):
            datetimes.append(self.points[i][0])
            total_score += self.scores[i]
            values.append(total_score / (len(values) + 1))

        # Convert datetime to a numerical format (e.g., timestamp or ordinal)
        times_numeric = np.array([dt.timestamp() for dt in datetimes])

        # Sort the data based on time, just in case it's not sorted
        sorted_indices = np.argsort(times_numeric)
        times_numeric_sorted = times_numeric[sorted_indices]
        values_sorted = np.array(values)[sorted_indices]

        # Interpolate using a cubic spline
        cs = CubicSpline(times_numeric_sorted, values_sorted)

        # Generate a dense set of points for analysis
        dense_times = np.linspace(times_numeric_sorted.min(), times_numeric_sorted.max(), 1000)
        dense_values = cs(dense_times)

        # Calculate the first derivative (slope)
        derivative = cs.derivative()(dense_times)

        return derivative[-1]
