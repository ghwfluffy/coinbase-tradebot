# Window Trader Strategy Intent

High-level goals and behaviors for the rolling window mean-reversion trader.

- **Objective**: harvest small mean-reversion moves with high trade count (to reach low/zero fee tiers) while capping downside during crashes and runaway spikes.
- **Data**: maintains a rolling price window (size = `Config::windowSize`, sliced into `Config::candleSize` candles) using the older half of the window to anchor bands and percentiles.
- **Buy posture**:
  - Only buys inside a safe band: min/max window with optional percentile bands; adaptive middle band (default P20–P60) blocks chasing highs or catching falling knives.
  - Spacing and trend guards prevent stacking too close or buying through clear downtrends.
  - Buys can be forced to sit a fixed dollar *and* percent below the running average cost (`buyDelta`, `buyBelowPct`) so new buys are meaningful discounts.
  - Bet size can scale with volatility to reduce size in choppy markets.
- **Sell posture**:
  - Profit-take vs. average cost plus fee buffer and configurable delta or percent lift (`takeProfitDelta`, `takeProfitPct`); partial exits allowed.
  - Trailing/high-water and volatility boosts tighten exits when momentum fades.
  - Stop-loss band can shed part of the position if price sinks a set percent below the running average.
  - Soft exits shed part of the position when price drifts below a low percentile, even without profit, to reduce exposure.
- **Crash/spike handling**:
  - Defensive fire-sale when price pierces the low percentile guard; sets a crash cooldown and pauses re-entry until a percentile-based recovery is seen.
  - Pauses new buys when price breaches a high percentile guard; sells remain allowed so we can exit into strength.
- **Risk limits**:
  - Spend/exposure caps, per-trader capital caps (e.g., only deploy $20k from a larger wallet per trader), buy spacing, and pause timers limit runaway allocation.
  - Window reset on fire-sale to avoid stale anchors.
 - **Order placement (maker bias)**:
  - Prices for buys/sells are offset using shared helpers (`makerBuyPrice`/`makerSellPrice`) to prefer maker orders (e.g., ±$2) and keep fee tiers low.
- **Logging/toggles**:
  - Trade-level logs can be toggled with `--trade-logs`; debug logs with `--debug-logs`. Mock time is reflected in logs for replay analysis.

Tunable levers live in `WindowTrader::Config` (see `src/traders/WindowTrader.h`) and can be adjusted. The intent is fast, frequent mean-reversion trades inside the “safe zone,” with aggressive risk-off behavior when the market breaks out or breaks down.***
