#include <gtb/StaticTrader.h>
#include <gtb/IntegerUtils.h>
#include <gtb/OrderPairDb.h>
#include <gtb/Time.h>
#include <gtb/Uuid.h>
#include <gtb/Log.h>

using namespace gtb;

StaticTrader::StaticTrader(
    BotContext &ctx,
    Config confIn)
        : OrderPairTrader(ctx, confIn.name)
        , conf(std::move(confIn))
{
}

void StaticTrader::handleNewPair(
    const BtcPrice &price)
{
    (void)price;

    // We already have an order pair created
    if (!orderPairs.empty())
        return;

    // Add a new order pair for our static buy price
    OrderPair pair;
    pair.algo = conf.name;
    pair.uuid = Uuid::generate();
    pair.state = OrderPair::State::Pending;
    pair.created = ctx.data.get<Time>().getTime();
    pair.buyPrice = conf.buyCents;
    pair.sellPrice = conf.sellCents;
    pair.betCents = conf.cents;
    pair.quantity = IntegerUtils::getSatoshiForPrice(pair.buyPrice, pair.betCents);

    // Add the pair
    if (!OrderPairDb::insert(db, pair))
    {
        log::error("Failed to insert new order pair for static trader '%s' in database.",
            conf.name.c_str());
        return;
    }

    log::info("Created new pair for static trader '%s'.", conf.name.c_str());
    stateMachine.logChange(OrderPair::State::None, pair);
    orderPairs.push_back(std::move(pair));
}
