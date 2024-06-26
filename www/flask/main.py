from flask import Flask, jsonify

import io
import os
import base64
import subprocess

import matplotlib.pyplot as plt

from gtb.inspect.graph import create_plot

app = Flask("tradebot")

@app.route('/info', methods=['GET'])
def get_info():
    data = {
        "ghw": "Fluffy",
    }
    return jsonify(data)

@app.route('/logs', methods=['GET'])
def get_logs():
    logs = ""
    with open("/var/tradebot/logs/log.txt", "r") as fp:
        logs = fp.readlines()
    if len(logs) > 50:
        logs = logs[-50:]
    logs = "".join(logs)
    data = {
        "logs": logs,
    }
    return jsonify(data)

@app.route('/wallet', methods=['GET'])
def get_wallet():
    cmd = ['sudo', '-u', 'ghw', './bin/wallet.sh']
    process = subprocess.run(cmd, text=True, capture_output=True)
    stdout = process.stdout
    data = {
        "wallet": stdout,
    }
    return jsonify(data)

@app.route('/pnl', methods=['GET'])
def get_pnl():
    cmd = ['sudo', '-u', 'ghw', './bin/pnl.sh']
    process = subprocess.run(cmd, text=True, capture_output=True)
    stdout = process.stdout
    data = {
        "pnl": stdout,
    }
    return jsonify(data)

@app.route('/spread-stats', methods=['GET'])
def get_spread_stats():
    cmd = ['sudo', '-u', 'ghw', './bin/spread_stats.sh']
    process = subprocess.run(cmd, text=True, capture_output=True)
    stdout = process.stdout
    data = {
        "stats": stdout,
    }
    return jsonify(data)

@app.route('/orders', methods=['GET'])
def get_orders():
    cmd = ['sudo', '-u', 'ghw', './bin/orders.sh']
    process = subprocess.run(cmd, text=True, capture_output=True)
    stdout = process.stdout
    data = {
        "orders": stdout,
    }
    return jsonify(data)

@app.route('/history', methods=['GET'])
def get_history():
    cmd = ['sudo', '-u', 'ghw', './bin/history.sh']
    process = subprocess.run(cmd, text=True, capture_output=True)
    stdout = process.stdout
    data = {
        "history": stdout,
    }
    return jsonify(data)

@app.route('/alloc', methods=['GET'])
def get_alloc():
    cmd = ['sudo', '-u', 'ghw', './bin/allocations.sh']
    process = subprocess.run(cmd, text=True, capture_output=True)
    stdout = process.stdout
    data = {
        "alloc": stdout,
    }
    return jsonify(data)

@app.route('/count', methods=['GET'])
def get_count():
    cmd = ['sudo', '-u', 'ghw', './bin/counter.sh']
    process = subprocess.run(cmd, text=True, capture_output=True)
    stdout = process.stdout
    data = {
        "count": stdout,
    }
    return jsonify(data)

@app.route('/graph', methods=['GET'])
def get_graph():
    orig_dir = os.getcwd()
    try:
        os.chdir('/var/tradebot')
        create_plot()

        # Save the plot to a BytesIO object
        buf = io.BytesIO()
        plt.savefig(buf, format='png')  # Save as PNG to the buffer
        plt.close()  # Close the plot

        # Seek to the beginning of the BytesIO buffer
        buf.seek(0)

        # Read the buffer content into a byte array
        byte_array = buf.getvalue()

        data = {
            "data": base64.b64encode(byte_array).decode('utf-8'),
        }
        return jsonify(data)
    finally:
        os.chdir(orig_dir)

@app.route('/demand', methods=['GET'])
def get_demand():
    cmd = ['sudo', '-u', 'ghw', './bin/demand_graph.sh']
    process = subprocess.run(cmd, text=True, capture_output=True)
    stdout = process.stdout
    data = {
        "data": stdout,
    }
    return jsonify(data)

@app.route('/volume', methods=['GET'])
def get_volume():
    cmd = ['sudo', '-u', 'ghw', './bin/volume_graph.sh']
    process = subprocess.run(cmd, text=True, capture_output=True)
    stdout = process.stdout
    data = {
        "data": stdout,
    }
    return jsonify(data)
