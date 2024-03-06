from datetime import datetime
from enum import Enum

import numpy as np

from scipy.integrate import trapezoid
from scipy.interpolate import CubicSpline

from market.smooth import SmoothMarket

def get_score(points, ):

    plt.show()


    return shape

class Phase():
    class Type(Enum):
        Unknown = 0
        Waxing = 1
        Waning = 2
        Plateau = 3

    phase: 'Phase.Type'
    when: datetime
    price: float
    points: []
    max_score: float
    min_score: float
    current_score: float

    def __init__(self):
        self.phase = Phase.Type.Unknown
        self.when = datetime.now()
        self.price = None
        self.points = []
        self.max_score = 0.0
        self.min_score = 0.0
        self.current_score = 0.0

    def add_point(self, bid: float, when: datetime = None) -> None:
        if not when:
            when = datetime.now()
        self.points.append((when, bid))
        if self.price == None:
            self.price = bid
        score = self.get_score()
        if score > self.max_score:
            self.max_score = score
        if score < self.min_score:
            self.min_score = score
        self.current_score = score

    def to_dict(self) -> None:
        return {
            'phase': self.phase.name,
            'when': self.when.strftime("%Y-%m-%d %H:%M:%S.%f"),
            'price': self.price,
            'points': self.points_to_list(),
            'max': self.max_score,
            'min': self.min_score,
            'current': self.current_score,
        }

    @staticmethod
    def from_dict(data: dict) -> 'Phase':
        ret = Phase()
        ret.phase = Phase.Type[data['phase']]
        ret.when = datetime.strptime(data['when'], "%Y-%m-%d %H:%M:%S.%f")
        ret.price = float(data['price']) if data['price'] else None
        ret.points_from_list(data['points'])
        ret.max_score = float(data['max'])
        ret.min_score = float(data['min'])
        ret.current_score = float(data['current'])

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
        first_derivative = cs.derivative()(dense_times)

        # Calculate the area under the curve for the first derivative for the tail section
        start_index = int(len(dense_times) * (1.0 - tail_percent))
        area_under_curve = trapezoid(first_derivative[start_index:], dense_times[start_index:])

        return area_under_curve

    def get_score_tail_seconds(self, seconds: float) -> float:
        if len(self.points) < 2:
            return 0.0
        total_seconds = self.phase_seconds()
        if total_seconds < seconds:
            return self.get_score()
        return self.get_score(seconds / total_seconds)
