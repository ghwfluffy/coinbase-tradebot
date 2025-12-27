# Order Pair (buy + sell)

Concept
- An OrderPair represents a single logical trade composed of two linked orders: the buy that acquires some quantity of BTC and the matching sell that later disposes of that same quantity.
- Every executed trade is tracked as an OrderPair in the system. The pair always has one buy and one sell for the same quantity of bitcoins (quantity may be computed from a USD "bet" and the buy price).

Key data structure
- File: src/pairs/OrderPair.h
- Important fields:
  - uuid: unique id for the pair
  - algo: name of the algorithm that created the pair
  - buyOrder / sellOrder: exchange order UUIDs (strings)
  - bet: USD amount to risk/spend when buying
  - buyPrice / sellPrice / origSellPrice: price cents for buy/sell
  - quantity: btc quantity (satoshis)
  - created: timestamp when pair was created
  - state: OrderPair::State tracking lifecycle
  - nextTry: SteadyClock::TimePoint used for retry/backoff
  - profit: Profits::Data for accounting of bought/sold amounts and fees

Lifecycle (states)
- Enum values (src/pairs/OrderPair.h):
  - Pending: pair defined but buy order not yet placed
  - BuyActive: buy order submitted and open
  - Holding: buy filled; BTC is held waiting for sell
  - SellActive: sell order submitted and open
  - Complete: sell filled, trade finished
  - Canceled / Error: aborted or failed

How pairs are created
- Order pairs are created by market engines based on the trader configuration.
  - File: src/pairs/OrderPairMarketEngine.cpp
  - Two common creation paths:
    - newStatic(...): creates a pair with explicit buyPrice and sellPrice (used by StaticTrader-like behavior)
    - newSpread(...): creates a pair around current market price using a spread (e.g., buy X% below and sell X% above current price)
  - After prices are set, modifiers based on market periods or configuration may adjust buy/sell prices before finalizing the pair.
  - Quantity calculation: after buy/sell prices are determined the quantity is set via IntegerUtils::getSatoshiForPrice(pair.buyPrice, pair.bet). That makes the bet (USD) the determinant of quantity so the sell will use the same quantity.

How pairs are progressed (state machine)
- File: src/pairs/OrderPairStateMachine.cpp
- The state machine inspects the current market price (BtcPrice model) and the order book (CoinbaseOrderBook) and moves the pair through states:
  - Pending -> BuyActive: when buy conditions are met and a buy order is successfully submitted (ctx.coinbase().submitOrder).
  - BuyActive -> Holding: when the buy order is observed as Filled. When filled, the code records buyPrice and quantity from the executed order and stores paid-before-fees and fee amounts into pair.profit.
  - Holding -> SellActive: when sell conditions are met (market price reaches target); a sell order is submitted for the same quantity.
  - SellActive -> Complete: when the sell order is observed as Filled; sold value and fees are recorded and Profits model is updated.
  - Cancel/Error transitions handled when orders cannot be found, are canceled, or other failures occur.
- The state machine uses SteadyClock::now() and the Time model to control retries and throttling (nextTry and retry windows).

Persistence and bookkeeping
- OrderPair objects are persisted via OrderPairDb (src/pairs/OrderPairDb.h) so the system can resume, inspect, and audit historical pairs.
- When a sell completes, the pair's profit data is added to the Profits model (ctx.data.get<Profits>().addOrderPair(pair.profit)).

Who creates and drives pairs
- Higher-level traders (SpreadTrader, StaticTrader, TimeTrader) use the market engine to create candidate pairs according to their configured logic and push them into the state machine / trading loop.
- The state machine performs order submission via the CoinbaseInterface (production) or MockCoinbase (mock mode) and inspects CoinbaseOrderBook entries and order UUIDs to monitor order status.

Invariant
- An OrderPair always represents the intent to buy a fixed USD bet worth of BTC (quantity derived from buyPrice) and later sell exactly that quantity. That invariant — same quantity bought and sold — is enforced by the creation path and by recording the executed buy quantity into the pair when the buy order fills.

Example (high-level)
- Spread-based pair
  - currentBtcPrice = 100_000 USD, spread = 10% -> buyPrice = 95_000, sellPrice = 105_000
  - quantity = getSatoshiForPrice(buyPrice, bet=500 USD)
  - pair is created as Pending; when price <= buyPrice and wallet has funds the system submits a buy order -> BuyActive
  - when buy fills: record pair.quantity from executed order -> Holding
  - when price >= sellPrice submit sell order for pair.quantity -> SellActive
  - when sell fills: mark Complete and record profits

Files of interest (implementation)
- Data structure and fields: src/pairs/OrderPair.h
- Creation and modifiers: src/pairs/OrderPairMarketEngine.cpp
- Lifecycle and order handling: src/pairs/OrderPairStateMachine.cpp
- DB persistence utilities: src/pairs/OrderPairDb.h

This should give a concise, code-accurate view of what an OrderPair is, how it's created and tracked, and where to look in the source for the concrete behavior.