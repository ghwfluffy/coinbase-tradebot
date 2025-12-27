# Directory Layout

Top-level folders and what they contain.

- `src/` — production code for the trading bot.
  - `algorithms/` — pluggable strategy implementations and factory wiring.
  - `cli/` — command-line entrypoint and misc tools.
  - `comm/` — REST and WebSocket clients plus Coinbase interface glue.
  - `core/` — core trading bot architecture classes.
  - `database/` — lightweight SQLite wrapper utilities.
  - `historical/` — writers/readers for persisting and replaying historical data.
  - `mock/` — infrastructure for running the trading bot in "mock" mode against historical data.
  - `models/` — data models that can be watched for changes and referenced by algorithm implementations.
  - `pairs/` — order pairs for creating a buy and matching sell pair.
  - `traders/` — trader configurations and concrete trader behaviors.
  - `utils/` — generic helpers.
  - `test/` — unit-test driver and fixtures.
- `docs/` — architecture notes, style guides, and organization docs (this file).
- `libs/` — vendored third-party libraries and submodules
- `cmake/` — build files.
- `data/` — input/output directory for SQLite data
- `schema/` — SQL definitions for initializing the SQLite schemas.
- `graph/` — scripts and environment files for plotting or analysis workflows.
- `secrets/` — holds API credentials/config (not checked in; keep private).
