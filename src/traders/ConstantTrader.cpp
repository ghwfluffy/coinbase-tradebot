#include <gtb/ConstantTrader.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/Profits.h>

#include <gtb/Log.h>
#include <gtb/IntegerUtils.h>

using namespace gtb;

ConstantTrader::ConstantTrader(
    BotContext &ctx,
    usd_t betSize,
    usd_t windowSize)
        : ctx(ctx)
        , betSize(betSize)
        , windowSize(windowSize)
{
    lastBuyPrice = 0_Dollars;
    buyCount = 0;
    sellCount = 0;
    maxWallet = 0_Dollars;

    ctx.data.subscribe<BtcPrice>(*this);
}

void ConstantTrader::process(
    const BtcPrice &price)
{
    // Sane validate
    if (!ctx.data.get<CoinbaseInit>())
        return;
    if (price.getPrice() <= 10'000_Dollars)
        return;

    std::lock_guard<std::mutex> lock(mtx);

    if (checkMax())
        return;

    if (sellPrices.empty() ||
        (price.getPrice() < lastBuyPrice && (lastBuyPrice - price.getPrice() > windowSize)))
    {
        queueBuy(price);
    }

    // Have we exceeded any sell prices?
    size_t count = 0;
    auto iter = sellPrices.begin();
    while (iter != sellPrices.end())
    {
        usd_t sellPrice = *iter;
        if (sellPrice > price.getPrice())
            break;

        iter = sellPrices.erase(iter);
        ++count;
    }

    for (size_t ui = 0; ui < count; ui++)
        queueSell(price);
}

void ConstantTrader::queueBuy(
    const BtcPrice &price)
{
    if (ctx.data.get<CoinbaseWallet>().getAvailUsd() < betSize)
        return;

    // Try to place new order
    CoinbaseOrder order;
    order.buy = true;
    order.setQuantity(price.getPrice() - 2_Dollars, betSize);
    order.createdTime = ctx.data.get<Time>().getTime();
    if (ctx.coinbase().submitOrder(order))
    {
        lastBuyPrice = price.getPrice();
        sellPrices.insert(price.getPrice() + windowSize);
#if 0
        if (buyCount++ % 1000 == 0)
            log::info("Buy %s", IntegerUtils::toUsdString(lastBuyPrice).c_str());
#endif
        // XXX: On complete
        ctx.data.get<Profits>().addOrderPair(order.value(), 0_Dollars, 0_Dollars, 0_Dollars);
    }
}

void ConstantTrader::queueSell(
    const BtcPrice &price)
{
    btc_t haveBtc = ctx.data.get<CoinbaseWallet>().getAvailBtc();
    if (haveBtc <= 0_Satoshi)
        return;

    usd_t sellPrice = price.getPrice() + 2_Dollars;
    usd_t estBuyPrice = price.getPrice() - windowSize;
    btc_t quantity = IntegerUtils::getSatoshiForPrice(estBuyPrice, betSize);
    if (quantity > haveBtc)
        quantity = haveBtc;

    // Try to place new order
    CoinbaseOrder order;
    order.buy = false;
    order.price = sellPrice;
    order.quantity = quantity;
    order.createdTime = ctx.data.get<Time>().getTime();
    if (ctx.coinbase().submitOrder(order))
    {
        lastBuyPrice = sellPrice - (windowSize * 2);
#if 0
        if (sellCount++ % 1000 == 0)
            log::info("Sell %s", IntegerUtils::toUsdString(sellPrice).c_str());
#endif
        // XXX: On complete
        ctx.data.get<Profits>().addOrderPair(0_Dollars, order.value(), 0_Dollars, 0_Dollars);
    }
}

bool ConstantTrader::checkMax()
{
    // Paused
    if (SteadyClock::now() < pauseTimer)
        return true;

    if (sellPrices.empty())
        return false;

    usd_t price = ctx.data.get<BtcPrice>().getPrice();

    usd_t usd = ctx.data.get<CoinbaseWallet>().getUsd();
    btc_t btc = ctx.data.get<CoinbaseWallet>().getBtc();

    if (btc <= 0_Satoshi)
        return false;

    usd_t btcValue = IntegerUtils::getValue(price, btc);

    usd_t curWallet = usd + btcValue;
    if (curWallet > maxWallet + 100_Dollars)
    {
        maxWallet = curWallet;
        printf("New max: %s\n", IntegerUtils::toUsdString(maxWallet).c_str());

        // Try to sell everything we got
        CoinbaseOrder order;
        order.buy = false;
        order.price = price + 2_Dollars;
        order.quantity = btc; // TODO: Cancel
        order.createdTime = ctx.data.get<Time>().getTime();
        if (ctx.coinbase().submitOrder(order))
        {
            pauseTimer = SteadyClock::now() + std::chrono::minutes(30);
            sellPrices.clear();
        }

        return true;
    }

    return false;
}
