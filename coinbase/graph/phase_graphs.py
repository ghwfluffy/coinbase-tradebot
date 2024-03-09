#!/usr/bin/env python3

import math
import json
import numpy
from datetime import datetime

import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib

from scipy.integrate import trapezoid
from scipy.interpolate import CubicSpline

def parse_date(x):
    return datetime.strptime(x, "%Y-%m-%d %H:%M:%S.%f")

class Entry:
    time: datetime
    phase: str
    status: str
    market: float
    smooth: float
    phase_seconds: float
    phase_score: float
    tail_score: float
    tail_percent: float
    usd_percent: float
    market_movement: float
    my_score: float
    my_tail_score: float

def create_plot(
    START_AT="2024-03-04 00:00:00",
    backend='Agg'
    ):

    matplotlib.use(backend)

    # Optionally, plot the curve and its derivative for visualization
    plt.figure(figsize=(12, 6))

    # Get data
    lines = []
    with open("phase_data_001.csv", "r") as fp:
    #with open("phase_data.csv", "r") as fp:
        lines = fp.readlines()

    # Parse data
    top = None
    bottom = None
    my_score = 0
    tail_scores = []
    entries = []

    datetimes = []
    my_tail_scores = []
    for line in lines:
        fields = line.split(",")
        if len(fields) <= 0:
            continue
        entry = Entry()
        entry.time = parse_date(fields[0])
        entry.phase = fields[1]
        entry.status = fields[2]
        entry.market = float(fields[3])
        entry.smooth = float(fields[4])
        entry.phase_seconds = float(fields[5])
        entry.phase_score = float(fields[6])
        entry.tail_score = float(fields[7])
        entry.tail_percent = float(fields[8])
        entry.usd_percent = float(fields[9])
        entry.market_movement = float(fields[10])
        my_score += entry.phase_score
        entries.append(entry)
        entry.my_score = my_score / len(entries)

        tail_scores.append(entry.phase_score)
        if len(tail_scores) > 10:
            tail_scores.pop(0)
        entry.my_tail_score = numpy.mean(tail_scores)

        if not bottom or entry.market < bottom:
            bottom = entry.market
        if not top or entry.market > top:
            top = entry.market

        datetimes.append(entry.time)
        my_tail_scores.append(entry.my_tail_score)

    NUM_GRAPHS = 3
    GRAPH = 0

    # Plot market
    GRAPH += 1
    plt.subplot(NUM_GRAPHS, 1, GRAPH)
    plt.title("Market")
    current_phase = None
    for entry in entries:
        plt.scatter(entry.time, entry.market, color='green', s=1)
        plt.scatter(entry.time, entry.smooth, color='blue', s=1)
        # Show phase changes
        if current_phase != entry.phase:
            current_phase = entry.phase
            color = 'black'
            if current_phase == "Waxing":
                color = 'orange'
            elif current_phase == "Waning":
                color = 'red'
            plt.plot([entry.time, entry.time], [entry.smooth + 20, entry.smooth - 20], color=color)

    # Setup y axis
    plt.ylim(bottom=bottom - 10, top=top + 10)
    # Setup labels
    plt.xlabel('Time')
    plt.ylabel('Market')
    plt.legend()



    # Plot score
    GRAPH += 1
    plt.subplot(NUM_GRAPHS, 1, GRAPH)
    for entry in entries:
        plt.scatter(entry.time, entry.my_score, color='blue', s=1)

    # Setup labels
    plt.xlabel('Time')
    plt.ylabel('Score')
    plt.legend()



    # Convert datetime to a numerical format (e.g., timestamp or ordinal)
    times_numeric = numpy.array([dt.timestamp() for dt in datetimes])

    # Sort the data based on time, just in case it's not sorted
    sorted_indices = numpy.argsort(times_numeric)
    times_numeric_sorted = times_numeric[sorted_indices]
    values_sorted = numpy.array(my_tail_scores)[sorted_indices]

    # Interpolate using a cubic spline
    cs = CubicSpline(times_numeric_sorted, values_sorted)

    # Generate a dense set of points for analysis
    dense_times = numpy.linspace(times_numeric_sorted.min(), times_numeric_sorted.max(), 1000)
    dense_values = cs(dense_times)

    # Calculate the first derivative (slope)
    derivative = cs.derivative()(dense_times)

    # Calculate the area under the curve for the first derivative for the tail section
    start_index = 0 #int(len(dense_times) * (1.0 - tail_percent))
    #area_under_curve = trapezoid(derivative[start_index:], dense_times[start_index:])






    # Plot tail score
    GRAPH += 1
    plt.subplot(NUM_GRAPHS, 1, GRAPH)
    #for entry in entries:
    #    plt.scatter(entry.time, entry.my_tail_score, color='blue', s=1)
    plt.plot(dense_times, derivative, label="Derivative")

    # Setup labels
    plt.xlabel('Time')
    plt.ylabel('Score')
    plt.legend()



    return plt

if __name__ == '__main__':
    plt = create_plot(backend='Qt5Agg')
    plt.show()
