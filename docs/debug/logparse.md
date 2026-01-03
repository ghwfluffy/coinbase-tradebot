# Log Parsing Playbook

Guidance for Codex to extract signal from large `data/log.txt`–style runs using standard CLI tools (`rg`, `awk`, `python`, `bash`).

## General Approach
- Prefer `rg` for filtering; it’s fast on huge files.
- Work in passes: 1) slice relevant lines, 2) reduce/aggregate, 3) summarize.
- Beware time zones: logs use mock time (see `[YYYY-MM-DD HH:MM:SS (Day)]` prefix). No need to convert.
- When possible, reuse existing markers: `STATUS`, per-trader PnL lines, `ERROR`/`WARN`, and `trade`/`debug` levels.

## Quick Slices
- Errors/warnings/suspicious: `rg "ERROR|WARN|Not enough|Overflow|Can't cancel|slippage" data/log.txt`
- Status timeline: `rg "STATUS" data/log.txt`
- Trade-level noise: `rg "\\[ TRADE \\]" data/log.txt` (if enabled)
- Per-trader PnL lines: `rg "PnL" data/log.txt`
- Trader configs at startup (helps tie behavior to params/gating): `rg "\\[ CONFIG \\] VolumeTrader" data/log.txt`

## Profit and Volume
- Total profit over time (from STATUS):
  `rg "STATUS" data/log.txt | awk '{print $1, $2, $(NF-4), $(NF-2)}'`
  (Adjust field indices if STATUS format changes; typically profit and volume are near the end.)
- Per-trader profit snapshots (if logged):
  `rg "PnL" data/log.txt | awk '{print $1, $2, $3, $(NF)}'`
- Final profit/volume: take last STATUS line.
  `rg "STATUS" data/log.txt | tail -n1`
- Win/loss episodes: compute deltas between successive STATUS profits. Large negative deltas flag drawdowns.
  Example Python sketch:
  ```python
  import re, sys
  prev = None
  for line in open(sys.argv[1]):
      if "STATUS" not in line: continue
      m = re.search(r'Profit: \\$([-\\d.,KMR]+)', line)
      if not m: continue
      cur = float(m.group(1).replace(',', '').replace('K','000').replace('M','000000'))
      if prev is not None:
          delta = cur - prev
          if abs(delta) > 100:  # adjust threshold
              print(line.strip(), "delta", delta)
      prev = cur
  ```

## Time-of-Day/Week Impact
- Extract hour-of-day buckets for large moves (profit deltas as above).
  `... | awk '{split($2,t,":"); print t[1]}' | sort | uniq -c`
- Day-of-week: parse `(Mon)` token.
  `rg "STATUS" ... | awk '{print $(3)}' | tr -d '()' | sort | uniq -c`
- For “big loss” windows, filter deltas below threshold and group by hour/day to spot patterns.

## Trade Counts, Win Rate, PnL Stats
- If trade-level logs are enabled (`[ TRADE ]`), count buys/sells:
  `rg "\\[ TRADE \\].*Filled buy" ... | wc -l` and similarly for sells.
- If not, approximate trade count from order lifecycle logs (`submit order`, `filled`, etc.).
- Win rate/avg PnL: requires fills with amounts; if fills are logged, parse `beforeFees/fees` and compute net. Sketch:
  ```python
  import re, sys
  wins = losses = 0; pnl = []
  for line in open(sys.argv[1]):
      m = re.search(r'Filled .* beforeFees=\\$([\\d.]+) fees=\\$([\\d.]+)', line)
      if not m: continue
      val = float(m.group(1)) - float(m.group(2))
      pnl.append(val)
      (wins if val>0 else losses).__iadd__(1)
  print("trades", len(pnl), "win%", wins/max(1,(wins+losses))*100, "avg", sum(pnl)/max(1,len(pnl)))
  ```
- Largest loss: track `min(pnl)` from the same list.
- Fee/slippage: compare `beforeFees` vs execution price; if only fee percent is known, accumulate `fees` fields.

## Drawdown / Volatility Proxy
- Use STATUS profit over time; compute rolling max and drawdown.
  ```python
  import re, sys
  peak = dd = 0
  for line in open(sys.argv[1]):
      if "STATUS" not in line: continue
      m = re.search(r'Profit: \\$([-\\d.,KMR]+)', line);
      if not m: continue
      cur = float(m.group(1).replace(',','').replace('K','000').replace('M','000000'))
      peak = max(peak, cur)
      dd = min(dd, cur-peak)
  print("max_drawdown", dd)
  ```
- Volatility proxy: standard deviation of profit deltas between successive STATUS lines.

## Errors & Suspicious Behavior
- Scan once and report unique issues:
  `rg "ERROR|WARN|Not enough|Overflow|Can't cancel|refused" data/log.txt | sed 's/.*ERROR/ERROR/' | sort | uniq -c`
- Look for runaway trading: high frequency of “submit order” or “Filled” lines in a short window; bucket by minute with `awk` on timestamp.
- Missing config: lines like “Market time calculation invalid” or “Unknown trader” should be surfaced.

## Performance Tips for Huge Logs
- Use `rg` with `-g` to limit to a single file: `rg -n "STATUS" data/log.txt`
- For repeated passes, first extract a slim subset:
  `rg "STATUS|ERROR|WARN|PnL|TRADE" data/log.txt > /tmp/run.slice`
  then operate on `/tmp/run.slice`.
- Use `python -u` one-liners to avoid shell quoting pain; prefer streaming over loading entire files into memory.

## Validating Against `data/log.txt`
- Try: `rg "STATUS" data/log.txt | tail -n1` to get final profit/volume.
- Try: `rg "ERROR|WARN" data/log.txt | head` to ensure error scan works.
- Try: profit delta script above to spot large swings; adjust thresholds if too noisy.

These recipes should give Codex quick answers for total/per-trader profit, timing of big moves, drawdowns, trade stats, and surfaced errors without choking on multi-GB mock logs.
