#pragma once

#include <string>
#include <stdint.h>

namespace gtb
{

namespace IntegerUtils
{
    uint32_t usdToCents(
        const std::string &usd);

    uint64_t btcToSatoshi(
        const std::string &btc);

    std::string centsToUsd(
        uint32_t cents);

    std::string centsToUsd(
        int32_t cents);

    std::string satoshiToBtc(
        uint64_t satoshi);

    uint32_t getValueCents(
        uint32_t priceCents,
        uint64_t satoshi);

    uint64_t getSatoshiForPrice(
        uint32_t btcPriceCents,
        uint32_t amountCents);

    // I call it 'picodollars' but it's really 10^-13 dollars (10^-11 cents)
    uint64_t usdToPico(
        const std::string &str);
}

}
