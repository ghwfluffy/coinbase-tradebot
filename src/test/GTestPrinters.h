#pragma once

#include <gtb/IntLiterals.h>
#include <gtb/IntegerUtils.h>
#include <gtb/StrongTypedBigInt.h>
#include <gtb/BigInt.h>

#include <iomanip>
#include <ostream>
#include <sstream>

namespace gtb
{

inline void PrintTo(
    usd_t v,
    std::ostream *os)
{
    *os << IntegerUtils::toUsdString(v);
}

inline void PrintTo(
    big_usd_t v,
    std::ostream *os)
{
    *os << IntegerUtils::toUsdCompact(v);
}

inline void PrintTo(
    btc_t v,
    std::ostream *os)
{
    *os << IntegerUtils::toBtcString(v);
}

inline void PrintTo(
    big_btc_t v,
    std::ostream *os)
{
    uint64_t sat = 0;
    if (v.value().tryToUint64(sat))
    {
        *os << IntegerUtils::toBtcString(btc_t(sat));
    }
    else
    {
        *os << "<big_btc>";
    }
}

inline void PrintTo(
    pp_t v,
    std::ostream *os)
{
    double pct = static_cast<double>(v.value()) / static_cast<double>(PP_SCALE);
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << pct << '%';
    *os << ss.str();
}

inline void PrintTo(
    utime_t v,
    std::ostream *os)
{
    double secs = static_cast<double>(v.value()) / 1'000'000.0;
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3) << secs << "s";
    *os << ss.str();
}

inline void PrintTo(
    const BigInt &bn,
    std::ostream *os)
{
    uint64_t val = 0;
    if (bn.tryToUint64(val))
        *os << val;
    else
        *os << "<bigint>";
}

}

