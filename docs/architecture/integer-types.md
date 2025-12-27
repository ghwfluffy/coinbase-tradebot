# Strongly-typed integer units (usd_t, btc_t, pp_t, utime_t)

The codebase uses small, explicit integer wrapper types for domain quantities to avoid unit bugs and make intent clear. These types are defined using StrongTypedInt and a set of user-defined literals so you write money, satoshis, percentages and times in a natural, type-safe way.

Where to look in the source
- Strong-typed integer implementation: src/utils/StrongTypedInt.h
- Unit typedefs and user literals: src/utils/IntLiterals.h
- Utility helpers (conversion, formatting, arithmetic helpers that need high-precision math): src/utils/IntegerUtils.h/.cpp

Overview of the main types
- usd_t: decipicodollars (1 decipicodollar = 1 / 10^13 of a dollar). Used for all USD values in the system. Defined in src/utils/IntLiterals.h.
- btc_t: satoshi units (1 Bitcoin = 100,000,000 satoshis). Used for BTC quantities.
- pp_t: percentage-points (1 pp_t = 1/100 of a percent). Used for fee tiers, spreads, and related ratios.
- utime_t: microseconds since epoch used by the Time model.

Implementation details
- The wrappers are typedefs of StrongTypedInt<Rep, Tag> (src/utils/StrongTypedInt.h). StrongTypedInt provides:
  - explicit constructor from the underlying integral Rep (e.g., uint64_t)
  - .value() accessor to retrieve the raw integer
  - comparisons (operator<=>) only between the same StrongTypedInt type
  - same-type arithmetic (+, -, +=, -=)
  - scaling by integer scalars (T * int, T / int)
  - Ratio operation: dividing one StrongTypedInt by another of the same type returns the underlying Rep (Rep operator/(T,T))

User-defined literals (src/utils/IntLiterals.h)
- Dollars and cents
  - 1_Dollars, 1_Cents — construct usd_t values with literal helpers. Example:

```cpp
usd_t price = 100_Dollars;   // $100
usd_t fee  = 10_Cents;       // $0.10
```

- Bitcoin / Satoshis

```cpp
btc_t q = 1_Bitcoins;   // 100,000,000 satoshis
btc_t s = 123_Satoshi;   // 123 satoshis
```

- Percentage points

```cpp
pp_t p = 10_PercentagePoints; // 10 (meaning 0.10% when interpreted with PP_SCALE)
pp_t r = 5_Percent;           // 5% -> represented as 500 (because Percent multiplies by 100 internally)
```

- Time

```cpp
utime_t t = 10_Minutes;    // microseconds for 10 minutes
```

pp_t arithmetic and scaling
- IntLiterals.h provides helpers so you can multiply and divide strong-typed integers by pp_t (percentage-points). The semantics use a PP_SCALE of 10,000 which corresponds to 100%.
  - T * pp_t and pp_t * T scale a StrongTypedInt by a percentage-point value.
  - T / pp_t divides a StrongTypedInt by a percentage-point value (useful for computing a base value from a percentage).
- Implementation attempts to use 128-bit intermediate math when available to minimize overflow; fallbacks are provided for smaller platforms.

Integer utilities (src/utils/IntegerUtils.*)
- IntegerUtils provides correct and convenient conversions and math for usd_t, btc_t, and their interactions:
  - fromUsdString / toUsdString: parse/format usd_t in human readable $X.YY format.
  - fromBtcString / toBtcString: parse/format BTC quantities (fractional bitcoin) to/from btc_t (satoshis).
  - getValue(usd_t price, btc_t satoshi): compute USD value for a given satoshi amount at a price. Implemented using big-number (OpenSSL BIGNUM) mul/div to avoid intermediate overflow: value = (price * satoshi) / 1_Bitcoins.
  - getSatoshiForPrice(usd_t btcPrice, usd_t transactionSize): compute how many satoshis you can buy with transactionSize USD at btcPrice, using mul/div with BIGNUM to maintain precision.
  - Templates: difference, avg, fraction helpers. fraction returns a pp_t computed as (numerator * 100% / denominator) using the wrapper types.

Examples (real code snippets)
- Convert and compute value:

```cpp
usd_t price = IntegerUtils::fromUsdString("100000.00"); // $100,000
btc_t sat = IntegerUtils::fromBtcString("0.005"); // 0.005 BTC -> satoshis
usd_t value = IntegerUtils::getValue(price, sat); // accurate (price * satoshi / 1_Bitcoins)
```

- Compute quantity from USD spend:

```cpp
usd_t spend = 500_Dollars;
btc_t quantity = IntegerUtils::getSatoshiForPrice(price, spend);
```

- Percent scaling

```cpp
usd_t amount = 100_Dollars;
pp_t feeRate = 15_PercentagePoints; // e.g. 0.15%
usd_t fee = amount * feeRate; // uses operator* implemented in IntLiterals.h
```

Caveats and gotchas
- StrongTypedInt is intentionally strict: arithmetic and comparisons are only allowed between the same unit types. This prevents accidentally mixing cents with satoshis, etc.
- Use .value() when you need the raw integer, but prefer IntegerUtils helpers and the provided operators wherever possible to avoid manual mistakes.
- IntegerUtils uses OpenSSL BIGNUM functions for (A * B) / C calculations to avoid overflow; those functions will fail if the denominator is zero — the caller must ensure denominators are valid.
- Division semantics:
  - Dividing a StrongTypedInt by an integer scalar returns a StrongTypedInt (floor division of the underlying representation).
  - Dividing one StrongTypedInt by another returns the underlying Rep (integer ratio) — not a typed result.

Files to read for implementation details
- src/utils/StrongTypedInt.h
- src/utils/IntLiterals.h
- src/utils/IntegerUtils.h
- src/utils/IntegerUtils.cpp

These utilities and types give you a clear, safe way to work with money, bitcoin quantities, percentages, and time in the codebase. They reduce unit-related bugs and centralize the tricky integer math (rounding/overflow) in a small, audited set of helpers.
