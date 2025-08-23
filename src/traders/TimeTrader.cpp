#include <gtb/TimeTrader.h>
#include <gtb/IntegerUtils.h>
#include <gtb/OrderPairUtils.h>
#include <gtb/OrderPairDb.h>
#include <gtb/Time.h>
#include <gtb/Uuid.h>
#include <gtb/Log.h>

using namespace gtb;

TimeTrader::TimeTrader(
    BotContext &ctx,
    Config confIn)
        : OrderPairTrader(ctx, confIn)
        , conf(std::move(confIn))
{
    startTime = 0;
    lowest = 0;
    highest = 0;
}

void TimeTrader::reset()
{
    uint32_t price = ctx.data.get<BtcPrice>().getCents();
    startTime = ctx.data.get<Time>().getTime();
    lowest = price;
    highest = price;
}

void TimeTrader::handleNewPair(
    const BtcPrice &price)
{
    // Initial values
    if (!startTime || !lowest || !highest)
    {
        reset();
        return;
    }

    // New high/lows
    if (price.getCents() > highest)
        highest = price.getCents();
    if (price.getCents() < lowest)
        lowest = price.getCents();

    // Enforce max
    if (highest > conf.maxValue)
        highest = conf.maxValue;
    if (lowest > highest)
        lowest = highest;

    // Window has not passed yet
    uint64_t time = ctx.data.get<Time>().getTime();
    if (time < startTime + (conf.seconds * 1'000'000))
        return;

    // Check if the spread was large enough
    uint32_t mid = (highest + lowest) / 2;
    uint32_t spread = ((highest - lowest) * 10'000) / mid;
    if (!spread || spread < conf.minSpread)
    {
        reset();
        return;
    }

    // Setup the pair
    OrderPair pair;
    pair.algo = conf.name;
    pair.uuid = Uuid::generate();
    pair.state = OrderPair::State::Pending;
    pair.created = time;
    uint32_t padding = ((mid * conf.paddingSpread) / 10'000) + 1;
    pair.buyPrice = lowest + padding;
    pair.sellPrice = highest - padding;
    pair.betCents = conf.cents;
    pair.quantity = IntegerUtils::getSatoshiForPrice(pair.buyPrice, pair.betCents);

    // Done with this window
    reset();

    // We need to decide if we can cancel something first
    // Try to cancel the furthest pending pair
    while (conf.numPairs >= orderPairs.size())
    {
        if (!OrderPairUtils::cancelPending(db, price.getCents(), conf.name, orderPairs))
            break;
    }

    // We have too many orders already and we haven't been patient enough to exceed max
    if (conf.numPairs <= orderPairs.size() && !patientOverride())
        return;

    // Add the pair
    if (!OrderPairDb::insert(db, pair))
    {
        log::error("Failed to insert new order pair for time trader '%s' in database.",
            conf.name.c_str());
        return;
    }

    log::info("Created new pair for time trader '%s'.", conf.name.c_str());
    stateMachine.logChange(OrderPair::State::None, pair);
    orderPairs.push_back(std::move(pair));
}

bool TimeTrader::patientOverride() const
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
