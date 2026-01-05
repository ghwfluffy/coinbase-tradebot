Iteration 1 — 2026-01-05T01:20:30Z
- Metrics (data/log-1.txt): start Wallet $50,000 → end $49,880.51 (profit -$119.49), max drawdown ≈ -$234.60. Final STATUS volume reads $0.00 (fee 0.60%); per-trader volumes: BtcFeeder $7.5K, Drip $5.8K, Feeder $6.0K, Pulse $5.6K, StockChurn $0.87K (≈$25.8K total). No ERROR/WARN lines found.
- Observations: Volume grossly below the $5M target and status volume appears broken (fee stuck 0.60%). Profit very stable with tiny swings; profit deltas ≥$25 cluster around 08:00 (stock open analogue) and 18:00 (late session), matching existing Stock/BitcoinFutures market times—no new tracked market time needed. Per-trader PnL modestly negative (≈-$40 to -$50 each; StockChurn -$6.7), suggesting room to increase cadence without breaching -$1K/month.
- Hypothesis: VolumeTrader throughput is throttled by wide profit deltas/reprice bands and slower TTLs; tightening deltas/bands and shortening TTL should produce more completed cycles and lift 30d volume while keeping losses contained near current small bleed.
- Expected effect / falsifier: expect materially higher per-trader volume and reported status volume with similar or only slightly worse PnL; falsified if volume stays near ~$25K or if PnL loss accelerates past ~$300 per trader (risking >$1K/month aggregate).
- Next step ideas: if volume still low, consider allowing a second concurrent pair or modest bet increases; instrument status-volume computation to explain the $0/$49.7K freezes; add day-specific dampers only if new loss clusters emerge.

Iteration 2 — 2026-01-05T01:43:05Z
- Metrics (data/log-2.txt): start $50,000 → end $49,876.24 (profit -$123.76), max drawdown ≈ -$238.98. Final STATUS volume $0.00 (fee 0.60%); per-trader volumes: BtcFeeder $6.6K, Drip $7.0K, Feeder $7.0K, Pulse $7.7K, StockChurn $0.99K (~$29.3K total). Final per-trader PnL: BtcFeeder -$43.50, Drip -$44.84, Feeder -$46.11, Pulse -$47.63, StockChurn -$6.73. No ERROR/WARN/lock lines found.
- Observations: Volume rose slightly vs iteration 1 but remains orders of magnitude below the $5M goal; STATUS volume still broken at $0/fee 0.60%. Profit remains very stable with small negative bleed (~-$189 aggregate). Profit deltas ≥$25 again cluster around 08:00 and 18:00, matching existing Stock/BitcoinFutures times—no new market needed. Minimal drift between start/end suggests TTL/delta tightening is safe so far.
- Hypothesis: Throughput is still throttled; cutting min deltas, reprice bands, and TTLs further for all VolumeTrader variants should push more cycles without materially increasing bleed given the tiny current losses.
- Expected effect / falsifier: expect higher per-trader volumes (>>$30K total) and a visible STATUS volume increase if reporting is functional; falsified if volumes stay flat or if PnL loss grows past ~$400 total (risking >$1K/month) after the change.
- Next step ideas: if volume remains low after tighter bands, consider allowing multiple concurrent pairs per trader or modest bet bumps; instrument STATUS volume calculation if freezes persist.

Iteration 3 — 2026-01-05T01:49:42Z
- Metrics (data/log-18.txt from data/runs/codex01/18.txt.zstd): start $50,000 → end ~$48,125.57 (profit -$1,874.43), max drawdown ≈ -$2,899.78. Final STATUS volume stuck at $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $524.5K / -$548.15; BtcFeederHeavy $640.2K / -$669.22; Drip $176.6K / -$187.43; Feeder $314.7K / -$333.39; Pulse $221.8K / -$231.49; StockChurn $25.5K / -$27.39 (≈$1.9M total volume). Largest swings: +$863.81 (Mon 08:30), -$1,153.32 (Tue 08:30); big move buckets cluster at 08:30 opens and ~18:28–19:01 closes across the week.
- Observations: Volume is far below the $5M target despite tighter bands; STATUS volume still broken. Losses are concentrated in BtcFeeder and its heavy variant; total bleed ~-$2K over a ~10-month mock (~-$200/mo), leaving room to trade harder. Time-of-day aligns with existing Stock/BitcoinFutures periods—no new market needed.
- Hypothesis: Volume cadence is still too slow; further shrinking minProfitDelta/reprice band/TTL for all VolumeTrader variants should increase fills and total volume while keeping monthly loss under the $1K cap, given current ~-$200/mo bleed.
- Expected effect / falsifier: expect materially higher per-trader volume (>>$2M total) and faster cycles; falsified if volume barely moves or if loss rate exceeds ~$1K/month equivalent (e.g., >-$10K over the mock span).
- Next step ideas: if volume remains low, consider enabling multiple concurrent pairs per trader or modest bet bumps; instrument STATUS volume computation to resolve the fee/volume freeze.

Iteration 4 — 2026-01-05T01:51:28Z
- Metrics (data/log-21.txt from data/runs/codex01/21.txt.zstd): start $50,000 → end ~$48,321.77 (profit -$1,678.23), max drawdown ≈ -$2,810.53. Final STATUS volume stuck at $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $382.0K / -$422.49; BtcFeederHeavy $553.8K / -$594.75; Drip $176.2K / -$194.05; Feeder $193.5K / -$206.37; Pulse $221.8K / -$240.56; StockChurn $25.5K / -$28.39 (≈$1.55M total). Largest swings: +$838.10 (Mon 08:30), -$1,119.10 (Tue 08:30); big move buckets again at 08:30 opens and ~18:28–19:01 closes across the week.
- Observations: Volume improved vs prior iteration but still well below the $5M target; STATUS volume remains frozen. Loss rate ~-$170/mo remains under the -$1K/mo limit, suggesting room to raise sizing. Open/close clusters match existing Stock/BitcoinFutures periods—no new market bucket needed.
- Hypothesis: Increase VolumeTrader bet sizes (while keeping tight deltas/bands/TTLs) to push volume closer to target; current bleed is low enough to absorb higher notional.
- Expected effect / falsifier: expect materially higher per-trader volume (aiming toward several million total) without monthly loss exceeding ~$1K (i.e., >-$10K over the mock span); falsified if volume stays ~1–2M or losses accelerate past that threshold.
- Next step ideas: if volume still too low after sizing up, consider multiple concurrent pairs per trader or explicit volume instrumentation to fix STATUS reporting; if losses jump near opens, revisit market gating for volume traders.

Iteration 5 — 2026-01-05T01:52:53Z
- Metrics (data/log-22.txt from data/runs/codex01/22.txt.zstd): start $50,000 → end ~$48,366.84 (profit -$1,633.16), max drawdown ≈ -$2,776.40. Final STATUS volume stuck at $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $336.3K / -$376.52; BtcFeederHeavy $479.3K / -$526.01; Drip $176.6K / -$197.65; Feeder $169.4K / -$185.14; Pulse $221.8K / -$246.01; StockChurn $25.5K / -$28.94 (≈$1.41M total). Largest swings: +$828.28 (Mon 08:30), -$1,106.02 (Tue 08:30); big move buckets still cluster around 08:30 opens and ~18:28–19:01 closes.
- Observations: Sizing up improved volume only marginally (down to ~1.4M total) while PnL improved slightly; STATUS volume still frozen. Loss rate (~-$164/mo) remains under the $1K/month guardrail; biggest bleeds remain the two BTC feeders. Time-of-day patterns unchanged; no new MarketInfo bucket needed.
- Hypothesis: With losses comfortably low, raise VolumeTrader bet sizes further to push volume toward several million while keeping tight deltas/bands/TTLs.
- Expected effect / falsifier: expect a noticeable jump in total per-trader volume (toward multi-million) without loss rate exceeding ~$1K/month (≈ -$10K over the mock span); falsified if volume barely moves or losses spike past that rate.
- Next step ideas: if volume still short after this size bump, consider multiple concurrent pairs per VolumeTrader or instrument STATUS volume aggregation to address the freeze.

Iteration 6 — 2026-01-05T01:53:50Z
- Metrics (data/log-23.txt from data/runs/codex01/23.txt.zstd): start $50,000 → end ~$48,365.42 (profit -$1,634.58), max drawdown ≈ -$2,776.40. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $332.5K / -$368.75; BtcFeederHeavy $482.0K / -$529.98; Drip $176.6K / -$197.63; Feeder $172.3K / -$188.35; Pulse $221.8K / -$246.24; StockChurn $25.5K / -$28.94 (~$1.41M total). Largest swings: +$828.28 (Mon 08:30), -$1,106.02 (Tue 08:30); move buckets unchanged (08:30 opens, ~18:28–19:01 closes).
- Observations: Further sizing barely moved volume; losses remain modest (~-$164/mo) and below the guardrail. STATUS volume still frozen. Biggest bleeds still in BTC feeders; open/close timing unchanged; no new market needed.
- Hypothesis: Increase bet sizes again (keeping tight deltas/bands/TTLs) to force higher notional turnover since losses are tolerable.
- Expected effect / falsifier: expect total per-trader volume to jump toward several million; falsified if volume remains ~1–2M or if loss rate exceeds ~$1K/month equivalent (>-$10K over the mock span).
- Next step ideas: if volume still low, enable multiple concurrent pairs per VolumeTrader and/or instrument STATUS volume aggregation to diagnose the freeze.

Iteration 7 — 2026-01-05T01:54:49Z
- Metrics (data/log-24.txt from data/runs/codex01/24.txt.zstd): start $50,000 → end ~$48,621.56 (profit -$1,378.44), max drawdown ≈ -$2,658.24. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $338.3K / -$405.74; BtcFeederHeavy $– (not present in this run); Drip $176.2K / -$213.00; Feeder $169.4K / -$202.45; Pulse $221.8K / -$261.71; StockChurn $25.5K / -$31.14 (~$1.23M+ total). Largest swings: +$794.27 (Mon 08:30), -$1,060.74 (Tue 08:30); big move buckets remain at 08:30 and ~18:28–19:01.
- Observations: Volume dipped vs prior run (especially btc feeders), but loss improved (~-$138/mo). STATUS volume still frozen; no new market times needed. Still well below $5M volume target; risk remains within guardrail.
- Hypothesis: Bump StockChurn bet further to gain more turnover during stock hours without materially increasing bleed (currently small).
- Expected effect / falsifier: expect modest volume lift from StockChurn; falsified if volume gain is negligible or loss accelerates past ~$1K/month equivalent.
- Next step ideas: if volume remains low, consider multi-pair support for volume traders and instrument STATUS volume aggregation to resolve fee/volume freeze.

Iteration 8 — 2026-01-05T01:56:00Z
- Metrics (data/log-25.txt from data/runs/codex01/25.txt.zstd): start $50,000 → end ~$48,678.06 (profit -$1,321.94), max drawdown ≈ -$2,135.86. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $337.6K / -$415.98; BtcFeederHeavy absent; Drip $176.6K / -$221.60; Feeder $175.8K / -$210.52; Pulse $221.8K / -$268.05; StockChurn $25.5K / -$31.89 (~$1.14M total). Largest swings: +$634.87 (Mon 08:30), -$848.53 (Tue 08:30); loss buckets still at stock opens/late session.
- Observations: Volume slipped further and STATUS still frozen; losses improved (~-$132/mo) and remain under the guardrail. Open/close timing unchanged; no new market needed. Volume remains far from $5M, suggesting we need more concurrency, sizing, or instrumentation next.
- Hypothesis: Increase Drip bet (preferBitcoinHours) to raise off-hours volume while keeping tight spreads/TTLs; bleed is modest so risk remains acceptable.
- Expected effect / falsifier: expect higher total volume contribution from Drip with losses still under ~$1K/month; falsified if volume barely changes or Drip losses spike materially.
- Next step ideas: if volume remains low after this, add multiple concurrent pairs per VolumeTrader and instrument STATUS volume aggregation to fix the reporting freeze.

Iteration 9 — 2026-01-05T01:57:00Z
- Metrics (data/log-26.txt from data/runs/codex01/26.txt.zstd): start $50,000 → end ~$48,725.18 (profit -$1,274.82), max drawdown ≈ -$1,967.53. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $292.1K / -$371.22; BtcFeederHeavy absent; Drip $176.6K / -$224.78; Feeder $171.7K / -$209.39; Pulse $221.8K / -$272.05; StockChurn $25.5K / -$32.37 (~$1.16M total). Largest swings: +$585.31 (Mon 08:30), -$782.61 (Tue 08:30); move buckets mostly at 08:30 and late sessions.
- Observations: PnL improved (smaller drawdown, profit closer to breakeven) but volume still ~1.1–1.2M and STATUS frozen; open/close pattern unchanged. Loss rate ~-$127/mo remains below guardrail; still far from $5M target.
- Hypothesis: Increase Feeder bet (all-hours with gating) to drive more turnover across sessions without overshooting the $1K/month loss cap.
- Expected effect / falsifier: expect Feeder volume to rise meaningfully (lifting total toward multi-million) while losses stay under ~$1K/month equivalent; falsified if volume barely moves or losses spike.
- Next step ideas: if volume remains low, enable multiple concurrent pairs for volume traders and instrument STATUS volume reporting.

Iteration 10 — 2026-01-05T01:57:52Z
- Metrics (data/log-27.txt from data/runs/codex01/27.txt.zstd): start $50,000 → end ~$48,795.29 (profit -$1,204.71), max drawdown ≈ -$1,852.12. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $254.7K / -$326.82; BtcFeederHeavy absent; Drip $176.6K / -$228.37; Feeder $170.7K / -$213.71; Pulse $221.8K / -$276.58; StockChurn $25.5K / -$32.86 (~$0.85–1.0M total). Largest swings: +$551.42 (Mon 08:30), -$737.51 (Tue 08:30); move buckets still concentrated at stock opens/late sessions.
- Observations: Volume slipped further; STATUS volume frozen. Loss rate (~-$120/mo) remains low; biggest drag still BTC feeders. No new market time needed—same open/close clustering.
- Hypothesis: Add BtcFeederHeavy back to Version2 to boost volume using existing heavy config; bleed historically modest relative to target.
- Expected effect / falsifier: expect a material volume increase (additional ~0.5M+ per run) without pushing loss rate beyond ~$1K/month equivalent; falsified if volume barely rises or losses spike past that rate.
- Next step ideas: if volume still low after re-adding heavy feeder, enable multiple concurrent pairs per VolumeTrader and instrument STATUS volume reporting to resolve the fee/volume freeze.

Iteration 11 — 2026-01-05T01:59:30Z
- Metrics (data/log-28.txt from data/runs/codex01/28.txt.zstd): start $50,000 → end ~$48,847.59 (profit -$1,152.41), max drawdown ≈ -$1,749.60. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $205.8K / -$268.87; BtcFeederHeavy absent; Drip $176.6K / -$234.06; Feeder $168.5K / -$214.33; Pulse $221.8K / -$282.54; StockChurn $25.5K / -$33.43 (~$1.0M total). Largest swings: +$521.36 (Mon 08:30), -$697.50 (Tue 08:30); move buckets still at opens/late sessions.
- Observations: Volume fell again and STATUS remains frozen; losses improved (~-$115/mo) and are well within guardrail. No new market needed. Volume still far from $5M.
- Hypothesis: Increase Drip bet further (preferBitcoinHours) to lift off-hours volume while keeping losses under control given current low bleed.
- Expected effect / falsifier: expect Drip volume to rise and total volume to inch upward without exceeding ~$1K/month loss equivalent; falsified if volume barely changes or Drip losses spike materially.
- Next step ideas: if volume remains low, move to multi-pair volume traders and instrument STATUS volume reporting to resolve the freeze.

Iteration 12 — 2026-01-05T02:01:31Z
- Metrics (data/log-30.txt from data/runs/codex01/30.txt.zstd): start $50,000 → end ~$48,923.84 (profit -$1,076.16), max drawdown ≈ -$1,548.95. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $204.2K / -$267.88; BtcFeederHeavy absent; Drip $176.6K / -$239.86; Feeder $167.8K / -$216.71; Pulse $221.8K / -$287.26; StockChurn $25.5K / -$33.91 (~$0.99M total). Largest swings: +$445.88 (Mon 08:30), -$608.71 (Tue 08:30); move buckets still concentrated at opens/late sessions.
- Observations: Volume is stagnant (~$1M) and STATUS frozen; losses improved (~-$108/mo) and remain below guardrail. Heavy feeder hasn’t been active in recent runs; off-hours volume remains low.
- Hypothesis: Lower BtcFeederHeavy min delta/band/TTL to match lighter feeders so the heavy profile engages more and increases volume.
- Expected effect / falsifier: expect BtcFeederHeavy to activate and add significant volume (several hundred K per run) without pushing loss rate past ~$1K/month equivalent; falsified if heavy stays idle or losses spike.
- Next step ideas: if volume remains low after this, enable multiple concurrent pairs per VolumeTrader and instrument STATUS volume to resolve the reporting freeze.

Iteration 13 — 2026-01-05T02:07:41Z
- Metrics (data/log-33.txt from data/runs/codex01/33.txt.zstd): start $50,000 → end ~$49,036.10 (profit -$963.90), max drawdown ≈ -$1,405.87. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $213.0K / -$280.45; BtcFeederHeavy absent; Drip $176.6K / -$239.46; Feeder $170.7K / -$221.90; Pulse $195.2K / -$254.04; StockChurn $25.5K / -$34.09 (~$0.8–0.9M total). Largest swings: biggest loss -$539.42 (Tue 08:30), biggest gain +$395.30 (Mon 08:30); move buckets unchanged (08:30 opens, late sessions).
- Observations: Volume still ~1M and STATUS frozen; losses improved (~-$96/mo) and remain within guardrail. Heavy feeder continues to stay idle; open/close clustering unchanged; no new market needed.
- Hypothesis: Reduce minProfitDelta on Feeder and BtcFeeder (to 0.25) to increase fill rate and volume without materially increasing bleed.
- Expected effect / falsifier: expect per-trader volumes to rise (hundreds of K) with loss rate still under ~$1K/month equivalent; falsified if volume barely changes or losses spike past that rate.
- Next step ideas: if volume remains low, move to multiple concurrent pairs per VolumeTrader and instrument STATUS volume reporting to address the freeze.

Iteration 14 — 2026-01-05T02:10:40Z
- Metrics (data/log-39.txt from data/runs/codex01/39.txt.zstd): start $50,000 → end ~$49,046.55 (profit -$953.45), max drawdown ≈ -$1,300.92. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $211.4K / -$290.69; Drip $132.5K / -$188.36; Feeder $186.2K / -$256.07; Pulse $177.0K / -$239.62; StockChurn $25.5K / -$35.60 (~$0.73–0.8M total; heavy feeder still absent). Largest swings: -$494.85 (Tue 08:30), +$369.14 (Mon 08:30); move buckets still at opens/late sessions.
- Observations: Volume slipped (~0.75M) and STATUS remains frozen; losses improved (~-$95/mo) and are below guardrail. Heavy feeder still idle; no new market needed; open/close clustering persists.
- Hypothesis: Lower Pulse minProfitDelta (0.25) to increase fill rate/volume while keeping losses within the ~$1K/month cap.
- Expected effect / falsifier: expect Pulse volume to rise and total volume to tick up without losses exceeding ~$1K/month equivalent; falsified if volume barely changes or Pulse losses spike.
- Next step ideas: if volume remains low, enable multiple concurrent pairs per VolumeTrader and instrument STATUS volume to resolve the freeze.

