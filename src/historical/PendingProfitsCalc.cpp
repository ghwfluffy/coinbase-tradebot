#include <gtb/PendingProfitsCalc.h>
#include <gtb/PendingProfits.h>
#include <gtb/IntegerUtils.h>
#include <gtb/OrderPairDb.h>
#include <gtb/Log.h>

using namespace gtb;

PendingProfitsCalc::PendingProfitsCalc(
    BotContext &ctx)
        : ctx(ctx)
{
    OrderPairDb::initDb(db);

    ctx.data.subscribe<BtcPrice>(*this);
    ctx.data.subscribe<CoinbaseOrderBook>(*this);
}

void PendingProfitsCalc::process(
    const CoinbaseOrderBook &orderbook)
{
    (void)orderbook;

    update();
}

void PendingProfitsCalc::process(
    const BtcPrice &price)
{
    (void)price;

    if (nextUpdate <= SteadyClock::now())
        update();
}

void PendingProfitsCalc::update()
{
    // TODO: This should be maintained not recalculated
#if 0
    // Get all the orders where we have bought but haven't sold
    std::list<OrderPair> orders;
    bool success = OrderPairDb::selectBought(db, orders);
    if (!success)
    {
        log::error("Failed to query orders to update pending profits.");
        return;
    }

    // Update most once every 2 minutes (or on orderbook changes)
    nextUpdate = SteadyClock::now() + std::chrono::minutes(2);

    // Sum the prices and satoshis
    usd_t spent;
    btc_t assets;
    for (const OrderPair &pair : orders)
    {
        spent += pair.bet;
        assets += pair.quantity;
    }

    // Convert satoshis to value at current market value
    usd_t value = IntegerUtils::getValue(ctx.data.get<BtcPrice>().getPrice(), assets);
    // Update data model
    ctx.data.get<PendingProfits>().setProfit(spent, value);
#endif
}
