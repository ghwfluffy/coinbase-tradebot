#pragma once

#include <gtb/OrderPair.h>
#include <gtb/Database.h>

#include <list>

namespace gtb
{

namespace OrderPairUtils
{
    std::string getFurthestPending(
        uint32_t currentPrice,
        const std::list<OrderPair> &orderPairs);

    bool cancelPair(
        Database &db,
        const std::string &algName,
        const std::string &uuid,
        std::list<OrderPair> &orderPairs);

    bool cancelPending(
        Database &db,
        uint32_t currentPrice,
        const std::string &algName,
        std::list<OrderPair> &orderPairs);
}

}
