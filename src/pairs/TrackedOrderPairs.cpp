#include <gtb/TrackedOrderPairs.h>
#include <gtb/OrderPairDb.h>
#include <gtb/Uuid.h>
#include <gtb/Log.h>

using namespace gtb;

// save=Load/Save order pair state to database
TrackedOrderPairs::TrackedOrderPairs(
    bool save)
        : save(save)
{}

void TrackedOrderPairs::init(
    const std::string &algorithm)
{
    this->algorithm = algorithm;
    if (save)
    {
        OrderPairDb::initDb(db);
        std::vector<OrderPair> pairs = OrderPairDb::select(db, algorithm);
        for (OrderPair &pair : pairs)
            orderPairs[pair.uuid] = pair;
    }
}

size_t TrackedOrderPairs::size() const
{
    return orderPairs.size();
}

bool TrackedOrderPairs::insert(
    OrderPair pair)
{
    if (pair.uuid.empty())
        pair.uuid = Uuid::generate();

    if (save && !OrderPairDb::insert(db, pair))
        return false;

    orderPairs[pair.uuid] = std::move(pair);
    return true;
}

bool TrackedOrderPairs::update(
    OrderPair pair)
{
    if (save && !OrderPairDb::update(db, pair))
        return false;

    orderPairs[pair.uuid] = std::move(pair);
    return true;
}

std::string TrackedOrderPairs::getFurthestPending(
    usd_t currentPrice) const
{
    std::string furthestUuid;
    usd_t furthestDistance;
    for (const auto &[uuid, pair] : orderPairs)
    {
        // Only pending
        if (pair.state > OrderPair::State::Pending)
            continue;

        usd_t buyDistance = IntegerUtils::difference(pair.buyPrice, currentPrice);
        usd_t sellDistance = IntegerUtils::difference(pair.sellPrice, currentPrice);
        usd_t distance = std::min(buyDistance, sellDistance);
        if (furthestUuid.empty() || distance > furthestDistance)
        {
            furthestUuid = uuid;
            furthestDistance = distance;
        }
    }

    return furthestUuid;
}

bool TrackedOrderPairs::cancelPair(
    const std::string &uuid)
{
    // Find in list to remove
    auto iter = orderPairs.find(uuid);
    if (iter == orderPairs.end())
        return false;

    OrderPair &pair = iter->second;
    // Set to canceled
    OrderPair clone(pair);
    clone.state = OrderPair::State::Canceled;

    if (save && !OrderPairDb::update(db, clone))
    {
        log::error("Failed to remove order pair '%s' for '%s' from database.",
            uuid.c_str(),
            algorithm.c_str());
        return false;
    }

    log::trade("Removed stale pair '%s' for '%s'.",
        pair.uuid.c_str(),
        algorithm.c_str());

    // Remove from tracking
    orderPairs.erase(iter);

    return true;
}

bool TrackedOrderPairs::cancelPending(
    usd_t currentPrice)
{
    // Find the further away pair that's still pending
    std::string furthestUuid = getFurthestPending(currentPrice);

    // Nothing to cancel
    if (furthestUuid.empty())
        return false;

    // Cancel
    return cancelPair(furthestUuid);
}

std::vector<OrderPair> TrackedOrderPairs::popComplete()
{
    std::vector<OrderPair> completed;

    auto iter = orderPairs.begin();
    while (iter != orderPairs.end())
    {
        OrderPair &pair = iter->second;

        if (pair.state >= OrderPair::State::Complete)
        {
            log::trade("Removing completed order pair '%s' from trader '%s'.",
                pair.uuid.c_str(),
                algorithm.c_str());
            completed.push_back(std::move(pair));
            iter = orderPairs.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    return completed;
}

const std::unordered_map<std::string, OrderPair> &TrackedOrderPairs::getPairs() const
{
    return orderPairs;
}

bool TrackedOrderPairs::patientOverride(
    utime_t conf,
    utime_t curTime) const
{
    if (!conf)
        return false;

    utime_t mostRecentTime;
    for (const auto &[uuid, pair] : orderPairs)
    {
        if (pair.created > mostRecentTime)
            mostRecentTime = pair.created;
    }

    return mostRecentTime + conf <= curTime;
}