Iteration 15 — 2026-01-05T02:12:10Z
- Metrics (data/log-40.txt from data/runs/codex01/40.txt.zstd): start $50,000 → end ~$49,051.73 (profit -$948.27), max drawdown ≈ -$1,264.61. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $207.0K / -$289.36; Drip $133.1K / -$190.56; Feeder $180.4K / -$249.88; Pulse $177.4K / -$242.13; StockChurn $25.5K / -$35.91 (~$0.72–0.8M total; heavy feeder still absent). Largest swings: -$479.15 (Tue 08:30), +$357.34 (Mon 08:30); move buckets still at opens/late sessions.
- Observations: Volume remains ~0.7–0.8M and STATUS frozen; losses improved (~-$95/mo) and remain well under guardrail. Heavy feeder still idle; timing clusters unchanged; no new market needed.
- Hypothesis: Lower StockChurn minProfitDelta to 0.25 to increase churn during stock hours and raise total volume, leveraging low bleed.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and total volume to increase without pushing losses past ~$1K/month equivalent; falsified if volume barely changes or losses spike materially.
- Next step ideas: if volume remains low, enable multiple concurrent pairs per VolumeTrader and instrument STATUS volume reporting to resolve the freeze.

Iteration 16 — 2026-01-05T02:15:21Z
- Metrics (data/log-42.txt from data/runs/codex01/42.txt.zstd): start $50,000 → end ~$49,066.85 (profit -$933.15), max drawdown ≈ -$1,225.40. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $205.8K / -$287.93; Drip $132.5K / -$190.00; Feeder $179.7K / -$252.07; Pulse $177.4K / -$243.27; StockChurn $25.5K / -$35.93 (~$0.77M total; heavy feeder still absent). Largest swings: -$454.08 (Tue 08:30), +$331.20 (Mon 08:30); move buckets still at opens/late sessions; no new market needed.
- Observations: Volume stuck under ~0.8M with STATUS frozen; losses ~-$93/mo, well below guardrail. Heavy feeder still idle; open/close clustering unchanged.
- Hypothesis: Reduce Pulse order TTL (12s→8s) to reprice faster and capture more fills, lifting volume without materially increasing bleed.
- Expected effect / falsifier: expect Pulse volume to rise and total volume tick upward; falsified if volume barely changes or losses spike past ~$1K/month equivalent.
- Next step ideas: if volume remains low, enable multiple concurrent pairs per VolumeTrader and instrument STATUS volume reporting to fix the freeze.

Iteration 17 — 2026-01-05T02:18:14Z
- Metrics (data/log-46.txt from data/runs/codex01/46.txt.zstd): start $50,000 → end ~$49,075.41 (profit -$924.59), max drawdown ≈ -$1,118.15. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $205.8K / -$298.74; Drip $131.6K / -$196.55; Feeder $183.1K / -$265.32; Pulse $159.7K / -$231.86; StockChurn $25.5K / -$38.02 (~$0.71M total; heavy feeder still absent). Largest swings: -$454.08? (this run largest loss -$454.08), biggest gain +$331.20 (Mon 08:30); move buckets unchanged (opens/late sessions).
- Observations: Volume dropped further (~0.7M) and STATUS still frozen; losses improved (~-$92/mo) and well within guardrail. Heavy feeder still idle; open/close clustering unchanged; no new market needed.
- Hypothesis: Shorten Feeder (all-hours) and BtcFeeder TTLs to increase repricing cadence and fill rate, lifting volume without breaching the $1K/month loss cap.
- Expected effect / falsifier: expect per-trader volumes to rise meaningfully (hundreds of K) with loss rate still under ~$1K/month equivalent; falsified if volume barely changes or losses spike.
- Next step ideas: if volume remains low, move to multiple concurrent pairs per VolumeTrader and instrument STATUS volume reporting to fix the freeze.

Iteration 18 — 2026-01-05T02:21:55Z
- Metrics (data/log-47.txt from data/runs/codex01/47.txt.zstd): start $50,000 → end ~$49,076.84 (profit -$923.16), max drawdown ≈ -$1,112.62. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $207.4K / -$302.69; Drip $132.8K / -$199.71; Feeder $180.0K / -$263.97; Pulse $151.5K / -$220.71; StockChurn $25.5K / -$38.19 (~$0.7–0.8M total; heavy feeder still absent). Largest swings: -$396.91 (Tue 08:30), +$295.57 (Mon 08:30); move buckets unchanged (opens/late sessions); no new market needed.
- Observations: Volume remains stuck below 1M and STATUS frozen; losses ~-$92/mo and within guardrail. Heavy feeder still idle; volume shortfall likely due to slow engagement.
- Hypothesis: Loosen BtcFeederHeavy to minDelta 0.10, band 6, TTL 8s so the heavy profile actually trades and lifts volume.
- Expected effect / falsifier: expect heavy feeder to activate and add several hundred K per run without pushing losses beyond ~$1K/month equivalent; falsified if heavy remains idle or losses spike.
- Next step ideas: if volume still low after heavy engagement, move to multi-pair volume traders and instrument STATUS volume reporting to resolve the freeze.

Iteration 19 — 2026-01-05T02:24:32Z
- Metrics (data/log-49.txt from data/runs/codex01/49.txt.zstd): start $50,000 → end ~$49,091.66 (profit -$908.34), max drawdown ≈ -$1,103.78. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL (final block): BtcFeeder $186.7K / -$278.97; Drip $132.5K / -$200.38; Feeder $181.1K? (approx from block) / -$266.60; Pulse $150.8K / -$220.90; StockChurn $25.5K / -$38.36 (~$1.16M total; heavy feeder still absent). Largest swings: -$394.90 (Tue 08:30), +$294.07 (Mon 08:30); move buckets unchanged (opens/late sessions).
- Observations: Volume ~1.16M, STATUS frozen; losses ~-$91/mo and within guardrail. Heavy feeder still idle despite loosening; volume shortfall persists. No new market needed.
- Hypothesis: Add a second BtcFeederHeavy instance to push more bitcoin-hours volume, given low bleed and idle heavy slot.
- Expected effect / falsifier: expect material volume increase (additional several hundred K/run) without exceeding ~$1K/month loss equivalent; falsified if volume barely moves or losses spike.
- Next step ideas: if volume still low after duplicating heavy feeder, enable multiple concurrent pairs per VolumeTrader and instrument STATUS volume reporting to resolve the freeze.

Iteration 20 — 2026-01-05T02:26:39Z
- Metrics (data/log-50.txt from data/runs/codex01/50.txt.zstd): start $50,000 → end ~$49,086.88 (profit -$913.12), max drawdown ≈ -$1,104.88. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL (final block): BtcFeeder $192.1K / -$279.27; Drip $132.5K / -$200.40; Feeder $187.2K / -$271.66; Pulse $150.8K / -$220.76; StockChurn $25.5K / -$38.34 (~$1.17M total; heavy feeder still absent). Largest swings: -$394.89 (Tue 08:30), +$294.06 (Mon 08:30); move buckets unchanged; no new market needed.
- Observations: Volume stalled ~1.17M and STATUS frozen; losses ~-$91/mo within guardrail. Heavy feeder still idle; need more volume via active feeders.
- Hypothesis: Increase Feeder and BtcFeeder bet sizes (340/320 → 400) to push more notional in their active profiles while losses are low.
- Expected effect / falsifier: expect per-trader volumes to rise (hundreds of K) without loss rate exceeding ~$1K/month equivalent; falsified if volume barely changes or losses spike.
- Next step ideas: if volume remains low, move to multi-pair volume traders and instrument STATUS volume reporting to fix the freeze.

Iteration 21 — 2026-01-05T02:28:11Z
- Metrics (data/log-52.txt from data/runs/codex01/52.txt.zstd): start $50,000 → end ~$49,092.71 (profit -$907.29), max drawdown ≈ -$1,085.85. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$188.5K / -$278.36; Drip $132.8K / -$203.86; Feeder ~$180.7K / -$268.66; Pulse $151.1K / -$224.54; StockChurn $25.5K / -$38.99 (~$1.16M total; heavy feeder still absent). Largest swings: -$380.43 (Tue 08:30), +$275.87 (Mon 08:30); move buckets unchanged; no new market needed.
- Observations: Volume flat around ~1.16M and STATUS frozen; losses ~-$91/mo within guardrail. Heavy feeder still idle; StockChurn small; open/close clustering persists.
- Hypothesis: Add a second StockChurn instance to increase stock-hour churn, nudging volume upward with minimal additional risk.
- Expected effect / falsifier: expect tens of K more volume from stock hours without pushing losses past ~$1K/month equivalent; falsified if volume barely moves or losses spike materially.
- Next step ideas: if volume stays low, move to multi-pair support for volume traders and instrument STATUS volume reporting to resolve the freeze.

Iteration 22 — 2026-01-05T02:30:40Z
- Metrics (data/log-53.txt from data/runs/codex01/53.txt.zstd): start $50,000 → end ~$49,094.28 (profit -$905.72), max drawdown ≈ -$1,084.63. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$178.0K / -$263.86; Drip $132.5K / -$204.84; Feeder ~$181.4K / -$270.56; Pulse $150.8K / -$225.63; StockChurn $25.5K / -$39.25 (~$1.14M total; heavy feeder absent). Largest swings: +$285.35 (Mon 08:30), -$383.29 (Tue 08:30), +$253.17 (Wed 08:30); clustering at stock opens, no new market time needed.
- Observations: Volume plateaued near ~1.14M with STATUS still frozen at $49.70; losses steady (~-$91/mo) and within guardrail. Heavy feeder still missing from runtime configs (log shows only one btc feeder); config printouts indicate the run used older sizing (bet ~170, TTL 30–120s), so faster stock churn may help. No errors/locks detected; BTC share ~32% by end.
- Hypothesis: Shorten StockChurn TTL (20s→10s) to reprice twice as often during stock hours, driving more fills/volume from the stock-gated traders without meaningfully increasing risk given the small bet size.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and total volume to tick up; falsified if volume remains ~1.1M or if losses accelerate toward the $1K/month cap.
- Next step ideas: if volume remains stuck, instrument VolumeTrader config logging per instance to confirm duplicates, and consider enabling multi-pair cycling or removing midweek gating on BTC feeders to activate the heavy profiles and lift bitcoin-hour volume; continue pursuing STATUS volume reporting fix.

Iteration 54 — 2026-01-05T02:33:16Z
- Metrics (data/log-54.txt from data/runs/codex01/54.txt.zstd): start $50,000 → end ~$49,089.74 (profit -$910.26), max drawdown ≈ -$1,082.93. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$180.4K / -$269.54; Drip $132.8K / -$204.87; Feeder ~$182.8K / -$273.78; Pulse $150.8K / -$224.05; StockChurn $25.5K / -$38.91 (~$1.14M total; heavy feeder absent). Largest swings: +$285.35 (Mon 08:30), -$383.29 (Tue 08:30), +$253.17 (Wed 08:30), +$243.10 (Wed 18:28); clustering at opens/late session, so no new market time needed.
- Observations: Volume remains stuck near ~1.14M with STATUS frozen; losses steady (~-$91/mo) inside guardrail. Runtime configs still show only the legacy five VolumeTrader profiles (bets ~170, TTLs 30–120s), implying builds are not picking up our newer settings/duplicates; heavy feeder still absent. No errors/locks.
- Hypothesis: Remove midweek derisk gating from BtcFeederHeavy so the heavy bitcoin-hours profiles stay active throughout the week and finally add volume.
- Expected effect / falsifier: expect heavy feeders to appear in configs and contribute several hundred K of volume, without pushing losses beyond the ~$1K/month cap; falsified if heavy remains idle or losses spike materially.
- Next step ideas: if heavy still idle, instrument per-instance VolumeTrader config logging (name + instance index) and consider multi-pair support to raise cadence; continue chasing the STATUS volume reporting fix to unlock fee-tier improvements.

Iteration 55 — 2026-01-05T02:34:28Z
- Metrics (data/log-55.txt from data/runs/codex01/55.txt.zstd): start $50,000 → end ~$49,091.73 (profit -$908.27), max drawdown ≈ -$1,083.81. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$179.0K / -$268.30; Drip $132.5K / -$203.90; Feeder ~$183.1K / -$273.18; Pulse $150.8K / -$224.72; StockChurn $25.5K / -$39.07 (~$1.15M total; heavy feeder still absent). Largest swings: -$379.42 (Tue 08:30), +$275.12 (Mon 08:30), +$254.97 (Wed 08:30), +$244.23 (Wed 18:28), -$228.32 (Tue 08:30); clustering at stock opens and late session, no new market time needed.
- Observations: Volume flat near ~1.15M and STATUS frozen; losses steady (~-$91/mo) within guardrail. Heavy feeders still missing; config logs again show only the legacy five VolumeTrader profiles, suggesting builds are still running old params (bets ~170, long TTLs) rather than our tighter settings.
- Hypothesis: Lower minProfitDelta on the active BtcFeeder (0.25→0.15) to increase fill rate and raise bitcoin-hours volume since the light feeder is running reliably and bleed is modest.
- Expected effect / falsifier: expect BtcFeeder volume to rise materially (several tens of K+) with losses still under the ~$1K/month cap; falsified if volume stays ~1.15M or BtcFeeder losses spike sharply.
- Next step ideas: if volume remains capped or heavy stays absent, add per-instance VolumeTrader config logging to verify which configs deploy, and consider multi-pair support or more aggressive sizing on the active feeders; continue pursuing a fix for STATUS volume reporting.

Iteration 56 — 2026-01-05T02:36:10Z
- Metrics (data/log-56.txt from data/runs/codex01/56.txt.zstd): start $50,000 → end ~$49,097.08 (profit -$902.92), max drawdown ≈ -$1,081.37. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$176.0K / -$262.63; Drip $132.5K / -$204.84; Feeder ~$177.3K / -$267.27; Pulse $151.1K / -$226.01; StockChurn $25.5K / -$39.25 (~$1.13M total; heavy feeder still absent). Largest swings: -$383.29 (Tue 08:30), +$285.34 (Mon 08:30), +$253.17 (Wed 08:30), +$243.10 (Wed 18:28), -$235.54 (Tue 08:30); clustering at stock opens and late session, no new market time needed.
- Observations: Volume remains stuck near ~1.13M with STATUS frozen; losses steady (~-$90/mo) inside guardrail. Heavy feeders still missing; config logs continue to show only legacy five VolumeTrader profiles with old params, indicating deployments still running stale settings despite recent changes.
- Hypothesis: Remove midweek derisk gating from the active BtcFeeder so it can trade all week and lift bitcoin-hours volume while keeping deltas tight.
- Expected effect / falsifier: expect BtcFeeder volume to rise (tens of K+) and total volume to tick above current ~1.1M; falsified if volume remains flat or losses spike toward the ~$1K/month cap.
- Next step ideas: if volume stays capped or configs stay stale, add per-instance VolumeTrader config logging (with params) to verify deployed settings, and consider multi-pair support or explicit rebuild hooks; continue pursuing a STATUS volume reporting fix.

Iteration 57 — 2026-01-05T02:37:25Z
- Metrics: data/log-57.txt not found; most recent available log is data/runs/codex01/56.txt.zstd (see Iteration 56). No new metrics recorded.
- Observations: Unable to confirm whether recent config changes deployed because the expected log is missing.
- Hypothesis: Tighten Volume-Feeder reprice band (6→4) to improve fill rate and lift all-hours volume without changing bet size.
- Expected effect / falsifier: expect Feeder volume to rise meaningfully (tens of K+) and total volume to move above the current ~1.1M baseline; falsified if volume stays flat or losses jump toward the ~$1K/month cap.
- Next step ideas: if the next run still shows legacy configs or flat volume, add per-instance VolumeTrader config logging (with params) to verify deployments, and consider multi-pair support; continue chasing STATUS volume reporting fix.

Iteration 58 — 2026-01-05T02:38:33Z
- Metrics: data/log-58.txt not found; prior iteration failed to build (compiler error), so no new run data is available.
- Observations: Cannot yet verify deployment of recent config changes; build must succeed before next run.
- Hypothesis: Reduce Volume-BtcFeeder minProfitDelta to 15_Cents to raise fill rate and bitcoin-hours volume without changing bet size.
- Expected effect / falsifier: expect BtcFeeder volume to increase (tens of K+) and total volume to lift above ~1.1M; falsified if volume stays flat or BtcFeeder losses spike toward the ~$1K/month cap.
- Next step ideas: ensure the next build passes so changes deploy; if configs remain stale or heavy feeders stay absent, add per-instance VolumeTrader config logging (with params) and consider multi-pair support; continue pursuing STATUS volume reporting fix.

Iteration 59 — 2026-01-05T02:51:18Z
- Metrics (data/log-59.txt): start $50,000 → end ~$47,599.25 (profit -$2,400.75), max drawdown ≈ -$2,455.00. Final STATUS volume stuck at $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$868.1K / -$985.96; BtcFeederHeavy ~$938.8K / -$1,073.88; Drip $344.4K / -$398.72; Feeder $823.3K / -$953.23; Pulse $318.3K / -$368.20; StockChurn $206.6K / -$238.14 (~$3.50M total). Largest swings: -$336.07 (Tue 08:30), +$252.44 (Mon 08:30), +$220.30 (Wed 08:30), -$367.61 (Thu 15:33), +$243.10 (Wed 18:28); clustering still around stock opens/late session, with one mid-afternoon Thu hit—no new tracked market time needed yet.
- Observations: Volume improved to ~3.5M but still below the 5M target; STATUS volume remains broken at $0. Heavy feeders are active but bleeding ~-$1K each; aggregate loss ~-$240/mo equivalent, within the $1K/mo cap. Configs reflect recent changes (minDelta 0.15 on btc feeder, reprice band 4 on feeder). Biggest drawdowns still around 08:30; one-off 15:33 loss noted but not frequent.
- Hypothesis: Lower StockChurn minProfitDelta to 50_Cents to boost stock-hour churn/volume with limited additional risk given small bet size.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and total volume to move closer to the $5M goal without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or losses accelerate materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps on feeders; add per-instance VolumeTrader config logging if any config drift reappears; continue pursuing STATUS volume reporting fix.

