#!/usr/bin/env python3
"""
Read a JSONL file of snapshots and output a PNG containing 3 charts:
1) Line: profit over time by trader (absolute profit at each snapshot)
2) Bar: profit delta by weekday + trader (day-over-day deltas bucketed by current snapshot weekday)
3) Bar: volume delta by weekday + trader (day-over-day deltas bucketed by current snapshot weekday)

Usage:
  python charts.py /path/to/data.jsonl -o out.png
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any, Dict, List, Tuple
from datetime import datetime

import matplotlib.dates as mdates
import matplotlib.pyplot as plt

DOW_ABBR = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]


def read_jsonl(path: Path) -> List[Dict[str, Any]]:
    rows: List[Dict[str, Any]] = []
    with path.open("r", encoding="utf-8") as f:
        for i, line in enumerate(f, start=1):
            line = line.strip()
            if not line:
                continue
            try:
                rows.append(json.loads(line))
            except json.JSONDecodeError as e:
                raise SystemExit(f"Invalid JSON on line {i}: {e}") from e
    return rows


def parse_time(s: str) -> datetime:
    # Sample format: "2025-02-01 10:26:45"
    try:
        return datetime.strptime(s, "%Y-%m-%d %H:%M:%S")
    except ValueError:
        try:
            return datetime.fromisoformat(s)
        except ValueError as e:
            raise SystemExit(f"Unparseable time value: {s!r}") from e


def dt_to_dow(dt: datetime) -> str:
    return DOW_ABBR[dt.weekday()]


def dow_order(dows: List[str]) -> List[str]:
    order = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
    seen = {d for d in dows if d}
    return [d for d in order if d in seen] + sorted([d for d in seen if d not in order])


def collect_series_with_deltas(
    snapshots: List[Dict[str, Any]],
    *,
    clip_negative_volume_deltas: bool = True,
) -> Tuple[
    Dict[str, List[Tuple[datetime, float]]],  # profit_ts (absolute)
    Dict[Tuple[str, str], float],             # profit_delta_by_dow_trader (summed deltas)
    Dict[Tuple[str, str], float],             # volume_delta_by_dow_trader (summed deltas)
]:
    """
    For weekday bar charts, compute deltas between consecutive snapshots *per trader*:
      d_profit = curr_profit - prev_profit
      d_volume = curr_volume - prev_volume

    Bucket each delta into the weekday of the current snapshot.
    If clip_negative_volume_deltas=True, negative volume deltas are set to 0 (handles resets/rollovers).
    """

    # Per-trader points: (dt, dow, profit, volume)
    per_trader_points: Dict[str, List[Tuple[datetime, str, float, float]]] = {}
    profit_ts: Dict[str, List[Tuple[datetime, float]]] = {}

    for snap in snapshots:
        if "time" not in snap:
            continue
        dt = parse_time(str(snap["time"]))
        dow = str(snap.get("dayOfWeek") or dt_to_dow(dt))

        traders = snap.get("traders", [])
        if not isinstance(traders, list):
            continue

        for t in traders:
            if not isinstance(t, dict):
                continue

            name = str(t.get("name", "UNKNOWN"))

            try:
                profit = float(t.get("profit", 0.0))
            except (TypeError, ValueError):
                profit = float("nan")

            try:
                volume = float(t.get("volume", 0.0))
            except (TypeError, ValueError):
                volume = float("nan")

            per_trader_points.setdefault(name, []).append((dt, dow, profit, volume))
            profit_ts.setdefault(name, []).append((dt, profit))

    # Sort profit_ts for plotting
    for name in list(profit_ts.keys()):
        profit_ts[name].sort(key=lambda p: p[0])

    profit_delta_by_dow_trader: Dict[Tuple[str, str], float] = {}
    volume_delta_by_dow_trader: Dict[Tuple[str, str], float] = {}

    # Compute deltas per trader
    for name, pts in per_trader_points.items():
        pts.sort(key=lambda p: p[0])

        prev_profit: float | None = None
        prev_volume: float | None = None

        for (dt, dow, profit, volume) in pts:
            if prev_profit is None or prev_volume is None:
                prev_profit, prev_volume = profit, volume
                continue

            d_profit = profit - prev_profit
            d_volume = volume - prev_volume
            if clip_negative_volume_deltas and d_volume < 0:
                d_volume = 0.0

            profit_delta_by_dow_trader[(dow, name)] = profit_delta_by_dow_trader.get((dow, name), 0.0) + d_profit
            volume_delta_by_dow_trader[(dow, name)] = volume_delta_by_dow_trader.get((dow, name), 0.0) + d_volume

            prev_profit, prev_volume = profit, volume

    return profit_ts, profit_delta_by_dow_trader, volume_delta_by_dow_trader


def plot_grouped_bars(
    ax,
    x_labels: List[str],
    series: Dict[str, List[float]],
    title: str,
    ylabel: str,
):
    traders = sorted(series.keys())
    n_traders = max(1, len(traders))
    x = list(range(len(x_labels)))

    total_width = 0.8
    bar_w = total_width / n_traders
    offset0 = -total_width / 2 + bar_w / 2

    for i, trader in enumerate(traders):
        vals = series[trader]
        offs = [xi + offset0 + i * bar_w for xi in x]
        ax.bar(offs, vals, width=bar_w, label=trader)

    ax.set_title(title)
    ax.set_ylabel(ylabel)
    ax.set_xticks(x)
    ax.set_xticklabels(x_labels)
    ax.grid(True, axis="y", alpha=0.3)
    if traders:
        ax.legend(loc="best", fontsize="small")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("jsonl_path", type=Path, help="Path to JSONL input")
    ap.add_argument("-o", "--out", type=Path, default=Path("charts.png"), help="Output PNG path")
    ap.add_argument("--dpi", type=int, default=160)
    ap.add_argument(
        "--no-clip-negative-volume-deltas",
        action="store_true",
        help="If set, allow negative volume deltas (otherwise they are clipped to 0).",
    )
    args = ap.parse_args()

    snaps = read_jsonl(args.jsonl_path)
    if not snaps:
        raise SystemExit("No JSON objects found in input.")

    profit_ts, profit_delta_by_dow_trader, volume_delta_by_dow_trader = collect_series_with_deltas(
        snaps,
        clip_negative_volume_deltas=not args.no_clip_negative_volume_deltas,
    )

    # DOWs present
    all_dows = [k[0] for k in list(profit_delta_by_dow_trader.keys()) + list(volume_delta_by_dow_trader.keys())]
    dows = dow_order(all_dows) if all_dows else DOW_ABBR[:]  # fallback to full week if none

    # Traders present
    traders = sorted(
        set(profit_ts.keys())
        | set([k[1] for k in profit_delta_by_dow_trader.keys()])
        | set([k[1] for k in volume_delta_by_dow_trader.keys()])
    )
    if not traders:
        raise SystemExit("No traders found in input.")

    # Build grouped bar series arrays (aligned to dows)
    profit_bar_series: Dict[str, List[float]] = {t: [] for t in traders}
    volume_bar_series: Dict[str, List[float]] = {t: [] for t in traders}

    for dow in dows:
        for t in traders:
            profit_bar_series[t].append(float(profit_delta_by_dow_trader.get((dow, t), 0.0)))
            volume_bar_series[t].append(float(volume_delta_by_dow_trader.get((dow, t), 0.0)))

    # Figure layout: 3 rows
    fig, axes = plt.subplots(nrows=3, ncols=1, figsize=(12, 12), constrained_layout=True)

    # (1) Absolute profit over time
    ax0 = axes[0]
    for trader, pts in sorted(profit_ts.items(), key=lambda kv: kv[0]):
        pts_sorted = sorted(pts, key=lambda p: p[0])
        xs = [p[0] for p in pts_sorted]
        ys = [p[1] for p in pts_sorted]
        ax0.plot(xs, ys, marker="o", linewidth=1.8, label=trader)

    ax0.set_title("Profit Over Time by Trader (absolute)")
    ax0.set_ylabel("Profit")
    ax0.grid(True, alpha=0.3)
    ax0.legend(loc="best", fontsize="small")

    ax0.xaxis.set_major_locator(mdates.AutoDateLocator())
    ax0.xaxis.set_major_formatter(mdates.ConciseDateFormatter(ax0.xaxis.get_major_locator()))
    fig.autofmt_xdate(rotation=0)

    # (2) Profit deltas by weekday + trader
    plot_grouped_bars(
        axes[1],
        x_labels=dows,
        series=profit_bar_series,
        title="Profit by Day of Week and Trader (sum of deltas)",
        ylabel="Profit (delta)",
    )

    # (3) Volume deltas by weekday + trader
    plot_grouped_bars(
        axes[2],
        x_labels=dows,
        series=volume_bar_series,
        title="Volume by Day of Week and Trader (sum of deltas)",
        ylabel="Volume (delta)",
    )

    args.out.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(args.out, dpi=args.dpi)
    plt.close(fig)

    print(f"Wrote {args.out}")


if __name__ == "__main__":
    main()
