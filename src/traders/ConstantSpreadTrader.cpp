#include <gtb/ConstantSpreadTrader.h>

#include <gtb/OrderPairMarketEngine.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

using namespace gtb;

ConstantSpreadTrader::ConstantSpreadTrader(
    BotContext &ctx,
    Config confIn)
        : OrderPairTrader(ctx, confIn)
        , conf(std::move(confIn))
{
    lastBuyPrice = 0_Dollars;
}

void ConstantSpreadTrader::handleNewPair(
    const BtcPrice &price)
{
    if (!(orderPairs.size() == 0 ||
         (price.getPrice() < lastBuyPrice && (lastBuyPrice - price.getPrice() > conf.windowSize))))
    {
        return;
    }

    // Setup the pair
    OrderPair pair = OrderPairMarketEngine::newStatic(
        conf,
        ctx.data.get<Time>().getTime(),
        price.getPrice(),
        price.getPrice() + conf.windowSize
    );
    if (!pair)
        return;

#if 0
    if (checkMax())
        return;
#endif

    // Add the pair
    if (!orderPairs.insert(pair))
    {
        log::error("Failed to insert new order pair for spread '%s' in database.",
            conf.name.c_str());
        return;
    }

    log::trade("Created new pair for const spread '%s'.", conf.name.c_str());
    lastBuyPrice = pair.buyPrice;
    stateMachine.logChange(OrderPair::State::None, pair);
}

void ConstantSpreadTrader::handleComplete(
    OrderPair &pair)
{
    // Track a maker-friendly threshold before opening the next buy.
    lastBuyPrice = IntegerUtils::makerBuyPrice(pair.sellPrice, 2_Dollars + (conf.windowSize * 2));
}
