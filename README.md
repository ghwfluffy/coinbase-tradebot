# C++ Coinbase Bitcoin Trading Bot

A C++20 trading bot for **Coinbase** focused on **BTC** market (`BTC-USD`).
Uses REST for account/orders and WebSocket for market data/user events.

---

## System Dependencies

- **C++20** (compiler + standard library)
- **Boost.System**
- **Boost.Thread**
- **OpenSSL**
- **SQLite3**
- **nlohmann/json** (JSON for Modern C++)
- **cURL**

---

## Submodule Dependencies

- **websocketpp** (WebSocket client)
- **jwt-cpp** (JWT signing / auth helper)
- **cpr** (HTTP client wrapper over libcurl)
