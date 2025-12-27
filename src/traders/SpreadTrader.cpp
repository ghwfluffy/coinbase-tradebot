#include <gtb/SpreadTrader.h>

#include <gtb/OrderPairMarketEngine.h>
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
    for (const auto &[uuid, other] : orderPairs.getPairs())
    {
        bool firstOther = true;
        usd_t closestOtherDistance;
        for (usd_t myPrice : std::vector<usd_t> {
            pair.buyPrice,
            pair.sellPrice,
            IntegerUtils::avg(pair.buyPrice, pair.sellPrice),
            price.getPrice()})
        {
            usd_t buyDistance = IntegerUtils::difference(other.buyPrice, myPrice);
            usd_t sellDistance = IntegerUtils::difference(other.sellPrice, myPrice);
            usd_t midDistance = IntegerUtils::difference(IntegerUtils::avg(other.sellPrice, other.buyPrice), myPrice);
            usd_t distance = std::min(buyDistance, sellDistance);
            distance = std::min(distance, midDistance);
            if (firstOther || distance < closestOtherDistance)
                closestOtherDistance = distance;
        }

        if (first || closestOtherDistance < closestDistance)
            closestDistance = closestOtherDistance;
        first = false;
    }

    // Check if that's far enough away to want to create a new spread
    usd_t spread = (pair.sellPrice - pair.buyPrice) * conf.buffer;
    bool wantNew = first || closestDistance > spread;

    // Not far enough to consider a new pair
    if (!wantNew)
        return;

    // We need to decide if we can cancel something first
    // Try to cancel the furthest pending pair
    while (conf.numPairs <= orderPairs.size())
    {
        if (!orderPairs.cancelPending(price.getPrice()))
            break;
    }

    // We have too many orders already and we haven't been patient enough to exceed max
    if (conf.numPairs <= orderPairs.size() && !patientOverride())
        return;

    // Add the pair
    if (!orderPairs.insert(pair))
    {
        log::error("Failed to insert new order pair for spread '%s' in database.",
            conf.name.c_str());
        return;
    }

    log::trade("Created new pair for spread '%s'.", conf.name.c_str());
    stateMachine.logChange(OrderPair::State::None, pair);
}
