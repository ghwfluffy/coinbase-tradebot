# SQL Schema (historical & order pairs)

This document describes the SQLite schemas used by the bot and how the database columns map to the strongly-typed integer units used throughout the codebase. The canonical SQL files are in the schema/ directory.

Schema files
- Historical data: schema/historical.sql
- Order pairs (spread trader): schema/spread_trader.sql

Historical schema (schema/historical.sql)
- File contents:

```sql schema/historical.sql
CREATE TABLE IF NOT EXISTS btc_price (
    time INTEGER PRIMARY KEY,
    price INTEGER
);

CREATE TABLE IF NOT EXISTS profits_and_losses (
    time INTEGER PRIMARY KEY,
    purchased INTEGER,
    sold INTEGER,
    buy_fees INTEGER,
    sell_fees INTEGER,
    profit INTEGER
);

CREATE TABLE IF NOT EXISTS wallet (
    time INTEGER PRIMARY KEY,
    usd INTEGER,
    btc INTEGER,
    value INTEGER
);
```

- Column meanings and types (as used by the code):
  - btc_price.time: utime_t microseconds since epoch (stored as INTEGER). See src/models/Time.h/.cpp and IntLiterals for utime_t.
  - btc_price.price: usd_t price stored as INTEGER (decipicodollars — see IntLiterals for scale).
  - profits_and_losses.*: all monetary columns are usd_t stored as INTEGER. profit column is an INTEGER representing profit in the same usd_t units.
  - wallet.time: utime_t timestamp. wallet.usd and wallet.value are usd_t (INTEGER). wallet.btc is btc_t (satoshis stored as INTEGER).

Order pairs schema (schema/spread_trader.sql)
- File contents:

```sql schema/spread_trader.sql
CREATE TABLE IF NOT EXISTS order_pairs (
    uuid VARCHAR PRIMARY KEY,
    algorithm VARCHAR,
    state VARCHAR,
    buy_order_uuid VARCHAR,
    sell_order_uuid VARCHAR,
    bet INTEGER,
    buy_price INTEGER,
    sell_price INTEGER,
    quantity INTEGER,
    created INTEGER,
    final_purchased INTEGER,
    final_buy_fees INTEGER,
    final_sold INTEGER,
    final_sell_fees INTEGER
);
```

- Column meanings and code mapping:
  - uuid, algorithm, state: textual values mapping to OrderPair.uuid, OrderPair.algo, and OrderPair::State (to_string/from_string).
  - buy_order_uuid, sell_order_uuid: exchange order UUID strings recorded when orders are submitted.
  - bet: stored as INTEGER containing the underlying usd_t value (decipicodollars) — this is the USD stake used to calculate quantity.
  - buy_price / sell_price: usd_t (INTEGER) representing price in decipicodollars.
  - quantity: btc_t (INTEGER) stored in satoshis.
  - created: utime_t (INTEGER) microseconds timestamp for pair creation.
  - final_purchased, final_buy_fees, final_sold, final_sell_fees: usd_t (INTEGER) final accounting values recorded after fills.

How the code reads/writes these fields
- Historical DB is initialized in Version1:
  - src/algorithms/Version1.cpp: ctx.historicalDb.init("historical.sqlite", "./schema/historical.sql");
  - In mock mode initMock copies historical.sqlite -> mock_historical.sqlite and uses that copy for replay.
- Order pair DB is initialized by OrderPairDb::initDb which calls Database::init(dbFile, SCHEMA_FILE) with SCHEMA_FILE = "./schema/spread_trader.sql" (src/pairs/OrderPairDb.cpp).
- When reading rows the code reads raw integer columns and constructs the typed wrappers. Example (src/pairs/OrderPairDb.cpp):

```cpp src/pairs/OrderPairDb.cpp
pair.bet = usd_t(result[col++].getUInt64());
pair.buyPrice = usd_t(result[col++].getUInt64());
pair.quantity = btc_t(result[col++].getUInt64());
pair.created = utime_t(result[col++].getUInt64());
```

- When writing rows the code emits the underlying .value() (or .value() cast to integer) into the SQL. Example inserts/updates in OrderPairDb.cpp use pair.buyPrice.value(), pair.quantity.value(), pair.bet.value(), etc.

Units and scaling notes
- usd_t is a decipicodollars integer type. The user literals in src/utils/IntLiterals.h define 1_Dollars, 1_Cents, and the base unit (Decipicodollars). Internally, dollars are represented as large integers (1_Dollars == 10^13 decipicodollars). That means all USD money stored in DB columns is in this high-resolution integer unit.
- btc_t stores satoshis: 1_Bitcoins == 100,000,000 satoshis. quantity columns are stored as satoshis.
- utime_t stores microseconds since epoch (INTEGER).
- pp_t (percentage points) are not stored directly in these schemas but are used elsewhere.

Recommendations and cautions
- The DB stores the raw underlying integer representation. Always use the typed wrappers (usd_t, btc_t, utime_t) when loading values from the DB and use .value() when writing back to SQL — see OrderPairDb.cpp.
- When displaying values to humans, use IntegerUtils::toUsdString(), toBtcString(), etc., to convert the stored integers into readable formats.
- Because USD is stored in a very fine-grained integer unit, displaying or formatting must convert to conventional dollars/cents for UI or logs.
- The schema integers can be large — use 64-bit storage (INTEGER in SQLite maps to 64-bit) and consistent typed conversion in code to avoid overflow or truncation.

Files of interest
- SQL: schema/historical.sql, schema/spread_trader.sql
- DB wiring: src/algorithms/Version1.cpp (historical DB init), src/pairs/OrderPairDb.cpp (order pairs DB init / select / insert / update)
- Type definitions: src/utils/IntLiterals.h, src/utils/StrongTypedInt.h
- Conversions: src/utils/IntegerUtils.*

This document should allow you to understand how data is persisted and how DB integer columns map to the in-memory strongly-typed units used throughout the codebase.