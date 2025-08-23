#!/usr/bin/env python3

import sqlite3
import pandas as pd
import matplotlib.pyplot as plt
import pytz
from datetime import datetime, time

# Connect & load data
conn = sqlite3.connect('../mock_historical.sqlite')
df = pd.read_sql_query("SELECT time, price FROM btc_price", conn)
conn.close()

# Convert microseconds to EST datetime
df['datetime'] = pd.to_datetime(df['time'], unit='us', utc=True)\
                   .dt.tz_convert('US/Eastern')

# Plot BTC price over time
plt.figure(figsize=(12,6))
plt.plot(df['datetime'], df['price'], label='BTC Price')

est = pytz.timezone('US/Eastern')
# Get unique dates (as naive dates)
unique_dates = sorted(df['datetime'].dt.date.unique())

for d in unique_dates:
    # NYSE (Mon-Fri): open at 9:30, close at 16:00
    if d.weekday() < 5:
        nyse_open = est.localize(datetime.combine(d, time(9,30)))
        nyse_close = est.localize(datetime.combine(d, time(16,0)))
        plt.axvline(nyse_open, color='green', linestyle='--', alpha=0.5,
                    label='NYSE Open' if d==unique_dates[0] else "")
        plt.axvline(nyse_close, color='red', linestyle='--', alpha=0.5,
                    label='NYSE Close' if d==unique_dates[0] else "")
    
    # Bitcoin Futures:
    # Closed daily 17:00-18:00; plus extended weekend closure:
    wd = d.weekday()
    if wd < 4:  # Mon-Thu: add both daily boundaries
        fut_close = est.localize(datetime.combine(d, time(17,0)))
        fut_open  = est.localize(datetime.combine(d, time(18,0)))
        plt.axvline(fut_close, color='blue', linestyle='--', alpha=0.5,
                    label='BTC Futures Close' if d==unique_dates[0] else "")
        plt.axvline(fut_open,  color='blue', linestyle='--', alpha=0.5,
                    label='BTC Futures Open' if d==unique_dates[0] else "")
    elif wd == 4:  # Friday: only close at 17:00 (then closed till Sunday)
        fut_close = est.localize(datetime.combine(d, time(17,0)))
        plt.axvline(fut_close, color='blue', linestyle='--', alpha=0.5,
                    label='BTC Futures Close' if d==unique_dates[0] else "")
    elif wd == 6:  # Sunday: open at 18:00
        fut_open = est.localize(datetime.combine(d, time(18,0)))
        plt.axvline(fut_open, color='blue', linestyle='--', alpha=0.5,
                    label='BTC Futures Open' if d==unique_dates[0] else "")

plt.xlabel("Time (EST)")
plt.ylabel("Price")
plt.title("Bitcoin Price Over Time")
plt.legend()
plt.tight_layout()
plt.savefig("btc_price.png")
plt.close()
