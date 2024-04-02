import io
import matplotlib
import matplotlib.pyplot as plt
from scipy.interpolate import interp1d
from typing import List

from gtb.core.context import Context
from gtb.utils.logging import Log
from gtb.market.volume import get_volume_orders, MarketVolume

Log.INFO = False

def create_demand_plot() -> None:
    # Get market data
    ctx: Context = Context()
    volume: MarketVolume = get_volume_orders(ctx)

    # Figure out the "demand" at each market delta
    MAX_WINDOW = 2000
    SAMPLE_PERIOD = 10

    market_delta: List[int] = []
    demand: List[float] = []
    window: int = 1
    roots: List[int] = []
    while window <= MAX_WINDOW:
        bid = volume.get_bids(window)
        ask = volume.get_asks(window)
        if bid <= 0 or ask <= 0:
            window += 1
            continue
        market_delta.append(window)
        demand.append(bid / ask)
        window += SAMPLE_PERIOD

        if len(demand) > 1:
            change: int = 0
            if demand[-2] <= 1.0 and demand[-1] >= 1.0:
                change = 1
            elif demand[-2] >= 1.0 and demand[-1] <= 1.0:
                change = -1

            if change != 0:
                for subwindow in range(market_delta[-2], market_delta[-1] + 1):
                    bid = volume.get_bids(subwindow)
                    ask = volume.get_asks(subwindow)
                    if bid <= 0 or ask <= 0:
                        continue
                    curdemand: float = bid / ask
                    if change == 1 and curdemand >= 1.0:
                        roots.append(subwindow)
                        break
                    elif change == -1 and curdemand <= 1.0:
                        roots.append(subwindow)
                        break

    # Graph the data
    matplotlib.use('Agg')

    # Interpolate our data points
    cubic_interp = interp1d(market_delta, demand, kind='cubic')

    # New x values for a smoother curve
    demand_smooth = cubic_interp(market_delta)

    # Plotting
    plt.figure(figsize=(8, 4))
    plt.plot(market_delta, demand, 'o', label='Data')
    plt.plot(market_delta, demand_smooth, '-', label='Interpolated')
    plt.plot([0, MAX_WINDOW], [1, 1], color='black')
    for r in roots:
        plt.scatter(r, 1.0, color='black', zorder=2, label="x={}".format(r))
    plt.legend()
    plt.title('Market Demand')
    plt.xlabel('Market Delta')
    plt.ylabel('Demand')

def create_volume_plot() -> None:
    # Get market data
    ctx: Context = Context()
    volume: MarketVolume = get_volume_orders(ctx)

    # Figure out the "demand" at each market delta
    MAX_WINDOW = 2000
    SAMPLE_PERIOD = 10

    market_delta: List[int] = []
    demand: List[float] = []
    bids: List[float] = []
    asks: List[float] = []
    window: int = 1
    while window <= MAX_WINDOW:
        bid = volume.get_bids(window)
        ask = volume.get_asks(window)
        if bid <= 0 and ask <= 0:
            window += 1
            continue
        market_delta.append(window)
        bids.append(bid)
        asks.append(ask)
        window += SAMPLE_PERIOD

    # Graph the data
    matplotlib.use('Agg')

    # Interpolate our data points
    cubic_interp_bids = interp1d(market_delta, bids, kind='cubic')
    cubic_interp_asks = interp1d(market_delta, asks, kind='cubic')

    # New x values for a smoother curve
    bids_smooth = cubic_interp_bids(market_delta)
    asks_smooth = cubic_interp_asks(market_delta)

    # Plotting
    plt.figure(figsize=(8, 4))
    plt.plot(market_delta, bids_smooth, '-', label='Bids')
    plt.plot(market_delta, asks_smooth, '-', label='Asks')
    plt.legend()
    plt.title('Market Volme')
    plt.xlabel('Market Delta')
    plt.ylabel('Volume')

if __name__ == '__main__':
    #create_demand_plot()
    create_volume_plot()
    #plt.show()

    # Save the plot to a BytesIO object
    buf = io.BytesIO()
    plt.savefig(buf, format='png')
    plt.close()

    # Seek to the beginning of the BytesIO buffer
    buf.seek(0)

    # Read the buffer content into a byte array
    byte_array = buf.getvalue()

    # Output to test file
    with open("/tmp/graph.png", "wb") as fp:
        fp.write(byte_array)
