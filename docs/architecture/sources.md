# Data Sources

Overview
- Data sources are the origin of runtime events and external state for the trading bot. They receive data from external systems (exchange websockets, REST APIs, mocks, etc.) and translate those external events into updates on DataModel instances.
- Each data source typically runs on its own thread or uses asynchronous callbacks so that incoming network events do not block the main algorithm logic.
- Examples of sources in the codebase: CoinbaseMarket (websocket trade feed), CoinbaseUserTrades (user trade events), periodic pollers that fetch wallet/trade history, or mock sources used for simulation.

Responsibilities
- Connect to an external data stream (websocket, REST polling, or mock generator).
- Parse incoming messages and convert them into domain objects (e.g., CoinbaseOrder, trades, balances).
- Update the relevant DataModel(s) via the BotContext's DataController (e.g., ctx.data.get<BtcPrice>().setPrice(...)).
- Handle reconnection, retries, and any source-specific error handling.
- Provide clean shutdown of its thread(s) and network clients.

Threading and safety
- Many sources inherit from ThreadedDataSource (or similar) and implement a process() loop that runs on a dedicated thread. This isolates network I/O from the rest of the system.
- Sources should perform minimal work on network callbacks: parse, update models, and return. Heavy computation should be scheduled on the action pool if needed.
- DataModel implementations are responsible for their own internal synchronization. For example CoinbaseOrderBook uses a mutex when mutating orders.

Interaction with DataModels and subscriptions
- When a source updates a DataModel, the model calls updated(), which notifies the DataController listener. The DataController then enqueues notifications to all subscribers of that DataModel type.
- Because notifications are queued through the ActionQueue, subscribers process updates on the action/thread pool rather than inside the source thread. This reduces coupling and prevents long-running subscriber work from delaying network handling.

Sequence diagram (ASCII)

Source Thread (e.g. CoinbaseMarket) -> parse message -> update model

  [Source Thread]
      |
      |  (1) receive websocket / poll response
      |  parse -> domain object
      v
  [DataModel] (e.g. BtcPrice, CoinbaseOrderBook)
      |
      |  (2) model mutates state and calls updated()
      v
  [DataController]
      |
      |  (3) DataController::updated<T>() enqueues callbacks on ActionQueue
      v
  [ActionQueue / ActionThreadPool]
      |
      |  (4) ActionQueue executes each subscriber callback
      v
  [Subscriber]
      |
      |  (5) subscriber.process(const T&) runs and reacts to new state
      v

This flow keeps network I/O and parsing in the source thread while subscriber processing runs on the action pool.

Examples
- Coinbase websocket feed (CoinbaseMarket): receives trade messages, parses price/size, and updates market models (e.g., BtcPrice, order book entries, trade lists).
- Wallet poller: periodically calls the exchange REST API to fetch wallet balances and user orders, then writes that state to CoinbaseWallet or CoinbaseOrderBook models. Useful as a fallback if websockets miss events.
- Mock sources: used for testing and simulations; they update the same DataModels as production sources so algorithms and processors can operate unchanged.

Design benefits
- Clear separation of concerns: sources only handle receiving and translating external data; models hold state; controllers manage subscriptions; processors react to model changes.
- Resilience: combining websocket-driven updates with polling sources helps ensure eventual consistency even when live feeds miss messages.
- Testability: mock sources can drive models deterministically for unit and integration tests.

Implementation notes
- Register sources at startup (e.g., in an algorithm or bot initializer). Each source takes BotContext& so it can access ctx.data and ctx.actionPool.
- Ensure proper shutdown ordering: sources should be stopped before destroying shared resources in BotContext.

This document describes the role and expectations of data sources in the bot architecture: they feed DataModels, and through the DataController/subscription mechanism, changes propagate to interested processors.