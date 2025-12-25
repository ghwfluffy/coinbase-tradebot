#include <gtb/OrderPairTrader.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/Log.h>

using namespace gtb;

OrderPairTrader::OrderPairTrader(
    BotContext &ctx,
    const BaseTraderConfig &conf)
        : ctx(ctx)
        , conf(conf)
        , stateMachine(ctx, conf)
{
    loadDatabase();

    ctx.data.subscribe<BtcPrice>(*this);
    ctx.data.subscribe<CoinbaseOrderBook>(*this);
}

void OrderPairTrader::loadDatabase()
{
    orderPairs.init(conf.name);
    log::info("Read %zu pairs for trader '%s' from database.",
        orderPairs.size(),
        conf.name.c_str());
}

void OrderPairTrader::process(
    const BtcPrice &price)
{
    if (!ctx.data.get<CoinbaseInit>())
        return;

    // Sane validate
    if (price.getPrice() < 10'000_Dollars)
        return;

    std::lock_guard<std::mutex> lock(mtx);

    // Check if we need to do anything for the existing pairs
    handleExistingPairs();

    // Check if we want to create a new pair (which might make us drop a pending one)
    handleNewPair(price);
}

void OrderPairTrader::process(
    const CoinbaseOrderBook &orderbook)
{
    (void)orderbook;

    // Check state of each pair
    // Force querying order information from Coinbase (don't trust orderbook cache)
    std::lock_guard<std::mutex> lock(mtx);
    handleExistingPairs(true);
}

void OrderPairTrader::handleExistingPairs(
    bool force)
{
    // Handle each order pair
    for (auto [uuid, pair] : orderPairs.getPairs())
    {
        if (stateMachine.churn(pair, force))
            orderPairs.update(pair);
    }

    // Stop tracking completed states
    std::vector<OrderPair> completed = orderPairs.popComplete();
    for (OrderPair &pair : completed)
        handleComplete(pair);
}

void OrderPairTrader::handleComplete(
    OrderPair &pair)
{
    (void)pair;
}

// If we've queued our maximum number of pairs,
// and there is a patienceOverride configured,
// then we can exceed our maximum number of pairs
// if we are patient enough to wait the override period
// since the last pair was queued
bool OrderPairTrader::patientOverride() const
{
    if (!conf.patienceOverride)
        return false;

    return orderPairs.patientOverride(conf.patienceOverride, ctx.data.get<Time>().getTime());
}
