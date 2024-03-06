import datetime
import numpy as np
import matplotlib.pyplot as plt

from scipy.integrate import trapezoid
from scipy.interpolate import CubicSpline

def analyze_shape(points):
    datetimes = []
    values = []
    for x in points:
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

    # Calculate the second derivative (curvature)
    second_derivative = cs.derivative(2)(dense_times)

    # Analyze the shape based on derivatives
    # This is a simplified heuristic; you might need to adjust the thresholds based on your data
    if np.all(first_derivative > 0) and np.mean(np.abs(second_derivative)) < 0.01:
        shape = "Upwards Line"
    elif np.all(first_derivative < 0) and np.mean(np.abs(second_derivative)) < 0.01:
        shape = "Downwards Line"
    elif np.any(first_derivative[:-50] > 0) and np.all(np.abs(first_derivative[-50:]) < 0.01):  # Looking at the end of the curve for a plateau
        shape = "Upwards Line with a Hook"
    elif np.any(first_derivative[:-50] < 0) and np.all(np.abs(first_derivative[-50:]) < 0.01):
        shape = "Downwards Line with a Hook"
    else:
        shape = "Uncertain Shape"

    # Optionally, plot the curve and its derivative for visualization
    plt.figure(figsize=(12, 6))
    plt.subplot(3, 1, 1)
    plt.plot(dense_times, dense_values, label="Interpolated Curve")
    plt.title("Interpolated Curve and Derivatives")

    plt.subplot(3, 1, 2)
    plt.plot(dense_times, first_derivative, label="First Derivative")
    plt.legend()



    plt.subplot(3, 1, 3)
    #plt.plot(dense_times, first_derivative, label="First Derivative")
    plt.plot(dense_times, second_derivative, label="Second Derivative")
    plt.legend()


    # Calculate the area under the curve for the first derivative in the last 20% of the x-axis
    start_index = int(len(dense_times) * 0.8)  # Start from 80% of the x-axis to cover the last 20%
    area_under_curve = trapezoid(first_derivative[start_index:], dense_times[start_index:])
    print(area_under_curve)


    plt.show()


    return shape

# Function to generate wave-like data
def generate_wave_data(start_date_str, end_date_str):
    start_date = datetime.datetime.strptime(start_date_str, '%Y-%m-%d')
    end_date = datetime.datetime.strptime(end_date_str, '%Y-%m-%d')
    date_range = [start_date + datetime.timedelta(days=x) for x in range((end_date - start_date).days + 1)]

    x = np.arange(len(date_range))
    data = [(date_range[i], np.sin(2 * np.pi * (i / 30)) + np.random.normal(0, 0.1)) for i in x]
    return data

# Example usage
start_date_str = '2024-01-01'
end_date_str = '2024-03-31'
wave_data = generate_wave_data(start_date_str, end_date_str)

analyze_shape(wave_data)
