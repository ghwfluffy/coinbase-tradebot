#include <gtb/SpreadTrader.h>

#include <gtb/OrderPairMarketEngine.h>
#include <gtb/OrderPairUtils.h>
#include <gtb/OrderPairDb.h>
#include <gtb/Time.h>
#include <gtb/Uuid.h>
#include <gtb/Log.h>

using namespace gtb;

SpreadTrader::SpreadTrader(
    BotContext &ctx,
    Config confIn)
        : OrderPairTrader(ctx, confIn)
        , conf(std::move(confIn))
{
}

void SpreadTrader::handleNewPair(
    const BtcPrice &price)
{
    // Find the closest order pair
    bool first = true;
    uint32_t closestDistance = 0;
    for (const OrderPair &pair : orderPairs)
    {
        int32_t buyDistance = static_cast<int32_t>(pair.buyPrice - price.getCents());
        if (buyDistance < 0)
            buyDistance *= -1;

        int32_t sellDistance = static_cast<int32_t>(pair.sellPrice - price.getCents());
        if (sellDistance < 0)
            sellDistance *= -1;

        int32_t midDistance = static_cast<int32_t>(
            ((pair.sellPrice + pair.buyPrice) / 2) - price.getCents());
        if (midDistance < 0)
            midDistance *= -1;

        uint32_t distance = static_cast<uint32_t>(buyDistance);
        if (sellDistance < static_cast<int32_t>(distance))
            distance = static_cast<uint32_t>(sellDistance);
        if (midDistance < static_cast<int32_t>(distance))
            distance = static_cast<uint32_t>(midDistance);

        if (first || distance < closestDistance)
            closestDistance = distance;
        first = false;
    }

    // Check if that's far enough away to want to create a new spread
    uint32_t spread_cents = (((price.getCents() * conf.spread) + 9'999) / 10'000);
    bool wantNew = first || (closestDistance > ((spread_cents * conf.buffer_percent) / 100));

    // Not far enough to consider a new pair
    if (!wantNew)
        return;

    // Setup the pair
    OrderPair pair = OrderPairMarketEngine::newSpread(
        conf,
        price.getCents(),
        ctx.data.get<Time>().getTime(),
        conf.spread);
    if (!pair)
        return;

    // We need to decide if we can cancel something first
    // Try to cancel the furthest pending pair
    while (conf.num_pairs >= orderPairs.size())
    {
        if (!OrderPairUtils::cancelPending(db, price.getCents(), conf.name, orderPairs))
            break;
    }

    // We have too many orders already and we haven't been patient enough to exceed max
    if (conf.num_pairs <= orderPairs.size() && !patientOverride())
        return;

    // Add the pair
    if (!OrderPairDb::insert(db, pair))
    {
        log::error("Failed to insert new order pair for spread '%s' in database.",
            conf.name.c_str());
        return;
    }

    log::info("Created new pair for spread '%s'.", conf.name.c_str());
    stateMachine.logChange(OrderPair::State::None, pair);
    orderPairs.push_back(std::move(pair));
}

bool SpreadTrader::patientOverride() const
{
    uint64_t mostRecentTime = 0;
    for (const OrderPair &pair : orderPairs)
    {
        if (pair.created > mostRecentTime)
            mostRecentTime = pair.created;
    }

    // TODO: Configurable
    // Can put in 1 new trade every half hour over our limit
    constexpr const uint64_t PATIENCE_MICROSECONDS = 30ULL * 60ULL * 1'000'000ULL;
    return mostRecentTime + PATIENCE_MICROSECONDS <= ctx.data.get<Time>().getTime();
}
