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
}

}
