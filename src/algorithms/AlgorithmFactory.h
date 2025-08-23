#pragma once

#include <gtb/TradeBot.h>

namespace gtb
{

/**
 * Setup the TradeBot instance with a specific algorithm
 */
namespace AlgorithmFactory
{
    bool provision(
        gtb::TradeBot &bot,
        unsigned int version,
        bool mock);
}

}
