#include <gtb/SpreadTrader.h>

#include <gtb/OrderPairMarketEngine.h>
#include <gtb/OrderPairUtils.h>
#include <gtb/OrderPairDb.h>
#include <gtb/Time.h>
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
    // Setup the pair
    OrderPair pair = OrderPairMarketEngine::newSpread(
        conf,
        price.getPrice(),
        ctx.data.get<Time>().getTime(),
        conf.spread);
    if (!pair)
        return;

    // Find the closest order pair
    bool first = true;
    usd_t closestDistance;
    for (const OrderPair &pair : orderPairs)
    {
        usd_t buyDistance = IntegerUtils::difference(pair.buyPrice, price.getPrice());
        usd_t sellDistance = IntegerUtils::difference(pair.sellPrice, price.getPrice());
        usd_t midDistance = IntegerUtils::avg(buyDistance, sellDistance);

        usd_t distance = buyDistance;
        if (sellDistance < distance)
            distance = sellDistance;
        if (midDistance < distance)
            distance = midDistance;

        if (first || distance < closestDistance)
            closestDistance = distance;
        first = false;
    }

    // Check if that's far enough away to want to create a new spread
    usd_t spread = pair.sellPrice - pair.buyPrice;
    bool wantNew = first || closestDistance > spread;

    // Not far enough to consider a new pair
    if (!wantNew)
        return;

    // We need to decide if we can cancel something first
    // Try to cancel the furthest pending pair
    if (conf.num_pairs <= orderPairs.size())
    {
        if (!OrderPairUtils::cancelPending(db, price.getPrice(), conf.name, orderPairs))
            return;
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
