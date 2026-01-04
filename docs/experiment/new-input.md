Additioanl notes from the user:
  2026-01-03T19:07:00Z
    - Your 30-day trading volume (as seen in the STATUS messages) is way below the target of at least $1M
    - VolumeTrader is intended to lose money (minimally) to keep the 30-day volume high, and thus the maker fee low
  2026-01-04T00:03:00Z
    - Removed MarketConfFactory::allHours. Leaving marketParams empty already means “always act normal”; specifying marketParams adds modifiers on top of that default.
