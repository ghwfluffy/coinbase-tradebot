# Trader Catalog

This document summarizes every trader algorithm shipped in the codebase: how each one works and why it can improve profitability in the mock/real strategies.

## WindowTrader (mean-reversion, risk-aware)
- **What it does**: Tracks a rolling price window and trades inside "safe" bands derived from window min/max or percentiles. Buys fixed notional tranches at discounts to recent averages; sells when price clears fee-adjusted profit targets, with trailing stops, soft exits, crash fire-sales, and caps on deployed capital.
- **Why it helps**: Harvests frequent small reversions to build volume for low fees while shedding risk during breaks, crashes, or spikes. Capital caps let multiple instances coexist without stomping the wallet.

## VolumeTrader (volume churner)
- **What it does**: Places a small buy then immediately a sell to rack up filled trades.
- **Why it helps**: Pushes 30‑day volume toward fee-tier thresholds so other traders pay lower maker/taker fees, increasing net profit margin.

## MomentumTrader (breakout rider)
- **What it does**: Watches rolling highs; buys on breakouts above a configured percent, exits on take-profit, trailing-drop, or stop-loss.
- **Why it helps**: Captures upside momentum moves that mean-reversion systems might miss or fade too early, diversifying regime coverage.

## MovingAverageTrader (trend follower)
- **What it does**: Uses short/long moving-average crossovers with entry/exit buffers; exits on cross-under, take-profit, trailing drop, or stop-loss.
- **Why it helps**: Stays long during sustained trends and steps aside when trend weakens, complementing range/mean-reversion behavior.

## ConstantTrader (static buyer)
- **What it does**: Continuously buys fixed-size lots at the current maker-biased price.
- **Why it helps**: Acts as a baseline DCA stream and a steady volume generator to lift fee tiers; useful in strongly bullish regimes.

## ConstantSpreadTrader (fixed spread market-maker)
- **What it does**: Posts paired buy/sell orders separated by a fixed spread around mid.
- **Why it helps**: Earns the bid/ask spread if fills occur, adding low-latency micro-profits and volume without directional bias.

## SpreadTrader (dynamic spread maker)
- **What it does**: Similar to ConstantSpreadTrader but can adjust spread and sizing based on observed volatility/conditions.
- **Why it helps**: Seeks to capture spread with adaptive sizing, improving win rate during variable volatility while contributing to fee-tier volume.

## StaticTrader (one-shot price taker)
- **What it does**: Executes a single configured buy or sell and stops.
- **Why it helps**: Handy for deterministic test setups, seeding inventory, or asserting behavior in mock runs; not a profit engine by itself.

## TimeTrader (scheduled trader)
- **What it does**: Fires trades on a time schedule (e.g., every N minutes) with fixed sizing.
- **Why it helps**: Provides time-based diversification and steady volume accrual; can serve as a heartbeat for fee-tier building or periodic rebalancing.

## Interplay and Profit Goal
- Combining **mean-reversion (Window)**, **trend/momentum (Momentum, MovingAverage)**, and **spread/volume (Constant/Spread/Volume)** covers multiple market regimes: chop, breakout, and drift.
- **VolumeTrader/Constant* and Spread** raise monthly volume, unlocking lower fees that directly improve net PnL for all traders.
- **Risk controls** (caps, stops, trailing exits, fire-sales) constrain drawdowns so the system can keep trading and compounding small edges.
