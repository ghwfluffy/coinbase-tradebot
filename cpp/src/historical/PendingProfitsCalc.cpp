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
    ctx.data.subscribe<BtcPrice>(*this);
    ctx.data.subscribe<CoinbaseOrderBook>(*this);

    // TODO: Should have some ABI way of referencing this information
    db.init("spread_trader.sqlite", "./schema/spread_trader.sql");
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

    if (nextUpdate <= std::chrono::steady_clock::now())
        update();
}

void PendingProfitsCalc::update()
{
    // Get all the orders where we have bought but haven't sold
    std::list<OrderPair> orders;
    bool success = OrderPairDb::selectBought(db, orders);
    if (!success)
    {
        log::error("Failed to query orders to update pending profits.");
        return;
    }

    // Update most once every 2 minutes (or on orderbook changes)
    nextUpdate = std::chrono::steady_clock::now() + std::chrono::minutes(2);

    // Sum the prices and satoshis
    uint32_t spent = 0;
    uint64_t assets = 0;
    for (const OrderPair &pair : orders)
    {
        spent += pair.betCents;
        assets += pair.quantity;
    }

    // Convert satoshis to value at current market value
    uint32_t value = IntegerUtils::getValueCents(ctx.data.get<BtcPrice>().getCents(), assets);
    // Current value minus how much we spent
    int32_t delta = static_cast<int32_t>(value) - static_cast<int32_t>(spent);
    // Update data model
    ctx.data.get<PendingProfits>().setProfit(delta);
}
