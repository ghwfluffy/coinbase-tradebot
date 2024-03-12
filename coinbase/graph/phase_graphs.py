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
    phase_tilt: float
    tail_score: float
    tail_tilt: float
    usd_percent: float
    market_movement: float

def create_plot(
    START_AT="2024-03-04 00:00:00",
    backend='Agg'
    ):

    matplotlib.use(backend)

    # Optionally, plot the curve and its derivative for visualization
    plt.figure(figsize=(12, 6))

    # Get data
    lines = []
    with open("phase_data-v3.csv", "r") as fp:
        lines = fp.readlines()

    # Parse data
    top = None
    bottom = None
    entries = []
    datetimes = []
    scores = []
    tilts = []
    score_sums = []
    cur_score = 0
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
        entry.phase_tilt = float(fields[7])
        entry.tail_score = float(fields[8])
        entry.tail_tilt = float(fields[9])
        entry.usd_percent = float(fields[10])
        entry.market_movement = float(fields[11])
        entries.append(entry)

        if not bottom or entry.market < bottom:
            bottom = entry.market
        if not top or entry.market > top:
            top = entry.market

        datetimes.append(entry.time)
        scores.append(entry.tail_score)
        tilts.append(entry.tail_tilt)

        cur_score += entry.phase_score
        score_sums.append(cur_score)

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
    if False:
        GRAPH += 1
        plt.subplot(NUM_GRAPHS, 1, GRAPH)
        #for entry in entries:
        #    plt.scatter(entry.time, entry.phase_score, color='blue', s=1)
            #plt.scatter(entry.time, entry.phase_score, color='blue', s=1)
            #plt.scatter(entry.time, entry.tail_score, color='blue', s=1)
        plt.plot(datetimes, score_sums, color='blue')

        # Setup labels
        plt.xlabel('Time')
        plt.ylabel('Score')
        plt.legend()



    # Interpolate using a cubic spline
    times_numeric = numpy.array([dt.timestamp() for dt in datetimes])
    cs = CubicSpline(times_numeric, score_sums)

    # Generate a dense set of points for analysis
    dense_times = numpy.linspace(times_numeric.min(), times_numeric.max(), 1000)

    # Calculate the first derivative (slope)
    first_derivative = cs.derivative()(dense_times)

    smooth_delta = []
    sum_delta = []
    cur_delta_sum = 0
    last_delta = first_derivative[0]
    last_time = dense_times[0]
    for t in range(0, len(dense_times)):
        change_seconds = dense_times[t] - last_time
        cur_delta = first_derivative[t]
        change_mode = 1
        if cur_delta < last_delta:
            change_mode = -1
        max_change = 0.0005 * change_seconds
        if abs(cur_delta - last_delta) > max_change:
            cur_delta = last_delta + (max_change * change_mode)
        smooth_delta.append(cur_delta)
        last_delta = cur_delta

        cur_delta_sum += cur_delta
        sum_delta.append(cur_delta_sum / len(smooth_delta))

    #cs = CubicSpline(times_numeric, tilts)
    #dense_times = numpy.linspace(times_numeric.min(), times_numeric.max(), 1000)
    #dense_values = cs(dense_times)

    # Plot tail score
    GRAPH += 1
    plt.subplot(NUM_GRAPHS, 1, GRAPH)
    #for entry in entries:
    #    plt.scatter(entry.time, entry.tail_tilt, color='blue', s=1)
    #plt.plot(dense_times, dense_values, color='red')
    plt.plot(dense_times, smooth_delta, label="Smooth First Derivative")
    #plt.plot(dense_times, first_derivative, label="First Derivative")

    # Setup labels
    plt.xlabel('Time')
    plt.ylabel('Tilt')
    plt.legend()





    # Plot tail score
    GRAPH += 1
    plt.subplot(NUM_GRAPHS, 1, GRAPH)
    #for entry in entries:
    #    plt.scatter(entry.time, entry.tail_tilt, color='blue', s=1)
    #plt.plot(dense_times, dense_values, color='red')
    plt.plot(dense_times, sum_delta, label="Smooth First Derivative")
    #plt.plot(dense_times, first_derivative, label="First Derivative")

    # Setup labels
    plt.xlabel('Time')
    plt.ylabel('Tilt')
    plt.legend()



    return plt

if __name__ == '__main__':
    plt = create_plot(backend='Qt5Agg')
    plt.show()
