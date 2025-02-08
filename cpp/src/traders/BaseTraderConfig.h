#pragma once

#include <gtb/MarketTimeTraderConfig.h>

#include <list>

namespace gtb
{

struct BaseTraderConfig
{
    std::string name;
    bool enabled = true;
    // Cents to buy each spread
    uint32_t cents = 0;
    // The maximum price we will queue a sale for
    uint32_t maxValue = 105'000'00;
    // How long we need to wait till we can exceed our max leverage
    uint32_t patienceOverride = 0;
    // Remove pending pairs that haven't filled in this much time
    uint64_t pendingPairExpiration = 30 * 60 * 1'000'000ULL;

    std::list<MarketTimeTraderConfig> marketParams;
};

}
