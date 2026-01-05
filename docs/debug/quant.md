# Codex Quant Loop: Key Takeaways (codex01)

This summarizes Codex’s iterative findings from `docs/experiment/codex01.md` to guide future tuning.

## High-Level Themes
- Stock open/close pain: Largest drawdowns consistently around ~08:30 (stock open) and, to a lesser extent, ~18:28–19:01 (late session/close), Tue/Wed (and Mon/Fri earlier) are worst.
- Volume reporting gap: Status volume remains frozen (~$49.7, fee 0.60%) despite millions of per-trader volume; likely a reporting/aggregation bug gating fee tiers.
- Main draggers: Spread-Small/BreakEven and Time-Small/Time-Medium bleed most; volume feeders/pulse/drip carry small intentional bleed.
- Drawdown trend: Gradual improvements via sizing cuts and shielding; max DD ~ -$5.5K early, now ~ -$1.2–1.5K in later iters.
- Per-trader volumes remain ample (hundreds of trades), but status fee never improves because volume not reflected.

## Evolution & Adjustments
- Added/off: AllHoursProbe enabled, AllHours factory removed (default is all-hours when `marketParams` empty).
- Gating/ramping: Applied shielded/gentle stock-hour profiles to reduce open/close whipsaws; trimmed exposure on Spread/Time offenders; weekend derisk on riskier traders.
- Volume configs: Added Pulse/Feeder/BtcFeeder to lift volume; trimmed bets/TTL/reprice to manage bleed; StockChurn heavily gated.
- Sizing trims: Progressive reductions on Spread/Time/Pulse/Feeder/BtcFeeder to curb losses while keeping cadence.

## Current State (latest summarized)
- Wallet ~-$0.9K to -$1.0K vs $50K start; BTC ~20–30%.
- Status volume still ~$49.7 (fee 0.60%) despite per-trader volumes ~0.7–1.2M aggregate per run.
- Drawdown ~ -$1.2K; biggest single loss ~-$450 to -$700 at 08:30 Tue; biggest wins ~$300–$500 at 08:30 Mon/Wed, 18:28 Wed.
- Per-trader PnL: AllHoursProbe/BreakEven remain top spread bleeds; Time-Medium/Time-Small negative but reduced; volume feeders/pulse/drip modest negative by design; StockChurn minimal loss with gating.

## Open Issues / Next Targets
- Fix status volume/fee reporting so true 30d volume is reflected; current fee tier stuck at 0.60%.
- Further damp stock opens (Tue/Wed) for Spread/Time and perhaps volume traders; consider day-specific dampers.
- Continue trimming spread/time sizing or pairs if bleed persists; avoid killing off-hour coverage.
- Monitor DB lock errors (sporadic earlier, mostly resolved later).

## Parsing Reminders
- Use `STATUS` lines for wallet/profit/drawdown timeline; per-trader PnL/volume lines to rank offenders and counts.
- Large move detection: diff consecutive STATUS profits; bucket by hour/day to spot open/close hits.
- Errors: `rg "ERROR|WARN|locked|Overflow|Not enough"` for anomalies; DB locks noted around 2025-02-28/09-05 earlier iters.
