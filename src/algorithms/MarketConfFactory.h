#pragma once

#include <gtb/MarketTimeTraderConfig.h>

namespace gtb
{

/**
 * Sets of configurations that describe what to do
 * based on market hours
 */
namespace MarketConfFactory
{
    MarketTimeTraderConfig onlyNormalHours();
    MarketTimeTraderConfig preferNormalHours();
}

}
