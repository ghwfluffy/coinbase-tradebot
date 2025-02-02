#include <gtb/OrderPairTrader.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/OrderPairDb.h>
#include <gtb/Log.h>

using namespace gtb;

OrderPairTrader::OrderPairTrader(
    BotContext &ctx,
    std::string nameIn)
        : ctx(ctx)
        , name(std::move(nameIn))
        , stateMachine(ctx, db, name)
{
    loadDatabase();

    ctx.data.subscribe<BtcPrice>(*this);
    ctx.data.subscribe<CoinbaseOrderBook>(*this);
}

void OrderPairTrader::loadDatabase()
{
    OrderPairDb::initDb(db);
    orderPairs = OrderPairDb::select(db, name);
    log::info("Read %zu pairs for trader '%s' from database.",
        orderPairs.size(),
        name.c_str());
}

void OrderPairTrader::process(
    const BtcPrice &price)
{
    if (!ctx.data.get<CoinbaseInit>())
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
    stateMachine.churn(orderPairs, force);

    // Stop tracking completed states
    auto iter = orderPairs.begin();
    while (iter != orderPairs.end())
    {
        OrderPair &pair = (*iter);

        if (pair.state >= OrderPair::State::Complete)
        {
            log::info("Removing completed order pair '%s' from trader '%s'.",
                pair.uuid.c_str(),
                name.c_str());
            iter = orderPairs.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}