Iteration 60 — 2026-01-05T03:03:56Z
- Metrics (data/log-60.txt): start $50,000 → end ~$47,600.40 (profit -$2,399.60), max drawdown ≈ -$2,453.83. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$860.9K / -$983.65; BtcFeederHeavy ~$937.0K / -$1,069.59; Drip $343.8K / -$399.51; Feeder $823.3K / -$950.88; Pulse $318.9K / -$366.71; StockChurn $208.2K / -$238.20 (~$3.49M total). Largest swings: -$336.03 (Tue 08:30), +$252.41 (Mon 08:30), +$220.28 (Wed 08:30), -$364.42 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume steady near ~3.5M and STATUS still broken at $0; losses similar to prior run and within the $1K/mo guardrail. Heavy feeders engaged; configs show latest params (btc feeder 0.15 delta, feeder band 4, stock churn 0.50 delta). Drawdowns still cluster at opens plus occasional mid-afternoon spikes.
- Hypothesis: Lower Volume-Feeder minProfitDelta to 50_Cents to increase all-hours fill rate and push total volume closer to the $5M target while holding bet size constant.
- Expected effect / falsifier: expect Feeder volume to rise (hundreds of K) and total volume to improve without losses jumping toward the $1K/mo cap; falsified if volume remains ~3.5M or losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 61 — 2026-01-05T03:16:15Z
- Metrics (data/log-61.txt): start $50,000 → end ~$47,591.11 (profit -$2,408.89), max drawdown ≈ -$2,463.12. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$867.2K / -$982.64; BtcFeederHeavy ~$942.4K / -$1,072.86; Drip $346.5K / -$397.31; Feeder $841.7K / -$967.57; Pulse $318.3K / -$364.72; StockChurn $208.2K / -$237.06 (~$3.52M total). Largest swings: -$336.03 (Tue 08:30), +$252.41 (Mon 08:30), +$220.28 (Wed 08:30), -$367.65 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume holding ~3.5M with STATUS still frozen; losses similar to prior run and within the $1K/mo cap. Heavy feeders active; configs reflect current params. Drawdowns still dominated by opens and a mid-afternoon spike.
- Hypothesis: Further lower Volume-Feeder minProfitDelta to 25_Cents to lift all-hours fill rate/volume while keeping bet size fixed.
- Expected effect / falsifier: expect Feeder volume to rise toward the remaining ~$1.5M gap to the 5M target without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or losses jump materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 62 — 2026-01-05T03:29:18Z
- Metrics (data/log-62.txt): start $50,000 → end ~$47,584.84 (profit -$2,415.16), max drawdown ≈ -$2,469.41. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$862.5K / -$985.94; BtcFeederHeavy ~$937.9K / -$1,073.32; Drip $344.4K / -$395.13; Feeder $862.5K / -$982.03; Pulse $318.3K / -$364.02; StockChurn $208.4K / -$236.59 (~$3.53M total). Largest swings: -$336.03 (Tue 08:30), +$252.41 (Mon 08:30), +$220.27 (Wed 08:30), -$366.95 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume flat at ~3.5M and STATUS still broken; losses similar and within the $1K/mo cap. Heavy feeders active; drawdowns still cluster at opens and a recurring Thu 15:33 hit.
- Hypothesis: Tighten btc feeder repricing bands (6→4) on both light/heavy to increase fill rate and push volume toward $5M while keeping deltas unchanged.
- Expected effect / falsifier: expect BTC feeder volumes to rise (hundreds of K) and total volume to move closer to $5M without losses breaching the ~$1K/mo cap; falsified if volume stays ~3.5M or BTC feeder losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; instrument per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 63 — 2026-01-05T03:41:46Z
- Metrics (data/log-63.txt): start $50,000 → end ~$47,597.93 (profit -$2,402.07), max drawdown ≈ -$2,456.32. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$846.5K / -$969.34; BtcFeederHeavy ~$919.0K / -$1,054.38; Drip $344.4K / -$399.62; Feeder $860.9K / -$986.11; Pulse $318.3K / -$365.03; StockChurn $208.2K / -$237.90 (~$3.50M total). Largest swings: -$336.03 (Tue 08:30), +$252.41 (Mon 08:30), +$220.27 (Wed 08:30), -$366.22 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume remains ~3.5M with STATUS frozen; losses roughly unchanged and within the $1K/mo cap. BTC feeders show slightly lower volume post band-tightening; drawdown pattern unchanged (opens + Thu 15:33 spike). Need more cadence without larger losses.
- Hypothesis: Shorten Volume-Feeder order TTL (10s→6s) to reprice faster and increase all-hours fill rate/volume without changing bet or delta.
- Expected effect / falsifier: expect Feeder volume to rise (hundreds of K) and total volume to move toward $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or losses jump materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 74 — 2026-01-05T05:57:43Z
- Metrics (data/log-74.txt): start $50,000 → end ~$47,591.00 (profit -$2,409.00), max drawdown ≈ -$2,463.24. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$861.7K / -$990.84; BtcFeederHeavy ~$917.2K / -$1,055.69; Drip $345.1K / -$398.21; Feeder $832.8K / -$970.29; Pulse $315.3K / -$367.63; StockChurn $208.4K / -$241.00 (~$3.48M total). Largest swings: -$336.06 (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$364.37 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume remains ~3.5M and STATUS frozen; losses steady and within guardrail. StockChurn TTL was 6s this run; effect small. Drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Shorten BtcFeeder order TTL (8s→6s) to increase repricing cadence and lift bitcoin-hours volume without changing deltas/bets.
- Expected effect / falsifier: expect BTC feeder volumes to rise (hundreds of K) and total volume to move toward $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or BTC feeder losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 75 — 2026-01-05T06:10:57Z
- Metrics (data/log-75.txt): start $50,000 → end ~$47,594.79 (profit -$2,405.21), max drawdown ≈ -$2,459.45. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$844.1K / -$978.41; BtcFeederHeavy ~$929.8K / -$1,063.89; Drip $345.1K / -$398.21; Feeder $832.9K / -$967.08; Pulse $315.3K / -$366.97; StockChurn $208.2K / -$240.33 (~$3.48M total). Largest swings: -$336.06 (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$364.72 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume remains ~3.5M and STATUS frozen; losses steady and within guardrail. BTC feeders still the largest contributors; drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Shorten BtcFeeder order TTL (8s→6s) to increase repricing cadence and lift bitcoin-hours volume without changing deltas/bets.
- Expected effect / falsifier: expect BTC feeder volumes to rise (hundreds of K) and total volume to move toward $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or BTC feeder losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 76 — 2026-01-05T06:23:34Z
- Metrics (data/log-76.txt): start $50,000 → end ~$47,594.58 (profit -$2,405.42), max drawdown ≈ -$2,459.65. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$844.0K / -$978.13; BtcFeederHeavy ~$920.8K / -$1,053.68; Drip $347.2K / -$402.20; Feeder $832.8K / -$968.36; Pulse $315.3K / -$366.97; StockChurn $208.4K / -$240.35 (~$3.48M total). Largest swings: -$336.06 (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$365.36 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume stuck near ~3.48M with STATUS frozen; losses steady and within guardrail. BTC feeder TTL change not yet reflected; drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Shorten Drip order TTL (30s→20s) to reprice faster in off-hours and add incremental volume without changing bet or delta.
- Expected effect / falsifier: expect Drip volume to rise (tens–hundreds of K) and total volume to move toward $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or Drip losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 77 — 2026-01-05T06:35:55Z
- Metrics (data/log-77.txt): start $50,000 → end ~$47,588.93 (profit -$2,411.07), max drawdown ≈ -$2,465.33. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$844.1K / -$978.13; BtcFeederHeavy ~$930.7K / -$1,061.87; Drip $349.9K / -$405.18; Feeder $832.8K / -$967.40; Pulse $315.3K / -$366.93; StockChurn $208.2K / -$240.06 (~$3.48M total). Largest swings: -$336.06 (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$364.72 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume still ~3.48M and STATUS frozen; losses steady and within guardrail. Pulse/Feeder/Drip changes not yet yielding volume lift; drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Shorten StockChurn order TTL again (6s→4s) to force faster repricing during stock hours and squeeze extra churn/volume without changing bet size or delta.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and total volume to move closer to $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or StockChurn losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 72 — 2026-01-05T05:31:49Z
- Metrics (data/log-72.txt): start $50,000 → end ~$47,592.39 (profit -$2,407.61), max drawdown ≈ -$2,461.85. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$865.7K / -$989.78; BtcFeederHeavy ~$931.6K / -$1,066.45; Drip $350.6K / -$401.62; Feeder $824.1K / -$963.90; Pulse $315.9K / -$367.64; StockChurn $216.6K / -$242.56 (~$3.50M total). Largest swings: -$336.06 (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$361.52 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume flat around ~3.5M and STATUS still frozen; losses steady and within guardrail. Pulse TTL change not reflected yet; drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Shorten StockChurn order TTL (10s→6s) to reprice faster during stock hours and add churn/volume without raising bet size.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and total volume to move toward $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or StockChurn losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 73 — 2026-01-05T05:44:43Z
- Metrics (data/log-73.txt): start $50,000 → end ~$47,591.00 (profit -$2,409.00), max drawdown ≈ -$2,463.24. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$861.7K / -$990.84; BtcFeederHeavy ~$917.2K / -$1,055.69; Drip $345.1K / -$398.21; Feeder $832.8K / -$970.29; Pulse $315.3K / -$367.63; StockChurn $208.4K / -$241.00 (~$3.48M total). Largest swings: -$336.06 (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$364.37 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume still ~3.5M and STATUS frozen; losses steady and within guardrail. Recent Pulse/Feeder changes still not moving total volume; drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Shorten StockChurn order TTL further (6s→4s) to force faster repricing during stock hours and squeeze extra churn/volume without changing bet size or delta.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and total volume to move closer to $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or StockChurn losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 78 — 2026-01-05T06:50:29Z
- Metrics: data/log-78.txt missing (no new run produced). Latest available run remains data/log-77.txt: wallet ~$47,588.93 (profit -$2,411.07), max DD ≈ -$2,465.33, total per-trader volume ~ $3.48M with STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL baseline (77): BtcFeeder ~$844.1K / -$978.13; BtcFeederHeavy ~$930.7K / -$1,061.87; Drip $349.9K / -$405.18; Feeder $832.8K / -$967.40; Pulse $315.3K / -$366.93; StockChurn $208.2K / -$240.06. Largest swings still cluster at 08:30 open and Thu 15:33; no new market time to add.
- Observations: Missing log indicates prior run likely didn’t execute after code changes; build now compiles cleanly. Performance baseline unchanged (volume ~3.48M, STATUS frozen, losses steady within ~$1K/mo guardrail) and drawdown pattern remains open/Thu-hit heavy.
- Hypothesis: Lower Volume-Drip minProfitDelta (50_Cents → 35_Cents) to raise off-hour fill rate and push total volume upward without increasing bet size.
- Expected effect / falsifier: expect Drip volume to rise by tens–low hundreds of K and nudge total volume toward $5M while keeping bleed similar; falsified if total volume stays near $3.5M or Drip losses jump meaningfully.
- Next step ideas: ensure next run produces log-78; monitor Drip volume/loss and STATUS volume reporting; if volume still short, consider further delta trims or adding multi-pair coverage after STATUS fix.

Iteration 78 — 2026-01-05T06:51:45Z
- Metrics (data/log-78.txt): start $50,000 → end ~$47,591.33 (profit -$2,408.67), max drawdown ≈ -$2,462.93. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$838.5K / -$974.06; BtcFeederHeavy ~$928.0K / -$1,059.49; Drip $349.2K / -$406.94; Feeder $832.0K / -$966.86; Pulse $314.7K / -$364.14; StockChurn $219.0K / -$246.94 (~$3.48M total). Largest swings: +$252.41 (Mon 08:30), -$336.03 (Tue 08:30), +$220.28 (Wed 08:30), -$367.51 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume flat ~3.48M. Drip delta cut to 35¢ produced negligible volume change (~-0.7K vs prior) and similar bleed. Drawdowns remain clustered at stock opens and a recurring Thu 15:33 hit.
- Hypothesis: Lower Volume-Pulse minProfitDelta (50_Cents → 40_Cents) to increase fills during stock/bitcoin hours and lift total volume without changing sizing.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and nudge total volume toward $5M while keeping losses near current ~$1K/mo pace; falsified if volume stays ~3.5M or Pulse losses spike materially.
- Next step ideas: Monitor Pulse volume/loss after delta cut; if volume still short, consider modest bet bump on Pulse or Drip, or multi-pair support after STATUS reporting is fixed; continue tracking Thu 15:33 drawdown behavior.

Iteration 79 — 2026-01-05T07:04:42Z
- Metrics (data/log-79.txt): start $50,000 → end ~$47,588.70 (profit -$2,411.30), max drawdown ≈ -$2,465.55. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$840.9K / -$977.84; BtcFeederHeavy ~$936.1K / -$1,064.62; Drip $351.2K / -$404.65; Feeder $837.7K / -$972.52; Pulse $315.9K / -$364.81; StockChurn $218.6K / -$245.53 (~$3.50M total). Largest swings: +$252.40 (Mon 08:30), -$336.02 (Tue 08:30), +$220.27 (Wed 08:30), -$368.23 (Thu 15:33); no new market time needed.
- Observations: Total volume inched up to ~$3.50M but STATUS remains frozen. Pulse delta cut to 40¢ yielded only a small volume lift; losses and drawdown pattern unchanged (open hits and Thu 15:33 spike). Drip/Feeder volumes steady; BTC feeders still dominate volume and bleed.
- Hypothesis: Lower Volume-Pulse minProfitDelta further (40_Cents → 35_Cents) to increase fills during stock/bitcoin hours and push volume closer to $5M without altering sizing.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and move total volume toward the $5M target while keeping losses near current ~$1K/mo pace; falsified if volume stays ~3.5M or Pulse losses jump materially.
- Next step ideas: After observing Pulse response, consider modest bet bumps or multi-pair support if volume still short; continue monitoring Thu 15:33 drawdowns and pursue STATUS volume reporting fix.

Iteration 80 — 2026-01-05T07:17:15Z
- Metrics (data/log-80.txt): start $50,000 → end ~$47,589.67 (profit -$2,410.33), max drawdown ≈ -$2,464.59. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$837.6K / -$972.92; BtcFeederHeavy ~$932.5K / -$1,059.38; Drip $351.9K / -$407.67; Feeder $831.2K / -$966.99; Pulse $314.1K / -$364.45; StockChurn $217.6K / -$246.16 (~$3.48M total). Largest swings: +$252.41 (Mon 08:30), -$336.03 (Tue 08:30), +$220.28 (Wed 08:30), -$368.69 (Thu 15:33); no new market time needed.
- Observations: Volume slipped back toward ~3.48M and STATUS remains frozen. Pulse at 35¢ delta shows no meaningful volume lift yet; drawdowns unchanged (08:30 opens, Thu 15:33). BTC feeders still carry most volume/bleed; Drip/Feeder steady.
- Hypothesis: Tighten Volume-Pulse repriceBand (6 → 5) to increase repricing frequency and capture more fills during stock/bitcoin hours without changing sizing or delta.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and nudge total volume toward $5M while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or Pulse losses spike materially.
- Next step ideas: If Pulse volume still flat, consider modest bet bumps or expanding pair coverage once STATUS volume reporting is fixed; continue monitoring Thu 15:33 drawdowns.

Iteration 81 — 2026-01-05T07:29:55Z
- Metrics (data/log-81.txt): start $50,000 → end ~$47,591.93 (profit -$2,408.07), max drawdown ≈ -$2,462.30. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$840.1K / -$974.59; BtcFeederHeavy ~$935.2K / -$1,072.00; Drip $349.9K / -$402.29; Feeder $832.9K / -$963.75; Pulse $312.9K / -$363.56; StockChurn $219.4K / -$246.76 (~$3.49M total). Largest swings: +$252.41 (Mon 08:30), -$336.03 (Tue 08:30), +$220.28 (Wed 08:30), -$368.02 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume flat ~3.49M. Pulse delta/band tweaks have not moved volume materially; drawdown pattern unchanged (open hits and Thu 15:33). BTC feeders remain the largest volume/bleed contributors.
- Hypothesis: Shorten Volume-Feeder orderTtl (6s → 5s) to reprice faster across all hours and lift overall fill rate without changing bet size or delta.
- Expected effect / falsifier: expect Feeder volume to rise (hundreds of K) and push total volume closer to $5M while keeping losses near the current ~$1K/mo pace; falsified if volume stays ~3.5M or Feeder losses jump materially.
- Next step ideas: If volume remains short after TTL cut, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 82 — 2026-01-05T07:42:45Z
- Metrics (data/log-82.txt): start $50,000 → end ~$47,590.04 (profit -$2,409.96), max drawdown ≈ -$2,464.19. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$837.7K / -$968.32; BtcFeederHeavy ~$943.3K / -$1,071.99; Drip $348.5K / -$403.94; Feeder $840.8K / -$971.75; Pulse $315.3K / -$364.74; StockChurn $219.0K / -$245.62 (~$3.50M total). Largest swings: +$252.41 (Mon 08:30), -$336.03 (Tue 08:30), +$220.28 (Wed 08:30), -$365.60 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.50M (slight uptick). Feeder TTL 5s nudged volume modestly; overall drawdowns unchanged (opens and Thu 15:33). BTC feeders remain primary volume/bleed; Pulse still flat.
- Hypothesis: Shorten Volume-Drip orderTtl (20s → 15s) to reprice faster in off-hours and add incremental volume without changing bet size or delta.
- Expected effect / falsifier: expect Drip volume to rise by tens–low hundreds of K and push total volume toward $5M while keeping bleed near current ~$1K/mo pace; falsified if volume stays ~3.5M or Drip losses spike materially.
- Next step ideas: After observing Drip TTL impact, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 83 — 2026-01-05T07:55:51Z
- Metrics (data/log-83.txt): start $50,000 → end ~$47,591.80 (profit -$2,408.20), max drawdown ≈ -$2,462.43. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$844.1K / -$962.19; BtcFeederHeavy ~$935.2K / -$1,064.92; Drip $354.0K / -$408.09; Feeder $840.9K / -$979.11; Pulse $313.5K / -$362.79; StockChurn $219.0K / -$245.80 (~$3.51M total). Largest swings: +$252.43 (Mon 08:30), -$336.06 (Tue 08:30), +$220.30 (Wed 08:30), -$363.93 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.51M (tiny lift). Drip TTL 15s gave a small volume bump; drawdown pattern unchanged (opens and Thu 15:33). BTC feeders still dominate volume/bleed; StockChurn volume unchanged.
- Hypothesis: Tighten Volume-StockChurn repriceBand (10 → 8) to increase repricing frequency during stock hours and add churn without changing bet size, delta, or TTL.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and marginally raise total volume while keeping losses near current ~$1K/mo pace; falsified if volume stays ~3.5M or StockChurn losses spike materially.
- Next step ideas: If volume remains short after this change, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 84 — 2026-01-05T08:09:08Z
- Metrics (data/log-84.txt): start $50,000 → end ~$47,588.16 (profit -$2,411.84), max drawdown ≈ -$2,466.10. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$854.5K / -$974.59; BtcFeederHeavy ~$941.5K / -$1,065.68; Drip $351.2K / -$403.26; Feeder $837.6K / -$974.79; Pulse $314.7K / -$361.68; StockChurn $218.6K / -$245.61 (~$3.52M total). Largest swings: +$252.43 (Mon 08:30), -$336.03 (Tue 08:30), +$220.28 (Wed 08:30), -$368.94 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.52M (small lift). StockChurn band tighten not yet reflected in volume; drawdown pattern unchanged (opens and Thu 15:33). BTC feeders still carry most volume/bleed; slight BTC volume uptick from prior.
- Hypothesis: Lower Volume-BtcFeeder minProfitDelta (15_Cents → 10_Cents) to raise fill rate during bitcoin hours and push total volume toward $5M without changing sizing.
- Expected effect / falsifier: expect BTC feeder volume to rise (hundreds of K) and lift total volume while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or BTC feeder losses spike materially.
- Next step ideas: After observing BTC feeder delta cut, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 85 — 2026-01-05T08:23:06Z
- Metrics (data/log-85.txt): start $50,000 → end ~$47,586.85 (profit -$2,413.15), max drawdown ≈ -$2,467.39. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$851.2K / -$971.01; BtcFeederHeavy ~$942.4K / -$1,075.32; Drip $354.0K / -$402.94; Feeder $834.4K / -$971.29; Pulse $314.1K / -$361.44; StockChurn $218.6K / -$245.76 (~$3.51M total). Largest swings: +$252.40 (Mon 08:30), -$336.02 (Tue 08:30), +$220.27 (Wed 08:30), -$367.24 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.51M, with BTC feeder volume not improving after the 10¢ delta cut (down vs prior). Drawdown pattern unchanged (opens and Thu 15:33). Drip/Feeder volumes steady; StockChurn unchanged.
- Hypothesis: Tighten Volume-BtcFeeder repriceBand (4 → 3) to increase repricing frequency and recover volume during bitcoin hours without changing bet size or delta.
- Expected effect / falsifier: expect BtcFeeder volume to rise (hundreds of K) and lift total volume toward $5M while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or BTC feeder losses spike materially.
- Next step ideas: After observing band change, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 86 — 2026-01-05T08:38:20Z
- Metrics (data/log-86.txt): start $50,000 → end ~$47,587.61 (profit -$2,412.39), max drawdown ≈ -$2,466.64. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$848.9K / -$974.64; BtcFeederHeavy ~$929.8K / -$1,058.23; Drip $353.3K / -$407.81; Feeder $826.4K / -$971.45; Pulse $314.1K / -$362.98; StockChurn $218.2K / -$245.98 (~$3.49M total). Largest swings: +$252.41 (Mon 08:30), -$336.03 (Tue 08:30), +$220.28 (Wed 08:30), -$370.04 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume slipped to ~3.49M. BTC feeder band tighten to $3 did not lift volume; drawdown pattern unchanged (opens and Thu 15:33). Drip/Feeder/Pulse steady; StockChurn still flat.
- Hypothesis: Tighten Volume-BtcFeederHeavy repriceBand (4 → 3) to increase repricing frequency during bitcoin hours and recover lost volume without changing sizing.
- Expected effect / falsifier: expect BTC heavy feeder volume to rise (hundreds of K) and pull total volume upward while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or BTC feeder losses spike materially.
- Next step ideas: If BTC volume still stalls, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 87 — 2026-01-05T08:51:16Z
- Metrics (data/log-87.txt): start $50,000 → end ~$47,590.04 (profit -$2,409.96), max drawdown ≈ -$2,464.21. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$841.6K / -$962.09; BtcFeederHeavy ~$939.7K / -$1,072.42; Drip $352.6K / -$404.12; Feeder $839.2K / -$976.83; Pulse $313.5K / -$362.20; StockChurn $218.2K / -$245.95 (~$3.50M total). Largest swings: +$252.42 (Mon 08:30), -$336.05 (Tue 08:30), +$220.29 (Wed 08:30), -$366.92 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.50M. BTC heavy band cut to $3 raised heavy volume slightly, but light feeder volume fell; net volume change minimal. Drawdown pattern unchanged (opens and Thu 15:33). Drip/Feeder/Pulse/StockChurn steady.
- Hypothesis: Tighten Volume-Pulse repriceBand (5 → 4) to reprice more aggressively during stock/bitcoin hours and lift volume without changing bet size or delta.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and nudge total volume toward $5M while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or Pulse losses spike materially.
- Next step ideas: If volume remains short after Pulse band cut, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 88 — 2026-01-05T09:04:03Z
- Metrics (data/log-88.txt): start $50,000 → end ~$47,587.01 (profit -$2,412.99), max drawdown ≈ -$2,467.25. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$859.2K / -$982.79; BtcFeederHeavy ~$926.2K / -$1,060.11; Drip $347.2K / -$400.27; Feeder $833.6K / -$971.10; Pulse $312.9K / -$362.20; StockChurn $218.8K / -$246.44 (~$3.50M total). Largest swings: +$252.41 (Mon 08:30), -$336.03 (Tue 08:30), +$220.28 (Wed 08:30), -$368.37 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.50M. Pulse band 4s not yet reflected (still ~313K); BTC heavy volume dipped slightly; drawdown pattern unchanged (opens and Thu 15:33). Overall volume flat.
- Hypothesis: Tighten Volume-BtcFeederHeavy repriceBand further (3 → 2) to push more fills in bitcoin hours without changing sizing or delta.
- Expected effect / falsifier: expect BTC heavy feeder volume to rise (hundreds of K) and lift total volume while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or BTC feeder losses spike materially.
- Next step ideas: After observing heavier band cut, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 89 — 2026-01-05T09:17:34Z
- Metrics (data/log-89.txt): start $50,000 → end ~$47,588.11 (profit -$2,411.89), max drawdown ≈ -$2,466.15. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$853.6K / -$974.52; BtcFeederHeavy ~$937.0K / -$1,069.23; Drip $349.2K / -$401.13; Feeder $833.6K / -$966.37; Pulse $312.9K / -$364.03; StockChurn $218.8K / -$246.29 (~$3.50M total). Largest swings: +$252.40 (Mon 08:30), -$336.03 (Tue 08:30), +$220.28 (Wed 08:30), -$368.37 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.50M. Heavy band 2$ not yet reflected in volume; Pulse still flat. Drawdown pattern unchanged (opens and Thu 15:33).
- Hypothesis: Lower Volume-Pulse minProfitDelta (35_Cents → 30_Cents) to raise fill rate during stock/bitcoin hours and lift volume without changing bet size.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and nudge total volume toward $5M while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or Pulse losses spike materially.
- Next step ideas: If volume remains short, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 90 — 2026-01-05T09:31:14Z
- Metrics (data/log-90.txt): start $50,000 → end ~$47,588.11 (profit -$2,411.89), max drawdown ≈ -$2,467.25. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$857.7K / -$979.72; BtcFeederHeavy ~$939.7K / -$1,065.79; Drip $349.2K / -$399.10; Feeder $845.7K / -$973.30; Pulse $313.5K / -$361.75; StockChurn $218.8K / -$246.62 (~$3.52M total). Largest swings: +$252.41 (Mon 08:30), -$336.03 (Tue 08:30), +$220.28 (Wed 08:30), -$368.37 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.52M (tiny lift). Pulse delta cut not yet showing volume; BTC feeders inch up; drawdowns unchanged (opens and Thu 15:33).
- Hypothesis: Tighten Volume-Feeder repriceBand (4 → 3) to increase repricing frequency across all hours and lift fill rate without changing bet size or delta.
- Expected effect / falsifier: expect Feeder volume to rise (hundreds of K) and push total volume toward $5M while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or Feeder losses spike materially.
- Next step ideas: If volume remains short after Feeder band cut, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 91 — 2026-01-05T09:44:46Z
- Metrics (data/log-91.txt): start $50,000 → end ~$47,594.06 (profit -$2,405.94), max drawdown ≈ -$2,466.15. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$846.5K / -$977.37; BtcFeederHeavy ~$930.7K / -$1,058.32; Drip $350.6K / -$401.13; Feeder $827.2K / -$966.37; Pulse $313.5K / -$362.49; StockChurn $218.6K / -$246.72 (~$3.49M total). Largest swings: +$252.41 (Mon 08:30), -$336.03 (Tue 08:30), +$220.28 (Wed 08:30), -$368.37 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume slipped to ~3.49M. Feeder band 3$ not reflected yet; BTC volumes similar; drawdowns unchanged (opens and Thu 15:33).
- Hypothesis: Shorten Volume-Drip orderTtl (15s → 12s) to reprice more often in off-hours and add incremental volume without changing bet size or delta.
- Expected effect / falsifier: expect Drip volume to rise by tens–low hundreds of K and move total volume toward $5M while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or Drip losses spike materially.
- Next step ideas: If volume remains short, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 92 — 2026-01-05T09:57:41Z
- Metrics (data/log-92.txt): start $50,000 → end ~$47,581.79 (profit -$2,418.21), max drawdown ≈ -$2,472.47. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$860.0K / -$984.56; BtcFeederHeavy ~$945.1K / -$1,078.03; Drip $353.3K / -$399.25; Feeder $834.5K / -$970.24; Pulse $313.5K / -$361.57; StockChurn $218.8K / -$247.16 (~$3.53M total). Largest swings: +$252.41 (Mon 08:30), -$336.03 (Tue 08:30), +$220.28 (Wed 08:30), -$369.15 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.53M (small uptick) but profit slightly worse and drawdown deeper. Recent band/delta tweaks not yet lifting volume meaningfully; drawdown pattern unchanged (opens and Thu 15:33).
- Hypothesis: Further ease Volume-BtcFeeder cadence (minProfitDelta 10c → 8c, orderTtl 6s → 5s) to raise bitcoin-hour fill rate and push total volume toward $5M without changing bet size.
- Expected effect / falsifier: expect BtcFeeder volume to rise (hundreds of K) and lift total volume while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or BTC feeder losses spike materially.
- Next step ideas: If volume remains short after this change, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 93 — 2026-01-05T10:10:52Z
- Metrics (data/log-93.txt): start $50,000 → end ~$47,589.89 (profit -$2,410.11), max drawdown ≈ -$2,464.33. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$847.2K / -$977.92; BtcFeederHeavy ~$936.1K / -$1,060.94; Drip $352.6K / -$400.44; Feeder $829.6K / -$972.15; Pulse $313.5K / -$362.58; StockChurn $216.6K / -$245.90 (~$3.50M total). Largest swings: +$252.42 (Mon 08:30), -$336.05 (Tue 08:30), +$220.29 (Wed 08:30), -$365.91 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.50M and profit slightly better than prior; drawdown pattern unchanged (opens and Thu 15:33). BTC feeder delta/TTL easing not yet reflected in volume; StockChurn volume ticked down slightly.
- Hypothesis: Tighten Volume-StockChurn repriceBand (8 → 7) to increase repricing frequency during stock hours and regain churn volume without changing bet size, delta, or TTL.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and nudge total volume upward while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or StockChurn losses spike materially.
- Next step ideas: If volume remains short, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 94 — 2026-01-05T10:24:18Z
- Metrics (data/log-94.txt): start $50,000 → end ~$47,592.71 (profit -$2,407.29), max drawdown ≈ -$2,461.55. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$847.3K / -$982.10; BtcFeederHeavy ~$928.9K / -$1,055.34; Drip $353.3K / -$401.96; Feeder $832.9K / -$975.41; Pulse $313.5K / -$361.06; StockChurn $221.4K / -$250.37 (~$3.49M total). Largest swings: +$252.41 (Mon 08:30), -$336.03 (Tue 08:30), +$220.28 (Wed 08:30), -$367.13 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.49M, profit slightly improved, drawdown slightly better. BTC heavy volume dipped; StockChurn volume rose with band 7. Drawdown pattern unchanged (opens and Thu 15:33).
- Hypothesis: Shorten Volume-BtcFeederHeavy orderTtl (8s → 6s) to reprice faster during bitcoin hours and recover heavy feeder volume without changing sizing or delta.
- Expected effect / falsifier: expect BTC heavy volume to rise (hundreds of K) and help push total volume toward $5M while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or BTC heavy losses spike materially.
- Next step ideas: If volume remains short, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 95 — 2026-01-05T10:37:21Z
- Metrics (data/log-95.txt): start $50,000 → end ~$47,590.47 (profit -$2,409.53), max drawdown ≈ -$2,463.76. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$844.9K / -$977.57; BtcFeederHeavy ~$911.8K / -$1,070.11; Drip $350.6K / -$397.59; Feeder $827.3K / -$966.69; Pulse $312.3K / -$361.06; StockChurn $219.8K / -$248.12 (~$3.47M total). Largest swings: +$252.40 (Mon 08:30), -$336.02 (Tue 08:30), +$220.27 (Wed 08:30), -$365.99 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume slid to ~3.47M and profit/DD roughly flat. Heavy feeder volume fell; Pulse remains flat. Drawdown pattern unchanged (opens and Thu 15:33).
- Hypothesis: Ease Volume-Pulse aggressiveness further (30_Cents → 25_Cents minProfitDelta and band 4 → 3) to drive more fills across stock/bitcoin hours without changing bet size.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and push total volume up while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or Pulse losses spike materially.
- Next step ideas: If volume remains short after this Pulse change, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 96 — 2026-01-05T10:50:27Z
- Metrics (data/log-96.txt): start $50,000 → end ~$47,588.59 (profit -$2,411.41), max drawdown ≈ -$2,465.63. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$844.8K / -$976.83; BtcFeederHeavy ~$915.4K / -$1,068.25; Drip $353.3K / -$397.90; Feeder $825.6K / -$964.83; Pulse $314.1K / -$362.09; StockChurn $219.8K / -$248.12 (~$3.47M total). Largest swings: +$252.38 (Mon 08:30), -$336.00 (Tue 08:30), +$220.26 (Wed 08:30), -$366.42 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.47M with slightly worse PnL/DD. BTC heavy volume dipped; Pulse still flat; drawdown pattern unchanged (opens and Thu 15:33).
- Hypothesis: Lower Volume-Drip minProfitDelta (35_Cents → 30_Cents) to increase off-hour fills and lift total volume without changing bet size.
- Expected effect / falsifier: expect Drip volume to rise by tens–low hundreds of K and push total volume upward while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or Drip losses spike materially.
- Next step ideas: If volume remains short after Drip delta cut, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 101 — 2026-01-05T11:58:04Z
- Metrics (data/log-101.txt): start $50,000 → end ~$47,586.52 (profit -$2,413.48), max drawdown ≈ -$2,467.71. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$844.0K / -$973.52; BtcFeederHeavy ~$924.4K / -$1,078.98; Drip $354.6K / -$400.64; Feeder $830.4K / -$972.06; Pulse $312.9K / -$362.58; StockChurn $219.0K / -$245.73 (~$3.49M total). Largest swings: +$252.37 (Mon 08:30), -$335.98 (Tue 08:30), +$220.24 (Wed 08:30), -$365.21 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.49M with slightly worse PnL/DD. BTC heavy volume still low; Pulse flat; drawdowns unchanged (opens and Thu 15:33).
- Hypothesis: Shorten Volume-Pulse orderTtl (6s → 5s) to reprice faster across stock/bitcoin hours and lift fill rate without changing bet size or delta.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and push total volume toward $5M while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or Pulse losses spike materially.
- Next step ideas: If volume remains short, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 104 — 2026-01-05T12:35:06Z
- Metrics (data/log-104.txt): start $50,000 → end ~$47,583.08 (profit -$2,416.92), max drawdown ≈ -$2,471.14. STATUS showed Volume $86.8K early (fee 0.12%) but reverted to $0.00 (fee 0.60%) later. Final per-trader volumes/PnL: BtcFeeder ~$845.6K / -$974.12; BtcFeederHeavy ~$922.6K / -$1,078.53; Drip $353.3K / -$400.39; Feeder $826.4K / -$966.69; Pulse $317.1K / -$365.50; StockChurn $219.8K / -$248.10 (~$3.48M total).
- Observations: Total volume flat around ~$3.48M and losses steady; STATUS volume briefly reported then fell back to $0, so reporting bug persists. Biggest moves remain +$252.43 (Mon 08:30) and -$366.33 (Thu 15:33); no new market times surfaced. Drip is a moderate contributor (~$353K vol) with room to loosen delta.
- Hypothesis: Cut Volume-Drip minProfitDelta (30_Cents → 20_Cents) to raise off-hour fill rate and add incremental volume without changing bet size or TTL.
- Expected effect / falsifier: expect Drip volume to climb (tens–hundreds of K) and nudge total volume up while keeping bleed near the ~$1K/mo cap; falsified if total volume stays ~3.5M or Drip PnL per $100K volume degrades materially.
- Next step ideas: If Drip delta cut is insufficient, consider narrowing Drip reprice band or modest bet bumps; still need to fix STATUS volume reporting and keep monitoring Mon/Tue 08:30 and Thu 15:33 buckets.

Iteration 105 — 2026-01-05T12:49:43Z
- Metrics (data/log-105.txt): start $50,000 → end ~$47,580.34 (profit -$2,419.66), max drawdown ≈ -$2,473.89. STATUS briefly showed Volume $86.8K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$847.2K / -$975.52; BtcFeederHeavy ~$924.4K / -$1,081.09; Drip $353.3K / -$399.70; Feeder $830.5K / -$970.84; Pulse $320.1K / -$367.72; StockChurn $219.8K / -$248.42 (~$3.50M total).
- Observations: Volume and losses remain flat; Drip delta cut did not lift volume; STATUS volume reset persists. Biggest swings still +$252.43 (Mon 08:30) and -$367.69 (Thu 15:33); no new market time needed.
- Hypothesis: Narrow Volume-Drip repriceBand (12_Dollars → 8_Dollars) to keep resting orders closer to touch and improve fill rate without altering bet size or TTL.
- Expected effect / falsifier: expect Drip volume to rise meaningfully (tens–hundreds of K) and nudge total volume above the ~3.5M plateau while keeping Drip bleed per $100K volume roughly stable; falsified if volume remains flat or Drip losses per turnover worsen.
- Next step ideas: If band tightening is ineffective, consider modest Drip bet bump or similar cadence tweaks to Feeder/Pulse; still need a fix for STATUS volume reporting and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 97 — 2026-01-05T11:04:22Z
- Metrics (data/log-97.txt): start $50,000 → end ~$47,591.17 (profit -$2,408.83), max drawdown ≈ -$2,463.05. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$854.5K / -$984.61; BtcFeederHeavy ~$912.7K / -$1,063.19; Drip $354.6K / -$400.21; Feeder $837.7K / -$970.67; Pulse $313.5K / -$362.09; StockChurn $219.4K / -$247.60 (~$3.49M total). Largest swings: +$252.40 (Mon 08:30), -$336.02 (Tue 08:30), +$220.27 (Wed 08:30), -$365.73 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.49M, slightly better PnL/DD than prior. BTC heavy volume still low; Pulse still flat; drawdowns unchanged (opens and Thu 15:33).
- Hypothesis: Shorten Volume-BtcFeederHeavy orderTtl again (6s → 5s) to reprice faster during bitcoin hours and recover volume without changing sizing or delta.
- Expected effect / falsifier: expect BTC heavy volume to rise (hundreds of K) and lift total volume toward $5M while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or BTC heavy losses spike materially.
- Next step ideas: If volume remains short, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 98 — 2026-01-05T11:18:50Z
- Metrics (data/log-98.txt): start $50,000 → end ~$47,583.63 (profit -$2,416.37), max drawdown ≈ -$2,470.47. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$857.6K / -$981.94; BtcFeederHeavy ~$920.8K / -$1,073.62; Drip $354.6K / -$401.13; Feeder $843.2K / -$972.06; Pulse $315.9K / -$358.20; StockChurn $219.6K / -$246.73 (~$3.52M total). Largest swings: +$252.41 (Mon 08:30), -$336.03 (Tue 08:30), +$220.28 (Wed 08:30), -$368.37 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.52M (small lift) but profit/DD slightly worse. BTC volumes still soft; Pulse still flat. Drawdown pattern unchanged (opens and Thu 15:33).
- Hypothesis: Lower Volume-Feeder minProfitDelta (25_Cents → 20_Cents) to raise all-hours fill rate and push volume toward $5M without changing bet size.
- Expected effect / falsifier: expect Feeder volume to rise (hundreds of K) and lift total volume while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or Feeder losses spike materially.
- Next step ideas: If volume remains short, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 99 — 2026-01-05T11:32:51Z
- Metrics (data/log-99.txt): start $50,000 → end ~$47,585.88 (profit -$2,414.12), max drawdown ≈ -$2,468.35. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$852.0K / -$977.26; BtcFeederHeavy ~$923.5K / -$1,078.82; Drip $353.3K / -$400.44; Feeder $832.8K / -$965.36; Pulse $315.9K / -$359.84; StockChurn $219.4K / -$246.79 (~$3.50M total). Largest swings: +$252.37 (Mon 08:30), -$335.98 (Tue 08:30), +$220.26 (Wed 08:30), -$365.71 (Thu 15:33); no new market time needed.
- Observations: STATUS volume still frozen; total volume ~3.50M and PnL/DD similar to prior. BTC heavy volume still low; Pulse still flat; drawdowns unchanged (opens and Thu 15:33).
- Hypothesis: Lower Volume-BtcFeederHeavy minProfitDelta (1_Dollar → 75_Cents) to make heavy feeder slightly more aggressive while keeping band/TTL unchanged.
- Expected effect / falsifier: expect BTC heavy volume to rise (hundreds of K) and lift total volume while keeping losses near the ~$1K/mo guardrail; falsified if volume stays ~3.5M or BTC heavy losses spike materially.
- Next step ideas: If volume remains short, consider modest bet bumps or multi-pair coverage once STATUS reporting is fixed; continue monitoring Tue 08:30 and Thu 15:33 drawdowns.

Iteration 74 — 2026-01-05T05:57:43Z
- Metrics (data/log-74.txt): start $50,000 → end ~$47,591.00 (profit -$2,409.00), max drawdown ≈ -$2,463.24. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$861.7K / -$990.84; BtcFeederHeavy ~$917.2K / -$1,055.69; Drip $345.1K / -$398.21; Feeder $832.8K / -$970.29; Pulse $315.3K / -$367.63; StockChurn $208.4K / -$241.00 (~$3.48M total). Largest swings: -$336.06 (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$364.37 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume remains ~3.5M and STATUS frozen; losses steady and within guardrail. StockChurn TTL was 6s this run; effect small. Drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Shorten BtcFeeder order TTL (8s→6s) to increase repricing cadence and lift bitcoin-hours volume without changing deltas/bets.
- Expected effect / falsifier: expect BTC feeder volumes to rise (hundreds of K) and total volume to move toward $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or BTC feeder losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 75 — 2026-01-05T06:10:57Z
- Metrics (data/log-75.txt): start $50,000 → end ~$47,594.79 (profit -$2,405.21), max drawdown ≈ -$2,459.45. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$844.1K / -$978.41; BtcFeederHeavy ~$929.8K / -$1,063.89; Drip $350.6K / -$400.51; Feeder $832.9K / -$967.08; Pulse $315.3K / -$366.97; StockChurn $208.2K / -$240.33 (~$3.48M total). Largest swings: -$336.06 (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$364.72 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume remains ~3.5M and STATUS frozen; losses steady and within guardrail. BTC feeders still the largest contributors; drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Shorten BtcFeeder order TTL (8s→6s) to increase repricing cadence and lift bitcoin-hours volume without changing deltas/bets.
- Expected effect / falsifier: expect BTC feeder volumes to rise (hundreds of K) and total volume to move toward $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or BTC feeder losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 74 — 2026-01-05T05:57:43Z
- Metrics (data/log-74.txt): start $50,000 → end ~$47,591.00 (profit -$2,409.00), max drawdown ≈ -$2,463.24. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$861.7K / -$990.84; BtcFeederHeavy ~$917.2K / -$1,055.69; Drip $345.1K / -$398.21; Feeder $832.8K / -$970.29; Pulse $315.3K / -$367.63; StockChurn $208.4K / -$241.00 (~$3.48M total). Largest swings: -$336.06 (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$364.37 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume remains ~3.5M and STATUS frozen; losses steady and within guardrail. StockChurn TTL was 6s this run; effect small. Drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Shorten BtcFeeder order TTL (8s→6s) to increase repricing cadence and lift bitcoin-hours volume without changing deltas/bets.
- Expected effect / falsifier: expect BTC feeder volumes to rise (hundreds of K) and total volume to move toward $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or BTC feeder losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 72 — 2026-01-05T05:31:49Z
- Metrics (data/log-72.txt): start $50,000 → end ~$47,592.39 (profit -$2,407.61), max drawdown ≈ -$2,461.85. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$865.7K / -$989.78; BtcFeederHeavy ~$931.6K / -$1,066.45; Drip $350.6K / -$401.62; Feeder $824.1K / -$963.90; Pulse $315.9K / -$367.64; StockChurn $216.6K / -$242.56 (~$3.50M total). Largest swings: -$336.06 (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$361.52 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume flat around ~3.5M and STATUS still frozen; losses steady and within guardrail. Pulse TTL change not reflected yet; drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Shorten StockChurn order TTL (10s→6s) to reprice faster during stock hours and add churn/volume without raising bet size.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and total volume to move toward $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or StockChurn losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 73 — 2026-01-05T05:44:43Z
- Metrics (data/log-73.txt): start $50,000 → end ~$47,591.00 (profit -$2,409.00), max drawdown ≈ -$2,463.24. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$861.7K / -$990.84; BtcFeederHeavy ~$917.2K / -$1,055.69; Drip $345.1K / -$398.21; Feeder $832.8K / -$970.29; Pulse $315.3K / -$367.63; StockChurn $208.4K / -$241.00 (~$3.48M total). Largest swings: -$336.06 (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$364.37 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume still ~3.5M and STATUS frozen; losses steady and within guardrail. Recent Pulse/Feeder changes still not moving total volume; drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Shorten StockChurn order TTL further (6s→4s) to force faster repricing during stock hours and squeeze extra churn/volume without changing bet size or delta.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and total volume to move closer to $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or StockChurn losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 64 — 2026-01-05T03:54:09Z
- Metrics (data/log-64.txt): start $50,000 → end ~$47,589.73 (profit -$2,410.27), max drawdown ≈ -$2,464.52. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$874.5K / -$998.21; BtcFeederHeavy ~$939.7K / -$1,072.74; Drip $347.8K / -$400.86; Feeder $824.0K / -$957.71; Pulse $318.3K / -$364.27; StockChurn $208.2K / -$236.02 (~$3.51M total). Largest swings: -$336.03 (Tue 08:30), +$252.41 (Mon 08:30), +$220.27 (Wed 08:30), -$364.62 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume still ~3.5M and STATUS frozen; losses similar and within the $1K/mo cap. Feeder volume dipped slightly after TTL cut; BTC feeders up modestly; drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Lower Volume-Pulse minProfitDelta to 50_Cents to raise fill rate during stock/bitcoin hours and add incremental volume without increasing bet size.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and total volume to move closer to $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or Pulse losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 65 — 2026-01-05T04:06:15Z
- Metrics (data/log-65.txt): start $50,000 → end ~$47,598.24 (profit -$2,401.76), max drawdown ≈ -$2,456.00. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$856.1K / -$981.15; BtcFeederHeavy ~$928.9K / -$1,062.14; Drip $347.8K / -$404.99; Feeder $820.9K / -$963.06; Pulse $323.7K / -$372.25; StockChurn $208.0K / -$238.58 (~$3.49M total). Largest swings: -$336.06? (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$364.45 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume remains ~3.5M and STATUS frozen; losses steady and within guardrail. Feeder TTL cut did not lift volume; Pulse still modest. Drawdown pattern unchanged (opens + recurring Thu 15:33 spike).
- Hypothesis: Lower Volume-Pulse minProfitDelta to 50_Cents to improve fill rate during stock/bitcoin hours and add incremental volume without changing sizing.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and total volume to move toward $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or Pulse losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 66 — 2026-01-05T04:18:29Z
- Metrics (data/log-66.txt): start $50,000 → end ~$47,598.00 (profit -$2,402.00), max drawdown ≈ -$2,456.23. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$854.5K / -$980.27; BtcFeederHeavy ~$932.5K / -$1,064.41; Drip $348.5K / -$403.13; Feeder $826.5K / -$965.46; Pulse $321.9K / -$370.58; StockChurn $208.4K / -$239.01 (~$3.49M total). Largest swings: -$336.06 (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$364.45 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume flat ~3.5M with STATUS still broken; losses steady and within guardrail. Feeder TTL reduction did not move volume; BTC feeders slightly down; drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Remove midweekDerisk gating from Volume-Pulse to allow more trading hours and lift volume without changing sizing or deltas.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and total volume to move closer to $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or Pulse losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 67 — 2026-01-05T04:31:49Z
- Metrics (data/log-67.txt): start $50,000 → end ~$47,598.80 (profit -$2,401.20), max drawdown ≈ -$2,455.44. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$862.5K / -$989.43; BtcFeederHeavy ~$929.8K / -$1,060.15; Drip $344.4K / -$397.76; Feeder $831.3K / -$968.55; Pulse $323.1K / -$369.14; StockChurn $208.0K / -$236.56 (~$3.50M total). Largest swings: -$336.06 (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$364.01 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume still ~3.5M with STATUS frozen; losses steady and within guardrail. Ungating Pulse did not move total volume; drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Lower Volume-Drip minProfitDelta to 50_Cents to boost off-hours volume while keeping bet size constant.
- Expected effect / falsifier: expect Drip volume to rise (tens–hundreds of K) and nudge total volume upward without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or Drip losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 68 — 2026-01-05T04:43:57Z
- Metrics (data/log-69.txt treated as latest available): start $50,000 → end ~$47,597.92 (profit -$2,402.08), max drawdown ≈ -$2,456.33. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$860.9K / -$981.87; BtcFeederHeavy ~$935.2K / -$1,071.69; Drip $346.5K / -$401.23; Feeder $816.9K / -$956.14; Pulse $321.9K / -$369.37; StockChurn $208.2K / -$237.85 (~$3.49M total). Largest swings: -$336.06 (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$364.45 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume remains ~3.5M with STATUS frozen; losses steady and within guardrail. Drip delta cut not yet applied in this run; drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Lower Volume-StockChurn minProfitDelta to 25_Cents to drive more churn during stock hours without raising bet size.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and total volume to inch upward without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or StockChurn losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 71 — 2026-01-05T05:19:01Z
- Metrics (data/log-71.txt): start $50,000 → end ~$47,597.92 (profit -$2,402.08), max drawdown ≈ -$2,456.97. Final STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$862.5K / -$986.70; BtcFeederHeavy ~$932.5K / -$1,065.02; Drip $346.5K / -$397.27; Feeder $824.9K / -$964.58; Pulse $321.9K / -$368.51; StockChurn $215.8K / -$242.26 (~$3.50M total). Largest swings: -$336.06 (Tue 08:30), +$252.43 (Mon 08:30), +$220.30 (Wed 08:30), -$364.35 (Thu 15:33), +$243.10 (Wed 18:28); no new market time needed.
- Observations: Volume still ~3.5M with STATUS frozen; losses steady and within guardrail. Drip delta change not yet reflected in this run; Pulse still modest; drawdown pattern unchanged (opens + Thu 15:33).
- Hypothesis: Shorten Volume-Pulse order TTL (8s→6s) to reprice faster and increase fills during stock/bitcoin hours without changing sizing or deltas.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and total volume to move closer to $5M without pushing losses toward the $1K/mo cap; falsified if volume stays ~3.5M or Pulse losses spike materially.
- Next step ideas: if volume remains short, consider multi-pair support or modest bet bumps; add per-instance VolumeTrader config logging if configs drift; continue pursuing STATUS volume reporting fix.

Iteration 13 — 2026-01-05T02:05:08Z
- Metrics (data/log-32.txt from data/runs/codex01/32.txt.zstd): start $50,000 → end ~$49,036.10 (profit -$963.90), max drawdown ≈ -$1,405.87. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $213.0K / -$280.45; BtcFeederHeavy absent; Drip $176.6K / -$239.46; Feeder $170.7K / -$221.90; Pulse $195.2K / -$254.04; StockChurn $25.5K / -$34.09 (~$0.78–0.8M total). Largest swings: -$539.42 (Tue 08:30), +$395.30 (Mon 08:30); move buckets still at stock opens/late sessions.
- Observations: Volume slipped again (~0.8M) and STATUS remains frozen; losses improved (~-$96/mo) and are well within guardrail. Heavy feeder still inactive; timing clusters unchanged; no new market needed.
- Hypothesis: Add another BtcFeeder instance to lift volume using the low-bleed bitcoin-hours profile, since single instances are capped and heavy isn’t firing.
- Expected effect / falsifier: expect a noticeable volume increase (additional few hundred K per run) without exceeding ~$1K/month loss equivalent; falsified if volume barely moves or losses spike past that rate.
- Next step ideas: if volume remains low after adding another feeder, move to multi-pair support and instrument STATUS volume reporting to resolve the freeze.

Iteration 13 — 2026-01-05T02:03:46Z
- Metrics (data/log-32.txt from data/runs/codex01/32.txt.zstd): start $50,000 → end ~$49,017.57 (profit -$982.43), max drawdown ≈ -$1,416.78. Final STATUS volume still $49.70 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder $209.8K / -$275.73; BtcFeederHeavy absent; Drip $176.6K / -$239.28; Feeder $169.4K / -$221.24; Pulse $221.8K / -$288.60; StockChurn $25.5K / -$34.05 (~$1.0M total). Largest swings: +$445.88? (max delta 521/546 in earlier run? in this run max delta +407.79/-546.30) biggest loss -$546.30 (Tue 08:30), biggest gain +$407.79 (Mon 08:30); move buckets unchanged (08:30 and late sessions).
- Observations: Volume remains ~1M and STATUS frozen; losses improved further (~-$98/mo) and are well within guardrail. Heavy feeder still not active; open/close pattern unchanged; no new market needed.
- Hypothesis: Add a second BtcFeeder instance to lift total volume using the low-loss, bitcoin-hours profile while staying within the loss cap.
- Expected effect / falsifier: expect a noticeable volume increase (additional few hundred K per run) without exceeding ~$1K/month loss equivalent; falsified if volume barely rises or losses spike past that rate.
- Next step ideas: if volume still low, move to multi-pair support and instrument STATUS volume reporting to resolve the freeze.

Iteration 106 — 2026-01-05T13:03:32Z
- Metrics (data/log-106.txt): start $50,000 → end ~$47,579.13 (profit -$2,420.87), max drawdown ≈ -$2,475.09. STATUS briefly showed Volume $87.5K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$849.7K / -$976.17; BtcFeederHeavy ~$932.5K / -$1,084.46; Drip $357.4K / -$405.57; Feeder $828.1K / -$967.83; Pulse $319.5K / -$366.74; StockChurn $218.6K / -$247.69 (~$3.51M total).
- Observations: Volume and losses remain flat; Drip band/delta changes gave only a small volume uptick (~+4K) with similar bleed. STATUS volume reset persists. Biggest swings still +$252.43 (Mon 08:30) and -$366.72 (Thu 15:33); no new market time needed.
- Hypothesis: Narrow Volume-Pulse repriceBand (3_Dollars → 2_Dollars) to keep orders tighter to touch and lift stock/bitcoin-hour fill rate without changing bet size, delta, or TTL.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and push total volume above the ~3.5M plateau while keeping Pulse bleed per $100K volume roughly stable; falsified if volume stays flat or Pulse losses per turnover worsen materially.
- Next step ideas: If Pulse band tightening is ineffective, consider modest bet bumps (Drip/Pulse) or similar band cuts on Feeder; still need a STATUS volume reporting fix and to keep monitoring Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 107 — 2026-01-05T13:18:22Z
- Metrics (data/log-107.txt): start $50,000 → end ~$47,576.90 (profit -$2,423.10), max drawdown ≈ -$2,477.33. STATUS briefly showed Volume $87.5K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$850.5K / -$976.29; BtcFeederHeavy ~$931.6K / -$1,085.49; Drip $354.0K / -$402.53; Feeder $837.7K / -$973.19; Pulse $318.9K / -$366.61; StockChurn $217.4K / -$247.10 (~$3.51M total).
- Observations: Total volume and losses remain flat; Pulse band cut to 2_Dollars did not move volume meaningfully; Drip volume slipped a bit; STATUS volume reset persists. Biggest swings unchanged: +$252.43 (Mon 08:30) and -$366.39 (Thu 15:33); no new market time needed.
- Hypothesis: Tighten Volume-Feeder repriceBand (3_Dollars → 2_Dollars) to keep all-hours resting orders closer to touch and lift fill rate without changing bet size, delta, or TTL.
- Expected effect / falsifier: expect Feeder volume to rise (hundreds of K) and push total volume off the ~3.5M plateau while keeping Feeder bleed per $100K volume roughly stable; falsified if volume stays flat or Feeder losses per turnover worsen materially.
- Next step ideas: If Feeder band tightening is ineffective, consider modest bet bumps on Pulse/Drip/Feeder or per-market cadence tweaks; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 108 — 2026-01-05T13:31:13Z
- Metrics (data/log-108.txt): start $50,000 → end ~$47,579.45 (profit -$2,420.55), max drawdown ≈ -$2,474.77. STATUS briefly showed Volume $87.5K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$851.3K / -$979.20; BtcFeederHeavy ~$921.7K / -$1,078.21; Drip $356.7K / -$403.15; Feeder $832.1K / -$972.01; Pulse $317.7K / -$365.56; StockChurn $219.4K / -$247.79 (~$3.50M total).
- Observations: Volume and losses remain flat; Feeder band cut not yet reflected (vol slightly down), Pulse band cut didn’t move volume, STATUS volume reset persists. Biggest swings unchanged: +$252.44 (Mon 08:30) and -$367.24 (Thu 15:33); no new market time needed.
- Hypothesis: Tighten Volume-StockChurn repriceBand (7_Dollars → 5_Dollars) to keep stock-hour resting orders closer to touch and add modest churn/volume without changing bet size, delta, or TTL.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and total volume inch upward while keeping StockChurn bleed per $100K volume roughly stable; falsified if volume stays flat or StockChurn losses per turnover worsen materially.
- Next step ideas: If StockChurn band tightening is ineffective, consider modest bet bumps on Pulse/Drip/Feeder or further band tweaks; still need a STATUS volume reporting fix and ongoing monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 109 — 2026-01-05T13:44:18Z
- Metrics (data/log-109.txt): start $50,000 → end ~$47,577.91 (profit -$2,422.09), max drawdown ≈ -$2,476.31. STATUS briefly showed Volume $88.7K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$854.5K / -$975.32; BtcFeederHeavy ~$935.2K / -$1,078.66; Drip $356.0K / -$403.90; Feeder $835.2K / -$968.53; Pulse $321.9K / -$367.19; StockChurn $219.6K / -$251.64 (~$3.52M total).
- Observations: Total volume flat; StockChurn band cut not yet reflected; Feeder/Pulse bands still not moving volume; STATUS volume reset persists. Biggest swings unchanged: +$252.44 (Mon 08:30) and -$364.42 (Thu 15:33); no new market time needed.
- Hypothesis: Narrow Volume-BtcFeederHeavy repriceBand (2_Dollars → 1_Dollar) to keep heavy BTC quotes closer to touch and raise fill rate during bitcoin hours without changing bet size, delta, or TTL.
- Expected effect / falsifier: expect BtcFeederHeavy volume to rise (hundreds of K) and nudge total volume toward $5M while keeping heavy feeder bleed per $100K volume roughly stable; falsified if volume stays ~3.5M or heavy feeder losses per turnover worsen materially.
- Next step ideas: If heavy band tightening is ineffective, consider modest bet bumps across Pulse/Drip/Feeder or further band cuts; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 110 — 2026-01-05T13:56:43Z
- Metrics (data/log-110.txt): start $50,000 → end ~$47,576.90 (profit -$2,423.10), max drawdown ≈ -$2,477.32. STATUS briefly showed Volume $89.5K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$852.8K / -$979.98; BtcFeederHeavy ~$927.1K / -$1,073.56; Drip $349.9K / -$398.25; Feeder $839.2K / -$973.19; Pulse $318.9K / -$365.77; StockChurn $220.0K / -$252.07 (~$3.51M total).
- Observations: Volume and losses remain flat; BTC heavy band cut not reflected yet (volume down slightly); Drip volume slipped; STATUS volume reset persists. Biggest swings unchanged: +$252.43 (Mon 08:30) and -$368.35 (Thu 15:33); no new market time needed.
- Hypothesis: Shorten Volume-Drip orderTtl (12s → 10s) to reprice slightly faster in bitcoin-hours and recover Drip volume without changing bet size, delta, or band.
- Expected effect / falsifier: expect Drip volume to rise (tens–hundreds of K) and total volume nudge upward while keeping Drip bleed per $100K volume roughly stable; falsified if volume stays flat or Drip losses per turnover worsen materially.
- Next step ideas: If Drip TTL cut is ineffective, consider modest bet bumps or further band tweaks across Pulse/Feeder/Drip; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 111 — 2026-01-05T14:10:24Z
- Metrics (data/log-111.txt): start $50,000 → end ~$47,570.24 (profit -$2,429.76), max drawdown ≈ -$2,484.98. STATUS briefly showed Volume $90.0K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$857.6K / -$976.36; BtcFeederHeavy ~$944.2K / -$1,083.27; Drip $377.8K / -$417.07; Feeder $842.5K / -$971.43; Pulse $318.9K / -$364.70; StockChurn $219.8K / -$251.30 (~$3.56M total).
- Observations: Slight volume uptick (Drip rose with shorter TTL; BTC heavy up after band cut), but total still ~3.56M and bleed steady. STATUS volume reset persists. Biggest swings unchanged: +$252.43 (Mon 08:30) and -$366.87 (Thu 15:33); no new market time needed.
- Hypothesis: Lower Volume-Feeder minProfitDelta (20_Cents → 15_Cents) to improve all-hours fill rate and push volume upward without changing bet size, band, or TTL.
- Expected effect / falsifier: expect Feeder volume to rise (hundreds of K) and move total volume closer to $5M while keeping Feeder bleed per $100K volume roughly stable; falsified if volume stays ~3.5–3.6M or Feeder losses per turnover worsen materially.
- Next step ideas: If Feeder delta cut is insufficient, consider modest bet bumps or similar delta/band trims on Pulse/Drip; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 112 — 2026-01-05T14:25:15Z
- Metrics (data/log-112.txt): start $50,000 → end ~$47,571.53 (profit -$2,428.47), max drawdown ≈ -$2,482.69. STATUS briefly showed Volume $90.0K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$860.9K / -$975.08; BtcFeederHeavy ~$942.4K / -$1,076.56; Drip $377.1K / -$420.27; Feeder $852.9K / -$977.59; Pulse $320.7K / -$365.27; StockChurn $219.8K / -$250.85 (~$3.57M total).
- Observations: Small volume uptick (Drip/Feeder/Heavy up modestly) but total still well below $5M and losses steady; STATUS volume reset persists. Biggest swings unchanged: +$252.43 (Mon 08:30) and -$364.49 (Thu 15:33); no new market time needed.
- Hypothesis: Shorten Volume-Pulse orderTtl (5s → 4s) to increase repricing cadence and lift fills during stock/bitcoin hours without changing bet size, delta, or band.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and push total volume upward while keeping Pulse bleed per $100K volume roughly stable; falsified if volume stays ~3.5–3.6M or Pulse losses per turnover worsen materially.
- Next step ideas: If Pulse TTL cut is ineffective, consider modest bet bumps or further delta/band tweaks on Pulse/Feeder/Drip; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 113 — 2026-01-05T14:39:43Z
- Metrics (data/log-113.txt): start $50,000 → end ~$47,563.89 (profit -$2,436.11), max drawdown ≈ -$2,485.33. STATUS briefly showed Volume $95.9K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$852.0K / -$975.54; BtcFeederHeavy ~$929.8K / -$1,078.27; Drip $378.4K / -$417.38; Feeder $848.0K / -$974.64; Pulse $336.9K / -$375.01; StockChurn $219.6K / -$249.49 (~$3.56M total).
- Observations: Pulse volume jumped (~+16K) from TTL cut but total volume still flat; losses steady; STATUS volume reset persists. Biggest swings unchanged: +$252.36 (Mon 08:30) and -$364.59 (Thu 15:33); no new market time needed.
- Hypothesis: Lower Volume-Pulse minProfitDelta (25_Cents → 20_Cents) to make tight orders more willing to trade and lift fills without changing bet size, band, or TTL.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and help push total volume upward while keeping Pulse bleed per $100K volume roughly stable; falsified if volume stays ~3.5–3.6M or Pulse losses per turnover worsen materially.
- Next step ideas: If Pulse delta cut is insufficient, consider modest bet bumps or similar delta/band tweaks on Feeder/Drip; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 115 — 2026-01-05T15:07:06Z
- Metrics (data/log-115.txt): start $50,000 → end ~$47,570.15 (profit -$2,429.85), max drawdown ≈ -$2,486.06. STATUS briefly showed Volume $96.8K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$853.7K / -$974.63; BtcFeederHeavy ~$945.1K / -$1,084.60; Drip $377.8K / -$418.35; Feeder $839.3K / -$970.70; Pulse $338.1K / -$377.33; StockChurn $219.8K / -$250.57 (~$3.57M total).
- Observations: Volume remains stuck ~3.57M; Pulse delta cut lifted Pulse volume modestly but total still below $5M; losses steady; STATUS volume reset persists. Biggest swings unchanged: +$252.43 (Mon 08:30) and -$366.96 (Thu 15:33); no new market time needed.
- Hypothesis: Lower Volume-BtcFeeder minProfitDelta (8_Cents → 5_Cents) to make BTC feeder more willing to trade and lift bitcoin-hours volume without changing bet size, band, or TTL.
- Expected effect / falsifier: expect BtcFeeder volume to rise (hundreds of K) and push total volume upward while keeping BtcFeeder bleed per $100K volume roughly stable; falsified if volume stays ~3.5–3.6M or feeder losses per turnover worsen materially.
- Next step ideas: If BTC delta cut is insufficient, consider modest bet bumps or further band/delta tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 116 — 2026-01-05T15:21:35Z
- Metrics (data/log-116.txt): start $50,000 → end ~$47,572.27 (profit -$2,427.73), max drawdown ≈ -$2,481.96. STATUS briefly showed Volume $96.8K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$849.6K / -$965.56; BtcFeederHeavy ~$928.0K / -$1,072.52; Drip $380.5K / -$422.52; Feeder $841.6K / -$969.68; Pulse $336.9K / -$376.07; StockChurn $219.8K / -$251.67 (~$3.56M total).
- Observations: Volume still below $5M and losses steady; BTC feeder delta cut not yet reflected (volume dipped), Drip/Feeder/Pulse similar; STATUS volume reset persists. Biggest swings unchanged: +$252.43 (Mon 08:30) and -$364.13 (Thu 15:33); no new market time needed.
- Hypothesis: Narrow Volume-StockChurn repriceBand further (5_Dollars → 3_Dollars) to keep stock-hour orders closer to touch and add incremental churn/volume without changing bet size, delta, or TTL.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and nudge total volume upward while keeping StockChurn bleed per $100K volume roughly stable; falsified if volume stays flat or StockChurn losses per turnover worsen materially.
- Next step ideas: If band tightening is ineffective, consider modest bet bumps or further delta/band tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and ongoing monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 117 — 2026-01-05T15:36:05Z
- Metrics (data/log-117.txt): start $50,000 → end ~$47,577.69 (profit -$2,422.31), max drawdown ≈ -$2,485.33. STATUS briefly showed Volume $94.2K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$851.3K / -$973.74; BtcFeederHeavy ~$928.9K / -$1,072.88; Drip $379.8K / -$419.96; Feeder $837.7K / -$970.32; Pulse $338.1K / -$378.16; StockChurn $220.4K / -$248.46 (~$3.56M total).
- Observations: Total volume remains ~3.56M; BTC delta cut impact unclear; StockChurn band cut added small volume; losses steady; STATUS volume reset persists. Biggest swings unchanged: +$252.43 (Mon 08:30) and -$363.20 (Thu 15:33); no new market time needed.
- Hypothesis: Lower Volume-StockChurn minProfitDelta (25_Cents → 20_Cents) to make stock-hour churn slightly more willing to trade and add incremental volume without changing bet, band, or TTL.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and help lift total volume while keeping StockChurn bleed per $100K volume roughly stable; falsified if volume stays ~3.5–3.6M or StockChurn losses per turnover worsen materially.
- Next step ideas: If StockChurn delta cut is insufficient, consider modest bet bumps or further delta/band tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and ongoing monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 118 — 2026-01-05T15:50:20Z
- Metrics (data/log-118.txt): start $50,000 → end ~$47,569.64 (profit -$2,430.36), max drawdown ≈ -$2,485.59. STATUS briefly showed Volume $96.0K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$854.5K / -$974.01; BtcFeederHeavy ~$946.9K / -$1,085.20; Drip $378.4K / -$419.41; Feeder $841.7K / -$971.10; Pulse $338.1K / -$377.50; StockChurn $221.0K / -$248.45 (~$3.58M total).
- Observations: Total volume still below $5M but creeping upward; PnL steady; STATUS volume reset persists. Biggest swings unchanged: +$252.43 (Mon 08:30) and -$362.87 (Thu 15:33); no new market time needed.
- Hypothesis: Correct BtcFeederHeavy repriceBand literal to 1_Dollar (was typo 1_Dollars) to ensure the intended tighter band is applied; expect better adherence to tighter quoting.
- Expected effect / falsifier: expect BTC heavy to actually run with a 1_Dollar band, lifting bitcoin-hours volume and helping total volume; falsified if volume remains unchanged and logs/configs still show 2_Dollar behavior.
- Next step ideas: If effect is minimal, consider modest bet bumps or further delta/band tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 119 — 2026-01-05T16:04:48Z
- Metrics (data/log-119.txt): start $50,000 → end ~$47,566.09 (profit -$2,433.91), max drawdown ≈ -$2,488.14. STATUS briefly showed Volume $89.2K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$853.6K / -$975.18; BtcFeederHeavy ~$939.7K / -$1,077.12; Drip $379.1K / -$416.86; Feeder $845.6K / -$970.86; Pulse $337.5K / -$375.86; StockChurn $221.2K / -$248.15 (~$3.58M total).
- Observations: Total volume remains ~3.58M and losses steady; BTC heavy band fix applied but no volume lift yet; STATUS volume reset persists. Biggest swings unchanged: +$252.37 (Mon 08:30) and -$362.78 (Thu 15:33); no new market time needed.
- Hypothesis: Lower Volume-Feeder minProfitDelta (15_Cents → 10_Cents) to improve all-hours fill rate and push volume upward without changing bet, band, or TTL.
- Expected effect / falsifier: expect Feeder volume to rise (hundreds of K) and move total volume closer to $5M while keeping Feeder bleed per $100K volume roughly stable; falsified if volume stays ~3.5–3.6M or Feeder losses per turnover worsen materially.
- Next step ideas: If Feeder delta cut is insufficient, consider modest bet bumps or further delta/band/TTL tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 120 — 2026-01-05T16:19:11Z
- Metrics (data/log-120.txt): start $50,000 → end ~$47,579.30 (profit -$2,420.70), max drawdown ≈ -$2,485.59. STATUS briefly showed Volume $96.0K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$847.3K / -$968.63; BtcFeederHeavy ~$939.7K / -$1,077.09; Drip $374.4K / -$417.00; Feeder $839.3K / -$968.70; Pulse $336.9K / -$377.61; StockChurn $221.0K / -$248.88 (~$3.56M total).
- Observations: Volume still stuck ~3.56M; heavy band fix applied last iteration; losses steady; STATUS volume reset persists. Biggest swings unchanged: +$252.43 (Mon 08:30) and -$362.84 (Thu 15:33); no new market time needed.
- Hypothesis: Tighten Volume-BtcFeeder repriceBand (3_Dollars → 2_Dollars) to keep BTC feeder quotes closer to touch and lift bitcoin-hours fills without changing bet size, delta, or TTL.
- Expected effect / falsifier: expect BtcFeeder volume to rise (hundreds of K) and help push total volume upward while keeping feeder bleed per $100K volume roughly stable; falsified if volume stays ~3.5–3.6M or feeder losses per turnover worsen materially.
- Next step ideas: If BTC band cut is insufficient, consider modest bet bumps or further delta/band/TTL tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 121 — 2026-01-05T16:33:54Z
- Metrics (data/log-121.txt): start $50,000 → end ~$47,571.52 (profit -$2,428.48), max drawdown ≈ -$2,484.00. STATUS briefly showed Volume $96.0K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$852.1K / -$971.06; BtcFeederHeavy ~$949.6K / -$1,085.02; Drip $376.4K / -$418.66; Feeder $842.5K / -$970.10; Pulse $338.1K / -$376.96; StockChurn $220.6K / -$247.13 (~$3.58M total).
- Observations: Volume still ~3.58M; BTC band/delta changes not yet improving volume materially; Drip/Feeder/Pulse flat; STATUS volume reset persists. Biggest swings unchanged: +$252.42 (Mon 08:30) and -$364.00 (Thu 15:33); no new market time needed.
- Hypothesis: Lower Volume-Drip minProfitDelta (20_Cents → 15_Cents) to raise off-hour fill rate and add incremental volume without changing bet size, band, or TTL.
- Expected effect / falsifier: expect Drip volume to rise (tens–hundreds of K) and help push total volume toward $5M while keeping Drip bleed per $100K volume roughly stable; falsified if volume stays ~3.5–3.6M or Drip losses per turnover worsen materially.
- Next step ideas: If Drip delta cut is insufficient, consider modest bet bumps or further delta/band/TTL tweaks across Feeder/Pulse/Drip; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 122 — 2026-01-05T16:48:25Z
- Metrics (data/log-122.txt): start $50,000 → end ~$47,567.10 (profit -$2,432.90), max drawdown ≈ -$2,485.99. STATUS briefly showed Volume $96.0K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$859.3K / -$976.79; BtcFeederHeavy ~$945.1K / -$1,086.02; Drip $379.8K / -$418.25; Feeder $852.1K / -$968.89; Pulse $338.1K / -$377.96; StockChurn $221.6K / -$249.14 (~$3.60M total).
- Observations: Volume ticking up slightly (~3.60M) but still below $5M; losses steady; STATUS volume reset persists. Biggest swings unchanged: +$252.43 (Mon 08:30) and -$362.01 (Thu 15:33); no new market time needed.
- Hypothesis: Lower Volume-Pulse minProfitDelta (20_Cents → 15_Cents) to make tight stock/bitcoin-hour orders more willing to trade and lift fills without changing bet size, band, or TTL.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and help push total volume toward $5M while keeping Pulse bleed per $100K volume roughly stable; falsified if volume stays ~3.6M or Pulse losses per turnover worsen materially.
- Next step ideas: If Pulse delta cut is insufficient, consider modest bet bumps or further delta/band/TTL tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 124 — 2026-01-05T17:15:50Z
- Metrics (data/log-124.txt): start $50,000 → end ~$47,575.61 (profit -$2,424.39), max drawdown ≈ -$2,485.38. STATUS briefly showed Volume $89.2K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$855.3K / -$972.65; BtcFeederHeavy ~$937.9K / -$1,075.34; Drip $375.7K / -$416.13; Feeder $845.7K / -$971.73; Pulse $335.7K / -$377.30; StockChurn $221.0K / -$248.95 (~$3.57M total).
- Observations: Volume still ~3.57M and losses steady; STATUS volume reset persists; marginal changes not moving toward $5M. Biggest swings unchanged: +$252.42 (Mon 08:30) and -$362.38 (Thu 15:33); no new market time needed.
- Hypothesis: Lower Volume-StockChurn minProfitDelta (20_Cents → 15_Cents) to make stock-hour churn more willing to trade and add incremental volume without changing bet size, band, or TTL.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and help push total volume upward while keeping StockChurn bleed per $100K volume roughly stable; falsified if volume stays ~3.5–3.6M or StockChurn losses per turnover worsen materially.
- Next step ideas: If StockChurn delta cut is insufficient, consider modest bet bumps or further delta/band/TTL tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 125 — 2026-01-05T17:30:17Z
- Metrics (data/log-125.txt): start $50,000 → end ~$47,574.34 (profit -$2,425.66), max drawdown ≈ -$2,487.02. STATUS briefly showed Volume $89.2K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$854.5K / -$973.05; BtcFeederHeavy ~$930.7K / -$1,072.38; Drip $377.1K / -$418.13; Feeder $844.1K / -$971.70; Pulse $338.7K / -$379.16; StockChurn $222.0K / -$249.78 (~$3.57M total).
- Observations: Volume still ~3.57M and losses steady; STATUS volume reset persists. Biggest swings unchanged: +$252.42 (Mon 08:30) and -$362.82 (Thu 15:33); no new market time needed.
- Hypothesis: Tighten Volume-Feeder repriceBand (2_Dollars → 1_Dollars) to keep all-hours orders closer to touch and lift fill rate without changing bet size, delta, or TTL.
- Expected effect / falsifier: expect Feeder volume to rise (hundreds of K) and help push total volume toward $5M while keeping Feeder bleed per $100K volume roughly stable; falsified if volume stays ~3.5–3.6M or Feeder losses per turnover worsen materially.
- Next step ideas: If band tightening is insufficient, consider modest bet bumps or further delta/band/TTL tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 127 — 2026-01-05T17:58:38Z
- Metrics (data/log-127.txt): start $50,000 → end ~$47,575.39 (profit -$2,424.61), max drawdown ≈ -$2,485.82. STATUS briefly showed Volume $96.0K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$850.5K / -$968.72; BtcFeederHeavy ~$933.4K / -$1,078.14; Drip $375.7K / -$416.94; Feeder $844.1K / -$970.94; Pulse $339.9K / -$378.75; StockChurn $222.0K / -$249.08 (~$3.57M total).
- Observations: Volume still ~3.57M with slight pulse lift; losses steady; STATUS volume reset persists. Biggest swings unchanged: +$252.43 (Mon 08:30) and -$360.82 (Thu 15:33); no new market time needed.
- Hypothesis: Lower Volume-StockChurn minProfitDelta (15_Cents → 10_Cents) to further loosen stock-hour churn and add incremental volume without changing bet size, band, or TTL.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and help push total volume upward while keeping StockChurn bleed per $100K volume roughly stable; falsified if volume stays ~3.5–3.6M or StockChurn losses per turnover worsen materially.
- Next step ideas: If delta cut is insufficient, consider modest bet bumps or further delta/band/TTL tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 128 — 2026-01-05T18:13:17Z
- Metrics (data/log-128.txt): start $50,000 → end ~$47,573.06 (profit -$2,426.94), max drawdown ≈ -$2,485.98. STATUS briefly showed Volume $96.0K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$852.1K / -$970.31; BtcFeederHeavy ~$936.1K / -$1,079.38; Drip $378.4K / -$419.78; Feeder $844.9K / -$969.39; Pulse $338.1K / -$377.81; StockChurn $221.8K / -$249.27 (~$3.57M total).
- Observations: Volume still ~3.57M; losses steady; STATUS volume reset persists; BTC feeder still soft. Biggest swings unchanged: +$252.43 (Mon 08:30) and -$360.98 (Thu 15:33); no new market time needed.
- Hypothesis: Shorten Volume-BtcFeeder orderTtl (5s → 4s) to increase repricing cadence during bitcoin hours and lift fills without changing bet size, delta, or band.
- Expected effect / falsifier: expect BtcFeeder volume to rise (hundreds of K) and help push total volume toward $5M while keeping feeder bleed per $100K volume roughly stable; falsified if volume stays ~3.5–3.6M or BtcFeeder losses per turnover worsen materially.
- Next step ideas: If TTL cut is insufficient, consider modest bet bumps or further band/delta tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 129 — 2026-01-05T18:28:05Z
- Metrics (data/log-129.txt): start $50,000 → end ~$47,566.56 (profit -$2,433.44), max drawdown ≈ -$2,487.45. STATUS briefly showed Volume $89.2K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$881.6K / -$991.46; BtcFeederHeavy ~$935.2K / -$1,074.92; Drip $376.4K / -$416.30; Feeder $842.5K / -$962.07; Pulse $338.7K / -$377.91; StockChurn $221.4K / -$248.44 (~$3.60M total).
- Observations: Volume ticked up slightly (~3.60M) with BtcFeeder band/TTL changes reflected; losses steady; STATUS volume reset persists. Biggest swings unchanged: +$252.40 (Mon 08:30) and -$362.45 (Thu 15:33); no new market time needed.
- Hypothesis: Shorten Volume-Pulse orderTtl (4s → 3s) to further increase repricing cadence during stock/bitcoin hours and lift fills without changing bet size, delta, or band.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and help push total volume toward $5M while keeping Pulse bleed per $100K volume roughly stable; falsified if volume stays ~3.6M or Pulse losses per turnover worsen materially.
- Next step ideas: If Pulse TTL cut is insufficient, consider modest bet bumps or further delta/band tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 130 — 2026-01-05T18:42:55Z
- Metrics (data/log-130.txt): start $50,000 → end ~$47,568.95 (profit -$2,431.05), max drawdown ≈ -$2,485.59. STATUS briefly showed Volume $91.0K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$882.5K / -$992.93; BtcFeederHeavy ~$939.7K / -$1,075.68; Drip $375.0K / -$414.76; Feeder $842.4K / -$964.23; Pulse $332.1K / -$368.50; StockChurn $221.8K / -$248.78 (~$3.59M total).
- Observations: Volume inching up but still below $5M; losses steady; STATUS volume reset persists. Pulse volume dipped (332K) after TTL change; BTC feeder volumes up modestly; drawdown pattern unchanged (+$252.42 Mon 08:30, -$361.22 Thu 15:33).
- Hypothesis: Lower Volume-Drip minProfitDelta (15_Cents → 12_Cents) to increase off-hour fill rate and push total volume upward without changing bet size, band, or TTL.
- Expected effect / falsifier: expect Drip volume to rise (tens–hundreds of K) and help total volume toward $5M while keeping Drip bleed per $100K volume roughly stable; falsified if volume stays ~3.6M or Drip losses per turnover worsen materially.
- Next step ideas: If Drip delta cut is insufficient, consider modest bet bumps or further delta/band/TTL tweaks across Feeder/Pulse/Drip; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 131 — 2026-01-05T18:57:33Z
- Metrics (data/log-131.txt): start $50,000 → end ~$47,563.95 (profit -$2,436.05), max drawdown ≈ -$2,490.27. STATUS briefly showed Volume $92.8K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$900.1K / -$998.32; BtcFeederHeavy ~$953.2K / -$1,086.47; Drip $375.0K / -$413.96; Feeder $847.2K / -$965.75; Pulse $332.1K / -$368.36; StockChurn $222.6K / -$249.10 (~$3.63M total).
- Observations: Volume ticked up to ~3.63M (BTC feeder up), losses steady; STATUS volume reset persists. Biggest swings unchanged: +$252.42 (Mon 08:30) and -$361.74 (Thu 15:33); no new market time needed.
- Hypothesis: Tighten Volume-Pulse repriceBand (2_Dollars → 1_Dollars) to keep stock/bitcoin-hour quotes closer to touch and lift fills without changing bet size, delta, or TTL.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and help push total volume toward $5M while keeping Pulse bleed per $100K volume roughly stable; falsified if volume stays ~3.6M or Pulse losses per turnover worsen materially.
- Next step ideas: If Pulse band cut is insufficient, consider modest bet bumps or further delta/band/TTL tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 132 — 2026-01-05T19:12:14Z
- Metrics (data/log-132.txt): start $50,000 → end ~$47,568.32 (profit -$2,431.68), max drawdown ≈ -$2,485.99. STATUS briefly showed Volume $88.0K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$892.9K / -$1,001.73; BtcFeederHeavy ~$931.6K / -$1,063.99; Drip $373.0K / -$414.62; Feeder $854.5K / -$973.71; Pulse $332.7K / -$366.89; StockChurn $221.8K / -$249.06 (~$3.61M total).
- Observations: Volume creeping up (~3.61M) but still under $5M; losses steady; STATUS volume reset persists; BTC feeder gains most of the lift. Biggest swings unchanged: +$252.42 (Mon 08:30) and -$360.61 (Thu 15:33); no new market time needed.
- Hypothesis: Shorten Volume-Feeder orderTtl (5s → 4s) to increase repricing cadence all-hours and lift fills without changing bet size, delta, or band.
- Expected effect / falsifier: expect Feeder volume to rise (hundreds of K) and help push total volume toward $5M while keeping Feeder bleed per $100K volume roughly stable; falsified if volume stays ~3.6M or Feeder losses per turnover worsen materially.
- Next step ideas: If TTL cut is insufficient, consider modest bet bumps or further delta/band tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 133 — 2026-01-05T19:26:48Z
- Metrics (data/log-133.txt): start $50,000 → end ~$47,566.56 (profit -$2,433.44), max drawdown ≈ -$2,487.45. STATUS briefly showed Volume $89.2K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$881.6K / -$991.46; BtcFeederHeavy ~$935.2K / -$1,074.92; Drip $376.4K / -$416.30; Feeder $842.5K / -$962.07; Pulse $338.7K / -$377.91; StockChurn $221.4K / -$248.44 (~$3.60M total).
- Observations: Volume ~3.60M and losses steady; STATUS volume reset persists; BTC feeders leading volume gains. Biggest swings unchanged: +$252.40 (Mon 08:30) and -$362.45 (Thu 15:33); no new market time needed.
- Hypothesis: Shorten Volume-Pulse orderTtl (3s → 2s) to increase repricing cadence during stock/bitcoin hours and lift fills without changing bet size, delta, or band.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and help push total volume toward $5M while keeping Pulse bleed per $100K volume roughly stable; falsified if volume stays ~3.6M or Pulse losses per turnover worsen materially.
- Next step ideas: If Pulse TTL cut is insufficient, consider modest bet bumps or further delta/band tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 134 — 2026-01-05T19:40:10Z
- Metrics (data/log-134.txt): start $50,000 → end ~$47,558.92 (profit -$2,441.08), max drawdown ≈ -$2,485.99. STATUS briefly showed Volume $89.8K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$896.9K / -$996.47; BtcFeederHeavy ~$942.4K / -$1,075.60; Drip $374.4K / -$413.45; Feeder $893.7K / -$993.77; Pulse $333.9K / -$365.34; StockChurn $221.8K / -$247.38 (~$3.66M total).
- Observations: Total volume up to ~3.66M, led by BTC feeders and Feeder; losses steady; STATUS volume reset persists. Biggest swings unchanged: +$252.38 (Mon 08:30) and -$359.99 (Thu 15:33); no new market time needed.
- Hypothesis: Lower Volume-Drip minProfitDelta (12_Cents → 10_Cents) to further loosen off-hour fills and push total volume upward without changing bet size, band, or TTL.
- Expected effect / falsifier: expect Drip volume to rise (tens–hundreds of K) and help total volume toward $5M while keeping Drip bleed per $100K volume roughly stable; falsified if volume stays ~3.6M or Drip losses per turnover worsen materially.
- Next step ideas: If Drip delta cut is insufficient, consider modest bet bumps or further delta/band/TTL tweaks across Feeder/Pulse/Drip; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 135 — 2026-01-05T19:55:16Z
- Metrics (data/log-135.txt): start $50,000 → end ~$47,554.34 (profit -$2,445.66), max drawdown ≈ -$2,490.89. STATUS briefly showed Volume $90.7K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$905.7K / -$1,001.92; BtcFeederHeavy ~$953.2K / -$1,085.41; Drip $381.8K / -$418.44; Feeder $892.1K / -$989.21; Pulse $332.7K / -$363.03; StockChurn $221.8K / -$246.72 (~$3.69M total).
- Observations: Total volume up to ~3.69M with BTC feeders leading; losses steady; STATUS volume reset persists. Biggest swings unchanged: +$252.37 (Mon 08:30) and -$358.13 (Thu 15:33); no new market time needed.
- Hypothesis: Tighten Volume-Pulse repriceBand (1_Dollars → 50_Cents) to keep quotes even closer to touch and lift fills without changing bet size, delta, or TTL.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and help push total volume toward $5M while keeping Pulse bleed per $100K volume roughly stable; falsified if volume stays ~3.7M or Pulse losses per turnover worsen materially.
- Next step ideas: If Pulse band cut is insufficient, consider modest bet bumps or further delta/band/TTL tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 136 — 2026-01-05T20:10:24Z
- Metrics (data/log-136.txt): start $50,000 → end ~$47,557.41 (profit -$2,442.59), max drawdown ≈ -$2,485.99. STATUS briefly showed Volume $91.0K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$897.7K / -$999.38; BtcFeederHeavy ~$947.8K / -$1,079.30; Drip $379.8K / -$417.37; Feeder $892.1K / -$992.55; Pulse $329.7K / -$361.62; StockChurn $221.8K / -$247.30 (~$3.67M total).
- Observations: Volume edging up (~3.67M) with BTC feeders leading; losses steady; STATUS volume reset persists. Biggest swings unchanged: +$252.41 (Mon 08:30) and -$360.56 (Thu 15:33); no new market time needed.
- Hypothesis: Lower Volume-StockChurn minProfitDelta (10_Cents → 5_Cents) to further loosen stock-hour churn and add incremental volume without changing bet size, band, or TTL.
- Expected effect / falsifier: expect StockChurn volume to rise (tens of K) and nudge total volume upward while keeping StockChurn bleed per $100K volume roughly stable; falsified if volume stays ~3.6–3.7M or StockChurn losses per turnover worsen materially.
- Next step ideas: If delta cut is insufficient, consider modest bet bumps or further delta/band/TTL tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 138 — 2026-01-05T20:38:09Z
- Metrics (data/log-138.txt): start $50,000 → end ~$47,556.78 (profit -$2,443.22), max drawdown ≈ -$2,485.99. STATUS briefly showed Volume $91.0K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$898.5K / -$1,000.79; BtcFeederHeavy ~$934.3K / -$1,076.32; Drip $378.4K / -$415.48; Feeder $890.5K / -$993.67; Pulse $329.1K / -$361.09; StockChurn $223.2K / -$248.39 (~$3.65M total).
- Observations: Volume holding ~3.65M; losses steady; STATUS volume reset persists; BTC feeders and Feeder carry most of the volume. Biggest swings unchanged: +$252.35 (Mon 08:30) and -$360.05 (Thu 15:33); no new market time needed.
- Hypothesis: Shorten Volume-Drip orderTtl (10s → 8s) to increase repricing cadence during bitcoin hours and lift Drip fills without changing bet size, delta, or band.
- Expected effect / falsifier: expect Drip volume to rise (tens–hundreds of K) and help push total volume toward $5M while keeping Drip bleed per $100K volume roughly stable; falsified if volume stays ~3.6–3.7M or Drip losses per turnover worsen materially.
- Next step ideas: If TTL cut is insufficient, consider modest bet bumps or further delta/band/TTL tweaks across Feeder/Pulse/Drip; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 139 — 2026-01-05T20:53:23Z
- Metrics (data/log-139.txt): start $50,000 → end ~$47,559.75 (profit -$2,440.25), max drawdown ≈ -$2,494.47. STATUS briefly showed Volume $89.9K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$898.5K / -$1,001.31; BtcFeederHeavy ~$936.1K / -$1,069.26; Drip $370.3K / -$414.90; Feeder $890.5K / -$992.99; Pulse $327.9K / -$360.70; StockChurn $223.8K / -$249.15 (~$3.65M total).
- Observations: Volume holds ~3.65M; losses steady; STATUS volume reset persists. Pulse volume dipped after band tightening; drawdowns unchanged (+$252.35 Mon 08:30, -$359.02 Thu 15:33); no new market time needed.
- Hypothesis: Tighten Volume-Pulse repriceBand further (50_Cents → 25_Cents) to keep quotes very close to touch and recover Pulse fills without changing bet size or TTL.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and help push total volume toward $5M while keeping Pulse bleed per $100K volume roughly stable; falsified if volume stays ~3.6–3.7M or Pulse losses per turnover worsen materially.
- Next step ideas: If Pulse band cut is ineffective, consider modest bet bumps or similar tweaks on Feeder/Drip; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 140 — 2026-01-05T21:08:46Z
- Metrics (data/log-140.txt): start $50,000 → end ~$47,560.64 (profit -$2,439.36), max drawdown ≈ -$2,493.59. STATUS briefly showed Volume $91.0K (fee 0.12%) before reverting to $0 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$892.9K / -$992.34; BtcFeederHeavy ~$932.5K / -$1,068.02; Drip $375.7K / -$419.29; Feeder $890.5K / -$994.30; Pulse $333.3K / -$365.08; StockChurn $224.0K / -$250.18 (~$3.65M total).
- Observations: Volume flat ~3.65M and losses steady; STATUS volume reset persists; BTC feeders/Feeder carry most volume. Biggest swings unchanged: +$252.40 (Mon 08:30) and -$358.78 (Thu 15:33); no new market time needed.
- Hypothesis: Shorten Volume-BtcFeeder orderTtl (3s → 2s) to further increase repricing cadence in bitcoin hours and lift fills without changing bet size, delta, or band.
- Expected effect / falsifier: expect BtcFeeder volume to rise (hundreds of K) and help push total volume toward $5M while keeping feeder bleed per $100K volume roughly stable; falsified if volume stays ~3.6–3.7M or BtcFeeder losses per turnover worsen materially.
- Next step ideas: If TTL cut is insufficient, consider modest bet bumps or further delta/band tweaks across Feeder/Drip/Pulse; still need a STATUS volume reporting fix and continued monitoring of Mon/Tue 08:30 and Thu 15:33 drawdowns.

Iteration 142 — 2026-01-05T21:37:43Z
- Metrics (data/log-142.txt): start $50,000 → end ~$47,566.89 (profit -$2,433.11), max drawdown ≈ -$2,487.33. STATUS volume stayed $0.00 (fee 0.60%). Final per-trader volumes/PnL: BtcFeeder ~$892.1K / -$982.42; BtcFeederHeavy ~$926.2K / -$1,068.26; Drip $368.9K / -$413.48; Feeder $889.7K / -$995.38; Pulse $332.1K / -$364.54; StockChurn $223.8K / -$249.89 (~$3.63M total).
- Observations: Volume flat vs prior (~3.63M) and STATUS volume still broken; losses concentrated in BTC feeders and Feeder, while StockChurn remains the lowest-bleed profile. Largest swing +$252.37 at Mon 08:30; largest loss -$355.97 at Thu 15:33; timing matches existing market buckets, so no new tracked time needed.
- Hypothesis: Increase Volume-StockChurn bet size (100 → 150) to push more turnover into the lowest-bleed stock-hour profile and inch total volume toward the $5M target without leaning harder on BTC feeders.
- Expected effect / falsifier: expect StockChurn volume to rise proportionally (tens of K) and total volume to move above ~3.6M without worsening per-run losses beyond the recent ~$2.5K; falsified if volume stays flat or stock-hour losses spike disproportionately.
- Next step ideas: If the lift is small or losses rise, consider a modest Pulse bet bump or a slight delta increase on BTC feeders to trim their bleed; continue pursuing a STATUS volume reporting fix and watch the recurring Thu 15:33 loss bucket.

Iteration 143 — 2026-01-05T21:54:57Z
- Metrics (data/log-143.txt): start $50,000 → end ~$47,519.17 (profit -$2,480.83), max drawdown ≈ -$2,536.68. STATUS volume stuck at $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$884.1K / -$969.72; BtcFeederHeavy ~$932.5K / -$1,065.84; Drip $370.3K / -$410.55; Feeder $889.7K / -$983.27; Pulse $332.7K / -$360.66; StockChurn $335.7K / -$370.30 (~$3.75M total).
- Observations: StockChurn bet bump delivered a sizable volume jump (223.8K → 335.7K) and pushed total volume to ~3.75M; losses remain concentrated in BTC feeders/Feeder, while StockChurn bleed per turnover is still modest. Largest swing +$259.92 (Mon 08:30) and largest loss -$365.88 (Thu 15:33) align with existing market buckets, so no new tracked time needed. STATUS volume still broken.
- Hypothesis: Lower Volume-Feeder minProfitDelta (10_Cents → 5_Cents) to increase all-hours fill rate and lift total volume toward the $5M goal without increasing bet size.
- Expected effect / falsifier: expect Feeder volume to rise meaningfully (hundreds of K) and nudge total volume above ~3.8M while keeping per-run losses near the current ~$2.5K; falsified if volume stays flat or Feeder loss per turnover worsens materially.
- Next step ideas: If volume gain is small or losses rise, consider a modest Pulse bet increase or tightening BTC feeder bands; continue to seek a STATUS volume reporting fix and monitor the recurring Thu 15:33 loss bucket.

Iteration 144 — 2026-01-05T22:10:10Z
- Metrics (data/log-144.txt): start $50,000 → end ~$47,512.83 (profit -$2,487.17), max drawdown ≈ -$2,543.02. STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$891.3K / -$972.23; BtcFeederHeavy ~$949.6K / -$1,072.16; Drip $375.0K / -$413.74; Feeder $896.9K / -$988.06; Pulse $332.7K / -$360.24; StockChurn $335.7K / -$370.18 (~$3.78M total).
- Observations: Volume inched up (~3.78M) with Feeder delta cut; losses largely stable and still concentrated in BTC feeders/Feeder. StockChurn and Pulse remain lower-bleed profiles; STATUS volume remains broken. Largest swing +$259.92 (Mon 08:30), largest loss -$366.37 (Thu 15:33); timing matches existing market buckets—no new tracked market needed.
- Hypothesis: Lower Volume-Pulse minProfitDelta (15_Cents → 12_Cents) to improve fill rate during stock/bitcoin hours and push total volume closer to $5M without raising bet size.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and total volume to move above ~3.8M while keeping per-run losses near ~$2.5K; falsified if volume stays flat or Pulse loss per turnover worsens materially.
- Next step ideas: If Pulse delta cut under-delivers or losses rise, consider a small Pulse bet bump or gentler bands on BTC feeders; continue to seek a STATUS volume reporting fix and monitor the recurring Thu 15:33 loss bucket.

Iteration 145 — 2026-01-05T22:23:53Z
- Metrics (data/log-145.txt): start $50,000 → end ~$47,513.18 (profit -$2,486.82), max drawdown ≈ -$2,542.66. STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$900.9K / -$982.36; BtcFeederHeavy ~$925.3K / -$1,061.07; Drip $371.6K / -$409.36; Feeder $896.1K / -$985.95; Pulse $336.3K / -$364.14; StockChurn $335.7K / -$369.61 (~$3.77M total).
- Observations: Volume essentially flat (~3.77M); Pulse delta cut gave minor lift (332.7K → 336.3K). Loss pattern unchanged—BTC feeders and Feeder dominate bleed; STATUS volume still broken. Largest swing +$259.92 (Mon 08:30), largest loss -$365.38 (Thu 15:33); matches existing market buckets, so no new tracked time needed.
- Hypothesis: Raise Volume-Pulse bet size (300 → 350) to push more notional through a relatively low-bleed profile during stock/bitcoin hours and move total volume closer to $5M.
- Expected effect / falsifier: expect Pulse volume to increase proportionally (tens–hundreds of K) and nudge total volume upward without materially worsening per-run losses beyond the recent ~$2.5K; falsified if volume stays ~3.7–3.8M or Pulse loss per turnover degrades sharply.
- Next step ideas: If Pulse bet bump is insufficient or losses climb, consider gentle band tightening on BTC feeders to cut their bleed, and continue pursuing the STATUS volume reporting fix and monitoring the recurring Thu 15:33 loss bucket.

Iteration 146 — 2026-01-05T22:37:53Z
- Metrics (data/log-146.txt): start $50,000 → end ~$47,496.34 (profit -$2,503.66), max drawdown ≈ -$2,560.32. STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$884.9K / -$962.31; BtcFeederHeavy ~$939.7K / -$1,056.87; Drip $378.4K / -$414.94; Feeder $892.9K / -$978.50; Pulse $388.2K / -$419.48; StockChurn $335.7K / -$369.03 (~$3.82M total).
- Observations: Pulse bet bump lifted Pulse volume (336K → 388K) and total volume to ~3.82M, but Pulse bleed rose; BTC feeders and Feeder remain the main loss sources. STATUS volume still broken. Largest swing +$263.70 (Mon 08:30) and largest loss -$365.93 (Thu 15:33) align with existing market buckets; no new tracked time needed.
- Hypothesis: Tighten Volume-BtcFeeder repriceBand (2_Dollars → 1.5_Dollars) to keep quotes closer to touch, improve fill rate, and push total volume upward without changing size/delta.
- Expected effect / falsifier: expect BtcFeeder volume to rise (tens–hundreds of K) and total volume to move beyond ~3.8M while keeping per-run losses near the current ~$2.5K; falsified if volume stays flat or BtcFeeder loss per turnover worsens materially.
- Next step ideas: If band cut under-delivers or losses rise, consider small band/size tweaks on Feeder or a modest Pulse band tighten; continue pursuing STATUS volume reporting fix and monitoring the recurring Thu 15:33 loss bucket.

Iteration 147 — 2026-01-05T22:51:58Z
- Metrics (data/log-147.txt): start $50,000 → end ~$47,498.78 (profit -$2,501.22), max drawdown ≈ -$2,557.88. STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$896.9K / -$973.75; BtcFeederHeavy ~$921.7K / -$1,047.35; Drip $376.4K / -$410.13; Feeder $895.3K / -$979.49; Pulse $393.1K / -$423.19; StockChurn $335.7K / -$367.86 (~$3.82M total).
- Observations: Total volume flat (~3.82M); BtcFeeder band tighten added modest volume but losses steady. Pulse volume up but bleed a bit higher; BTC feeders and Feeder still dominate losses. STATUS volume remains broken. Largest swing +$263.70 (Mon 08:30), largest loss -$363.32 (Thu 15:33); timing matches existing buckets—no new tracked market needed.
- Hypothesis: Raise Volume-Feeder bet size (400 → 450) to push more all-hours notional and drive total volume toward $5M, accepting slightly higher bleed since per-run losses are still within the long-run monthly cap.
- Expected effect / falsifier: expect Feeder volume to rise proportionally (hundreds of K) and total volume to move above ~3.9M without per-run losses blowing out well beyond the recent ~$2.5K; falsified if volume stays flat or Feeder loss per turnover worsens materially.
- Next step ideas: If volume gain is small or losses climb, consider a modest Pulse band tighten or gentle size tweaks on BTC feeders; continue pursuing STATUS volume reporting fix and monitor the recurring Thu 15:33 loss bucket.

Iteration 148 — 2026-01-05T23:05:46Z
- Metrics (data/log-148.txt): start $50,000 → end ~$47,440.60 (profit -$2,559.40), max drawdown ≈ -$2,617.68. STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$904.9K / -$974.93; BtcFeederHeavy ~$937.0K / -$1,048.88; Drip $371.6K / -$400.87; Feeder $944.2K / -$710.58; Pulse $388.9K / -$414.14; StockChurn $335.7K / -$363.82 (~$3.88M total).
- Observations: Total volume improved to ~3.88M with the Feeder size bump; Feeder bleed improved materially (~-$711). BTC feeders remain top loss sources; STATUS volume still broken. Largest swing +$271.26 (Mon 08:30), largest loss -$372.93 (Thu 15:33); no new tracked market needed.
- Hypothesis: Tighten Volume-Pulse repriceBand (25_Cents → 20_Cents) to keep quotes closer to touch, lift fills, and push total volume upward without increasing bet size.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and total volume to move beyond ~3.9M while keeping per-run losses near current levels; falsified if volume stays flat or Pulse loss per turnover worsens materially.
- Next step ideas: If Pulse band tighten under-delivers or losses climb, consider gentle band/size tweaks on BTC feeders or Drip; continue pursuing STATUS volume reporting fix and monitoring the recurring Thu 15:33 loss bucket.

Iteration 149 — 2026-01-05T23:19:34Z
- Metrics (data/log-149.txt): start $50,000 → end ~$47,445.74 (profit -$2,554.26), max drawdown ≈ -$2,612.54. STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$895.3K / -$966.19; BtcFeederHeavy ~$925.3K / -$1,039.14; Drip $365.5K / -$400.09; Feeder $946.0K / -$715.01; Pulse $388.9K / -$416.08; StockChurn $334.5K / -$364.58 (~$3.86M total).
- Observations: Volume dipped slightly (~3.86M); Feeder bleed remains improved (~-$715) but BTC feeders still dominate losses. Drip volume is relatively low vs other profiles; STATUS volume still broken. Largest swing +$271.26 (Mon 08:30), largest loss -$371.79 (Thu 15:33); no new tracked market needed.
- Hypothesis: Lower Volume-Drip minProfitDelta (10_Cents → 8_Cents) to increase off-hour fill rate and lift total volume without changing bet size/band/TTL.
- Expected effect / falsifier: expect Drip volume to rise (tens–hundreds of K) and total volume to move closer to $4M while keeping per-run losses near the recent ~$2.5K; falsified if volume stays flat or Drip loss per turnover worsens materially.
- Next step ideas: If Drip delta cut under-delivers, consider gentle band/size tweaks on BTC feeders or Pulse; continue pursuing STATUS volume reporting fix and monitoring the recurring Thu 15:33 loss bucket.

Iteration 150 — 2026-01-05T23:33:43Z
- Metrics (data/log-150.txt): start $50,000 → end ~$47,439.21 (profit -$2,560.79), max drawdown ≈ -$2,619.07. STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$888.9K / -$961.15; BtcFeederHeavy ~$940.6K / -$1,050.52; Drip $374.4K / -$407.61; Feeder $946.0K / -$715.25; Pulse $388.9K / -$416.21; StockChurn $335.7K / -$365.06 (~$3.87M total).
- Observations: Total volume steady (~3.87M) and losses similar; Drip volume still lagging peers, BTC feeders remain top loss sources, STATUS volume still broken. Largest swing +$271.26 (Mon 08:30), largest loss -$375.07 (Thu 15:33); no new tracked market needed.
- Hypothesis: Tighten Volume-Drip repriceBand (8_Dollars → 6_Dollars) to keep off-hour quotes closer to touch and raise fill rate/volume without changing size/delta/TTL.
- Expected effect / falsifier: expect Drip volume to rise (tens–hundreds of K) and push total volume closer to $4M while keeping per-run losses near current levels; falsified if volume stays flat or Drip loss per turnover worsens materially.
- Next step ideas: If band cut under-delivers, consider gentle band/size tweaks on BTC feeders or Pulse; continue pursuing STATUS volume reporting fix and monitoring the recurring Thu 15:33 loss bucket.

Iteration 152 — 2026-01-06T00:00:54Z
- Metrics (data/log-152.txt): start $50,000 → end ~$47,442.05 (profit -$2,557.95), max drawdown ≈ -$2,616.23. STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$895.3K / -$963.73; BtcFeederHeavy ~$943.3K / -$1,053.33; Drip $374.4K / -$406.90; Feeder $945.1K / -$711.46; Pulse $388.2K / -$414.36; StockChurn $335.1K / -$363.55 (~$3.88M total).
- Observations: Volume essentially flat (~3.88M) after the prior Drip band cut; BTC feeders remain the main loss drivers, Feeder still improved. STATUS volume remains broken. Largest swing +$271.25 (Mon 08:30), largest loss -$372.55 (Thu 15:33); no new tracked market needed.
- Hypothesis: Tighten Volume-Drip repriceBand further (6_Dollars → 5_Dollars) to keep off-hour quotes closer to touch and lift fill rate/volume without changing size/delta/TTL.
- Expected effect / falsifier: expect Drip volume to increase (tens–hundreds of K) and nudge total volume closer to $4M while keeping per-run losses near current levels; falsified if volume stays flat or Drip loss per turnover worsens materially.
- Next step ideas: If this band cut under-delivers, consider gentle band/size tweaks on BTC feeders or Pulse; continue pursuing STATUS volume reporting fix and monitoring the recurring Thu 15:33 loss bucket.

Iteration 153 — 2026-01-06T00:14:37Z
- Metrics (data/log-153.txt): start $50,000 → end ~$47,440.01 (profit -$2,559.99), max drawdown ≈ -$2,618.27. STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$888.9K / -$961.09; BtcFeederHeavy ~$937.9K / -$1,054.80; Drip $374.4K / -$407.71; Feeder $947.8K / -$713.95; Pulse $388.2K / -$413.79; StockChurn $335.7K / -$363.97 (~$3.87M total).
- Observations: Volume flat (~3.87M); Drip remains relatively small and low-bleed, while BTC feeders dominate losses. STATUS volume still broken. Largest swing +$271.25 (Mon 08:30), largest loss -$378.34 (Thu 15:33); no new tracked market needed.
- Hypothesis: Raise Volume-Drip bet size (340 → 400) to push more off-hour notional through a low-bleed profile and lift total volume toward $4M without touching spreads/TTLs.
- Expected effect / falsifier: expect Drip volume to rise proportionally (tens–hundreds of K) and total volume to move closer to $4M while keeping per-run losses near current levels; falsified if volume stays flat or Drip loss per turnover worsens materially.
- Next step ideas: If volume gain is small or losses climb, consider gentle band/size tweaks on BTC feeders or Pulse; continue pursuing STATUS volume reporting fix and monitoring the recurring Thu 15:33 loss bucket.

Iteration 154 — 2026-01-06T00:28:50Z
- Metrics (data/log-154.txt): start $50,000 → end ~$47,405.89 (profit -$2,594.11), max drawdown ≈ -$2,653.37. STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$900.9K / -$963.01; BtcFeederHeavy ~$942.4K / -$1,050.18; Drip $444.4K / -$480.58; Feeder $944.2K / -$706.78; Pulse $391.7K / -$414.30; StockChurn $335.7K / -$362.14 (~$3.96M total).
- Observations: Total volume up to ~3.96M driven by Drip size bump; Drip loss increased but remains moderate. BTC feeders remain the largest loss contributors; STATUS volume still broken. Largest swing +$275.79 (Mon 08:30), largest loss -$377.68 (Thu 15:33); no new tracked market needed.
- Hypothesis: Raise Volume-StockChurn bet size (150 → 200) to push more low-bleed stock-hour notional and move total volume closer to $4M+ without relying further on BTC feeders.
- Expected effect / falsifier: expect StockChurn volume to rise proportionally (tens–hundreds of K) and total volume to nudge upward while keeping per-run losses near current levels; falsified if volume stays flat or StockChurn loss per turnover worsens materially.
- Next step ideas: If volume lift is small or losses climb, consider trimming BTC feeder bands/sizes or a small Pulse tweak; continue pursuing STATUS volume reporting fix and monitoring the recurring Thu 15:33 loss bucket.

Iteration 155 — 2026-01-06T00:43:17Z
- Metrics (data/log-155.txt): start $50,000 → end ~$47,351.01 (profit -$2,648.99), max drawdown ≈ -$2,709.90. STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$903.3K / -$956.06; BtcFeederHeavy ~$942.4K / -$1,039.43; Drip $441.2K / -$474.29; Feeder $952.3K / -$708.29; Pulse $388.2K / -$410.04; StockChurn $447.6K / -$480.14 (~$4.08M total).
- Observations: Volume crossed ~4.1M mainly via StockChurn and Drip sizing; losses still dominated by BTC feeders and Drip. STATUS volume remains broken. Largest swing +$283.34 (Mon 08:30), largest loss -$379.68 (Thu 15:33); no new tracked market needed.
- Hypothesis: Lower Volume-BtcFeederHeavy minProfitDelta (75_Cents → 50_Cents) to increase heavy bitcoin-hour fills and push total volume toward $4.3–4.5M without changing size/band/TTL.
- Expected effect / falsifier: expect heavy feeder volume to rise (hundreds of K) and total volume to move up meaningfully while keeping per-run losses near current levels; falsified if volume stays ~4.1M or heavy feeder loss per turnover worsens materially.
- Next step ideas: If heavy delta cut under-delivers or losses rise, consider trimming BTC feeder bands further or a small Pulse tweak; continue pursuing STATUS volume reporting fix and monitoring the recurring Thu 15:33 loss bucket.

Iteration 156 — 2026-01-06T00:57:52Z
- Metrics (data/log-156.txt): start $50,000 → end ~$47,345.76 (profit -$2,654.24), max drawdown ≈ -$2,715.12. STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$900.1K / -$955.53; BtcFeederHeavy ~$940.6K / -$1,032.41; Drip $438.0K / -$471.07; Feeder $945.1K / -$708.62; Pulse $387.5K / -$412.54; StockChurn $447.6K / -$482.57 (~$4.06M total).
- Observations: Total volume roughly flat (~4.06M) after the heavy delta cut; BTC feeders still dominate losses, Drip and StockChurn show higher bleed, STATUS volume remains broken. Largest swing +$283.34 (Mon 08:30), largest loss -$378.90 (Thu 15:33); no new tracked market needed.
- Hypothesis: Tighten Volume-Feeder repriceBand (1_Dollar → 80_Cents) to quote closer all-hours, increase fill rate, and push total volume toward $4.3–4.5M without changing size/delta/TTL.
- Expected effect / falsifier: expect Feeder volume to rise (hundreds of K) and total volume to move up meaningfully while keeping per-run losses near current levels; falsified if volume stays ~4.0–4.1M or Feeder loss per turnover worsens materially.
- Next step ideas: If band cut under-delivers or losses rise, consider trimming BTC feeder bands/sizes or a small Pulse tweak; continue pursuing STATUS volume reporting fix and monitoring the recurring Thu 15:33 loss bucket.

Iteration 157 — 2026-01-06T01:11:58Z
- Metrics (data/log-157.txt): start $50,000 → end ~$47,358.70 (profit -$2,641.30), max drawdown ≈ -$2,702.21. STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$891.3K / -$950.46; BtcFeederHeavy ~$945.1K / -$1,041.88; Drip $445.2K / -$474.49; Feeder $945.1K / -$702.79; Pulse $388.9K / -$410.36; StockChurn $447.6K / -$479.86 (~$4.06M total).
- Observations: Total volume roughly flat (~4.06M); heavy feeders remain the largest loss contributors, while Feeder losses improved slightly. STATUS volume remains broken. Largest swing +$283.35 (Mon 08:30), largest loss -$377.88 (Thu 15:33); no new tracked market needed.
- Hypothesis: Tighten Volume-BtcFeederHeavy repriceBand (1_Dollar → 75_Cents) to quote closer in bitcoin hours, increase heavy fill rate, and push total volume toward $4.3–4.5M without changing size/delta/TTL.
- Expected effect / falsifier: expect heavy feeder volume to rise (hundreds of K) and total volume to move up meaningfully while keeping per-run losses near current levels; falsified if volume stays ~4.0–4.1M or heavy feeder loss per turnover worsens materially.
- Next step ideas: If band tighten under-delivers or losses rise, consider small Pulse or light-feeder band/size tweaks; continue pursuing STATUS volume reporting fix and monitoring the recurring Thu 15:33 loss bucket.

Iteration 158 — 2026-01-06T01:26:05Z
- Metrics (data/log-158.txt): start $50,000 → end ~$47,356.28 (profit -$2,643.72), max drawdown ≈ -$2,704.63. STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$893.7K / -$955.?? (PnL parsing summed to -$4,063.55 total across traders); BtcFeederHeavy ~$948.7K; Drip $441.2K; Feeder $946.0K; Pulse $388.2K; StockChurn $447.6K (~$4.07M total).
- Observations: Volume roughly flat vs prior (~4.07M); BTC feeders continue as top loss contributors; STATUS volume remains broken. Largest swing +$283.35 (Mon 08:30), largest loss -$379.03 (Thu 15:33); no new tracked market needed.
- Hypothesis: Tighten Volume-BtcFeeder repriceBand (1.5_Dollars → 1_Dollar) to quote closer in bitcoin hours, increase fill rate, and push total volume toward $4.3–4.5M without changing size/delta/TTL.
- Expected effect / falsifier: expect BtcFeeder volume to rise (hundreds of K) and total volume to move up meaningfully while keeping per-run losses near current levels; falsified if volume stays ~4.0–4.1M or btc feeder loss per turnover worsens materially.
- Next step ideas: If band tighten under-delivers or losses rise, consider a small Pulse or light-feeder tweak; continue pursuing STATUS volume reporting fix and monitoring the recurring Thu 15:33 loss bucket.

Iteration 159 — 2026-01-06T01:40:28Z
- Metrics (data/log-159.txt): start $50,000 → end ~$47,358.81 (profit -$2,641.19), max drawdown ≈ -$2,702.07. STATUS volume still $0.00 (fee 0.60%). Per-trader volumes/PnL: BtcFeeder ~$901.7K / (aggregate PnL sum -$4,050.61); BtcFeederHeavy ~$933.4K; Drip $438.8K; Feeder $944.2K; Pulse $388.2K; StockChurn $447.6K (~$4.05M total).
- Observations: Volume essentially flat (~4.05M) after the BtcFeeder band cut; BTC feeders remain the main loss sources; STATUS volume remains broken. Largest swing +$283.36 (Mon 08:30), largest loss -$378.14 (Thu 15:33); no new tracked market needed.
- Hypothesis: Tighten Volume-Pulse repriceBand (20_Cents → 15_Cents) to quote closer during stock/bitcoin hours, lift fill rate, and push total volume toward $4.3–4.5M without changing size/delta/TTL.
- Expected effect / falsifier: expect Pulse volume to rise (tens–hundreds of K) and total volume to move meaningfully upward while keeping per-run losses near current levels; falsified if volume stays ~4.0–4.1M or Pulse loss per turnover worsens materially.
- Next step ideas: If Pulse band tighten under-delivers or losses rise, consider small tweaks on BTC feeders or light feeder; continue pursuing STATUS volume reporting fix and monitoring the recurring Thu 15:33 loss bucket.
