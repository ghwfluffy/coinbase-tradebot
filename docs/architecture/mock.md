# Mock Mode

This document explains exactly how mock mode is wired in the codebase.

How mock mode is enabled
- The command-line switch "-m" (or --mock) is parsed in src/cli/tradebot.cpp. If present, the main routine passes mock=true into AlgorithmFactory::provision.
  - File: src/cli/tradebot.cpp
  - Arg parsing: gtb::ArgParser, parser.addSwitch('m', "mock", ...)
  - Behavior: main calls AlgorithmFactory::provision(bot, version, mock) and then bot.run().

How Version1 sets up mock mode (src/algorithms/Version1.cpp)
- Version1::init(bot, mock) calls initMock(bot) when mock==true.
- initMock performs these concrete steps:
  - ctx.data.initData(MockMode(true)); // registers MockMode data model
  - SteadyClock::setMockTime(ctx.data.get<Time>()); // register the Time model as the mock clock source
  - Copy or create a copy of the historical DB and initialize ctx.historicalDb with the copied file (mock_historical.sqlite)
  - ctx.setCoinbase(std::make_unique<MockCoinbase>(ctx, FEE_TIER)); // use the MockCoinbase implementation
  - Initialize some model state: set CoinbaseInit to full init, set CoinbaseWallet to a starting USD/BTC state, set CoinbaseFeeTier
  - Add sources/processors used in mock mode:
    - bot.addSource(std::make_unique<MockMarket>(ctx)); // replays historical market prices
    - bot.addProcessor(std::make_unique<MockUserTrades>(ctx)); // emulates order fills based on current price
    - bot.addProcessor(std::make_unique<PeriodicPrinter>(ctx));
    - bot.addProcessor(std::make_unique<PendingProfitsCalc>(ctx));

What SteadyClock / Time do (src/utils/SteadyClock.cpp, src/models/Time.*)
- The SteadyClock has a global pointer to a Time model instance (set by SteadyClock::setMockTime).
  - When set, SteadyClock::now() returns a TimePoint constructed from mockTime->getTime().
  - When not set, SteadyClock::now() returns wall-clock steady_clock time converted to microseconds.
- The Time data model (src/models/Time.h/.cpp) stores a utime_t microsecond timestamp and exposes setTime(utime_t) and getTime(). When setTime changes the stored value it calls updated() to notify subscribers.

How historical replay works (MockMarket — src/mock/MockMarket.cpp / MockMarket.h)
- MockMarket queries the historical database's btc_price table for the next record in chronological order (SELECT time, price ... ORDER BY time ASC LIMIT 1).
- For each row returned it:
  1. Acquires a MockLock (ctx.data.get<MockLock>().lock()) to provide a coarse synchronization boundary for mock state updates.
  2. Reads the time and price fields from the DB row.
  3. Calls ctx.data.get<Time>().setTime(time) — this updates the shared Time model and therefore changes SteadyClock::now() behavior.
  4. Calls ctx.data.get<BtcPrice>().setPrice(price) — this updates the market price DataModel and triggers its updated() path.
  5. Advances its internal curTime position and unlocks.
- After injecting the model updates, MockMarket waits for the action pool to finish executing all downstream actions enqueued by the notifications:
  - It queues a lambda that calls ctx.actionPool.waitComplete(wakeup) and then waits on a condition variable (condChurn) until the action pool invokes the supplied wakeup callback. This ensures subscribers have processed the current event before the mock advances to the next historical record.
- When no more rows are available MockMarket logs "Simulation complete." and raises SIGTERM to stop the process.

How mock order processing works (MockUserTrades, MockCoinbase)
- MockUserTrades subscribes to BtcPrice updates. Its process(const BtcPrice &price) inspects the current order book and wallet (via CoinbaseOrderBook and CoinbaseWallet models) and marks open orders as Filled when the replayed price meets the order's conditions. It then updates the order book and wallet models.
- MockCoinbase implements the CoinbaseInterface methods against the in-memory models. For example:
  - submitOrder checks wallet availability, moves funds to on-hold, and inserts an Open order into CoinbaseOrderBook.
  - cancelOrder updates wallet on-hold fields and marks orders Canceled in CoinbaseOrderBook.
- These components use MockLock to serialize access to multiple related models while performing multi-step updates.

Coordination of time and events
- Crucially, SteadyClock is set to read time from the Time data model when mock mode is active. Because mock replay calls Time::setTime(...) with the historical timestamp prior to updating BtcPrice and other models, all code that calls SteadyClock::now() will see the replayed event time rather than wall-clock time.
- The system therefore runs deterministically with respect to time so long as components use SteadyClock (or Time) rather than raw std::chrono::steady_clock directly.

Where historical data comes from
- In production mode the bot installs processors that record historical data into ctx.historicalDb (BtcHistoricalWriter, WalletHistoricalWriter, ProfitsWriter, etc.). Version1::initProd registers these processors so the historical database is continuously populated during normal runtime.
- initMock copies or opens a copy of historical.sqlite (mock uses mock_historical.sqlite) and MockMarket reads from that file.

Practical modes of replay
- The existing implementation replays strictly by reading the next database row and injecting its time/price into the models. MockMarket enforces that subscribers finish processing each injected event before continuing. The current code supports:
  - Full sequential replay from DB (default). The pace is controlled implicitly by MockMarket's waiting for action pool completion and a cond wait timeout.
  - You can construct MockMarket with optional start/end dates (it converts YYYY-mm-dd to microsecond timestamps) to limit the replay window.

Important implementation constraints (from the code)
- For determinism, the system requires components to call SteadyClock::now() (or use the Time model) — any code directly calling std::chrono::steady_clock::now() will observe wall-clock time and break mock determinism.
- MockMarket and MockUserTrades use MockLock to avoid races when updating related models; other components that touch multiple models under mock should follow the same pattern.

Summary
- Mock mode is explicitly enabled by the CLI -m switch and provisioned via Version1::initMock. The implementation:
  - Registers a Time model and points SteadyClock at it.
  - Uses a copy of the historical DB as the event source.
  - Replays rows from btc_price by setting Time and BtcPrice models and waiting for downstream processing to complete between events.
  - Uses MockCoinbase and MockUserTrades to emulate order submission and fills against the in-memory models.
  - You get deterministic, time-controlled replays driven by real historical data with in-process emulation of the exchange and order fills. You can test new algorithms faster than realtime without betting real month.
