#pragma once

#include <gtb/OrderPair.h>
#include <gtb/Database.h>

#include <list>

namespace gtb
{

/**
 * Serialize OrderPair to/from database
 */
namespace OrderPairDb
{
    std::list<OrderPair> select(
        Database &db,
        const std::string &algorithm = std::string(),
        bool activeOnly = true);

    bool remove(
        Database &db,
        const std::string &uuid);

    bool insert(
        Database &db,
        OrderPair &pair);

    bool update(
        Database &db,
        const OrderPair &pair);
}

}
