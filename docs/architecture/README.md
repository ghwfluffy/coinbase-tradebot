# Architecture Overview

High-level view of how the tradebot runs end-to-end. Detailed component docs live alongside this file.

## Runtime Flow
- Entry: `src/cli/tradebot.cpp` parses flags (algorithm version, `--mock`), constructs `gtb::TradeBot`, and calls `AlgorithmFactory::provision` to wire sources/processors, then `bot.run()`.
- Context: `TradeBot` owns a single `BotContext` with the `DataController`, `ActionThreadPool`, database handles, and the active `CoinbaseInterface` (real or mock).
- Execution loop: `TradeBot::run` starts all registered data sources (each typically on its own thread) and processes queued actions until shutdown signals are received.

## Data Pipeline
- Sources -> Models -> Subscriptions -> Processors.
- Data sources (e.g., `CoinbaseMarket`, `CoinbaseUserTrades`, `PeriodicTimeUpdater`, `MockMarket`) pull external or replayed data and update `DataModel` instances in `DataController`.
- `DataModel::updated()` notifies `DataController`, which enqueues subscriber callbacks on the action queue; processors and traders subscribe to specific models and react via `process(const T&)`.
- This separation keeps network I/O threads thin and pushes work onto the action pool. See `docs/architecture/sources.md` and `docs/architecture/subscriptions.md`.

## Trading Logic
- Traders (`SpreadTrader`, `StaticTrader`, `TimeTrader`, etc.) are processors that react to model updates and express trading intent; many use the order-pair engine, while others may implement different behaviors.
- Order-pair traders create `OrderPair` objects through `OrderPairMarketEngine`, and `OrderPairStateMachine` advances them across lifecycle states (Pending → BuyActive → Holding → SellActive → Complete) by submitting/canceling orders via `CoinbaseInterface` (real or mock) and monitoring `CoinbaseOrderBook`. Details: `docs/architecture/order-pair.md`.
- Positions and profits are tracked with strong-typed integers for USD/BTC/time. See `docs/architecture/integer-types.md`.

## Persistence and Replay
- Historical DB (`data/historical.sqlite`) is initialized with `schema/historical.sql`; processors like `BtcHistoricalWriter`, `WalletHistoricalWriter`, and `ProfitsWriter` record runtime history. Schema notes: `docs/architecture/sql-schema.md`.
- Order pairs are persisted via `OrderPairDb` (`schema/spread_trader.sql`) so the bot can resume and audit trades.
- Mock mode reuses a copy of the historical DB (`mock_historical.sqlite`) as the event source and swaps in mock implementations to simulate fills and wallet changes. See `docs/architecture/mock.md`.

## Modes
- Production: real Coinbase REST/websocket clients, live wallet/order updates, historical writers enabled.
- Mock: `MockMarket` replays historical price rows into the models, `MockCoinbase`/`MockUserTrades` emulate exchange behavior, `SteadyClock` is driven by the `Time` model for deterministic time.

## Algorithms and Configuration
- `AlgorithmFactory` selects a versioned initializer (e.g., `Version1`, `Version2`). Each initializer registers sources, processors, and trader configurations (spreads, bet size, concurrency, market period rules).
- Traders receive `BotContext` so they can subscribe to models, submit orders, and respect configured limits (spread, buffers, max value, etc.).

## Related Docs
- Sources: `docs/architecture/sources.md`
- Subscriptions & models: `docs/architecture/subscriptions.md`
- Order pairs: `docs/architecture/order-pair.md`
- Integer units: `docs/architecture/integer-types.md`
- SQL schemas: `docs/architecture/sql-schema.md`
- Mock mode: `docs/architecture/mock.md`
