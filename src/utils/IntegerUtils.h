#pragma once

#include <gtb/IntLiterals.h>
#include <gtb/BigInt.h>

#include <string>

namespace gtb
{

/**
 * Utilities for converting between different usd/btc price types
 */
namespace IntegerUtils
{
    // From $X.YY format (or X.YYYYYYYY) to usd_t
    usd_t fromUsdString(
        const std::string &usd);

    // Format a decipicodollar (usd_t) amount into $X.YY using cent granularity
    std::string toUsdString(
        usd_t picos);

    // Possibly negative
    std::string toUsdString(
        int64_t picos);

    // From fractional bitcoin format (X.YYYYYYYY) to btc_t
    btc_t fromBtcString(
        const std::string &btc);

    // Format satoshi's into fractional bitcoins
    std::string toBtcString(
        btc_t satoshi);

    // Value of 'satoshi' bitcoins at 'price'
    usd_t getValue(
        usd_t price,
        btc_t satoshi);

    // Price of 'satoshi' bitcoins purchased for 'value'
    usd_t getPrice(
        usd_t value,
        btc_t satoshi);

    usd_t getPrice(
        const BigInt &value,
        const BigInt &satoshi);

    usd_t getPrice(
        const big_usd_t &value,
        const big_btc_t &satoshi);

    // Number of bitcoins you could buy with 'transactionSize' USD at 'btcPrice' price
    btc_t getSatoshiForPrice(
        usd_t btcPrice,
        usd_t transactionSize);

    template<typename IntType>
    IntType difference(
        IntType lhs,
        IntType rhs)
    {
        return lhs < rhs ? (rhs - lhs) : (lhs - rhs);
    }

    template<typename IntType>
    IntType avg(
        IntType lhs,
        IntType rhs)
    {
        return IntType((lhs.value() + rhs.value()) / 2);
    }

    template<typename IntType>
    pp_t fraction(
        IntType numerator,
        IntType denominator)
    {
        return pp_t(static_cast<uint32_t>((numerator.value() * pp_t(100_Percent).value()) / denominator.value()));
    }

    // Compact formatting for USD values (e.g., $1.2K, $3M) using usd_t input.
    std::string toUsdCompact(
        usd_t amount);

    std::string toUsdCompact(
        big_usd_t amount);

    // Maker-friendly price adjustments to avoid taking.
    usd_t makerBuyPrice(
        usd_t mid,
        usd_t offset = 2_Dollars);

    usd_t makerSellPrice(
        usd_t mid,
        usd_t offset = 2_Dollars);
}

}
