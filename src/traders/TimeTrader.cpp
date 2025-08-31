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
}

void TimeTrader::reset()
{
    usd_t price = ctx.data.get<BtcPrice>().getPrice();
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
    if (price.getPrice() > highest)
        highest = price.getPrice();
    if (price.getPrice() < lowest)
        lowest = price.getPrice();

    // Enforce max
    if (highest > conf.maxValue)
        highest = conf.maxValue;
    if (lowest > highest)
        lowest = highest;

    // Window has not passed yet
    utime_t time = ctx.data.get<Time>().getTime();
    if (time < startTime + conf.sampleSize)
        return;

    // Check if the spread was large enough
    usd_t mid = IntegerUtils::avg(highest, lowest);
    pp_t spread = IntegerUtils::fraction(highest - lowest, mid);
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
    usd_t padding = mid * conf.paddingSpread;
    pair.buyPrice = lowest + padding;
    pair.sellPrice = highest - padding;
    pair.bet = conf.bet;
    pair.quantity = IntegerUtils::getSatoshiForPrice(pair.buyPrice, pair.bet);

    // Done with this window
    reset();

    // We need to decide if we can cancel something first
    // Try to cancel the furthest pending pair
    while (conf.numPairs >= orderPairs.size())
    {
        if (!OrderPairUtils::cancelPending(db, price.getPrice(), conf.name, orderPairs))
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
