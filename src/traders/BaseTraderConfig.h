#pragma once

#include <gtb/MarketTimeTraderConfig.h>

#include <vector>

namespace gtb
{

struct BaseTraderConfig
{
    std::string name;
    bool enabled = true;
    // Amount to buy each spread
    usd_t bet;
    // The maximum price we will queue a sale for
    usd_t maxValue = 115'000_Dollars;
    // How long we need to wait till we can exceed our max leverage (0=disabled)
    utime_t patienceOverride;
    // Remove pending pairs that haven't filled in this much time (0=diabled)
    utime_t pendingPairExpiration;

    std::vector<MarketTimeTraderConfig> marketParams;
};

}
