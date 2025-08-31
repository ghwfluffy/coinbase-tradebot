#pragma once

#include <gtb/OrderPair.h>
#include <gtb/BaseTraderConfig.h>

namespace gtb
{

/**
 * Create a new order pair for a trader based
 * on it's configuration and the current market.
 *
 * Or adjust pending sales to potentially be discounted
 * according to market conditions.
 */
namespace OrderPairMarketEngine
{
    OrderPair newSpread(
        const BaseTraderConfig &config,
        usd_t currentBtcPrice,
        utime_t currentTime,
        pp_t spread);

    OrderPair newStatic(
        const BaseTraderConfig &config,
        utime_t currentTime,
        usd_t buyPrice,
        usd_t sellPrice);

    void checkSale(
        OrderPair &pair,
        const BaseTraderConfig &config,
        utime_t currentTime);
}

}
