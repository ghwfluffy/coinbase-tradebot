#pragma once

#include <gtb/OrderPair.h>
#include <gtb/BaseTraderConfig.h>

namespace gtb
{

/**
 * Create a new order pair for a trader based
 * on it's configuration and the current market.
 *
 * Or adjust pending sales to potentially by discounted
 * according to market conditions.
 */
namespace OrderPairMarketEngine
{
    OrderPair newSpread(
        const BaseTraderConfig &config,
        uint32_t currentBtcPrice,
        uint64_t currentTime,
        uint32_t spread);

    OrderPair newStatic(
        const BaseTraderConfig &config,
        uint64_t currentTime,
        uint32_t buyPrice,
        uint32_t sellPrice);

    void checkSale(
        OrderPair &pair,
        const BaseTraderConfig &config,
        uint64_t currentTime);
}

}
