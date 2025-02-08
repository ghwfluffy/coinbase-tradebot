#pragma once

#include <gtb/MarketInfo.h>

namespace gtb
{

/**
 * Parameters for trading during a certain time in the market
 * IE: Configure ramping up trading at the begining of the market open
 */
struct MarketPeriodConfig
{
    // Ramp up to active or ramp down to inactive
    bool hot = true;
    // Don't create any new positions for this long (microseconds)
    uint32_t pausePeriod = 0;
    // Accept this much less on sales during pause buffer
    // Percentage points of the original spread
    // IE 10,000 discounts sell to original buy price
    uint32_t pauseAcceptLoss = 10'000;
    // Ramp up/down for this long (microseconds)
    uint32_t rampPeriod = 0;
    // Ramp up/down buys this much
    // Percentage points of spread
    // IE 10,000 doubles spread at height of ramp
    //  hot=Ramp down from this percent expected extra trade gains to normal
    //  cold=Ramp up from normal expected trade gains to this much extra
    uint32_t rampGrade = 10'000;

    uint32_t bufferPeriod() const
    {
        return pausePeriod + rampPeriod;
    }
};

/**
 * Configure trading based on market open/close times
 */
struct MarketTimeTraderConfig
{
    // Which market this pertains to
    MarketInfo::Market market = MarketInfo::Market::None;

    // How to trade during each market period
    // Open -> Closed
    MarketPeriodConfig closingMarket = {};
    // Closed ---->
    MarketPeriodConfig closedMarket = {};
    // Closed -> Open
    MarketPeriodConfig openingMarket {};
    // Open ---->
    MarketPeriodConfig openMarket = {};
    // Open -> Weekend
    MarketPeriodConfig weekendingMarket = {};
    // Weekend ---->
    MarketPeriodConfig weekendMarket = {};
    // Weekend -> Open
    MarketPeriodConfig weekStartingMarket = {};
};

}
