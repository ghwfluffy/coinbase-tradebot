#include <gtb/OrderPairUtils.h>
#include <gtb/OrderPairDb.h>
#include <gtb/IntegerUtils.h>
#include <gtb/Log.h>

using namespace gtb;

std::string OrderPairUtils::getFurthestPending(
    usd_t currentPrice,
    const std::list<OrderPair> &orderPairs)
{
    std::string furthestUuid;
    usd_t furthestDistance;
    for (const OrderPair &pair : orderPairs)
    {
        // Only pending
        if (pair.state > OrderPair::State::Pending)
            continue;

        usd_t buyDistance = IntegerUtils::difference(pair.buyPrice, pair.sellPrice);
        usd_t sellDistance = IntegerUtils::difference(pair.sellPrice, currentPrice);
        usd_t distance = std::min(buyDistance, sellDistance);
        if (furthestUuid.empty() || distance > furthestDistance)
        {
            furthestUuid = pair.uuid;
            furthestDistance = distance;
        }
    }

    return furthestUuid;
}

bool OrderPairUtils::cancelPair(
    Database &db,
    const std::string &algName,
    const std::string &uuid,
    std::list<OrderPair> &orderPairs)
{
    // Find in list to remove
    auto iter = orderPairs.begin();
    while (iter != orderPairs.end())
    {
        OrderPair &pair = (*iter);
        if (pair.uuid != uuid)
        {
            ++iter;
            continue;
        }

        // Set to canceled
        OrderPair clone(pair);
        clone.state = OrderPair::State::Canceled;
        if (!OrderPairDb::update(db, clone))
        {
            log::error("Failed to remove order pair '%s' for '%s' from database.",
                pair.uuid.c_str(),
                algName.c_str());
            return false;
        }

        log::info("Removed stale pair '%s' for '%s'.",
            pair.uuid.c_str(),
            algName.c_str());

        // Remove from tracking
        orderPairs.erase(iter);

        return true;
    }

    return false;
}

bool OrderPairUtils::cancelPending(
    Database &db,
    usd_t currentPrice,
    const std::string &algName,
    std::list<OrderPair> &orderPairs)
{
    // Find the further away pair that's still pending
    std::string furthestUuid = getFurthestPending(currentPrice, orderPairs);

    // Nothing to cancel
    if (furthestUuid.empty())
        return false;

    // Cancel
    return cancelPair(db, algName, furthestUuid, orderPairs);
}
