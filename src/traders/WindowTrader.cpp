#include <gtb/WindowTrader.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/IntegerUtils.h>
#include <gtb/Profits.h>
#include <gtb/CoinbaseOrderBook.h>
#include <gtb/Log.h>

using namespace gtb;

WindowTrader::Candle::Candle()
{
    start = SteadyClock::now();
}

WindowTrader::WindowTrader(
    BotContext &ctx,
    Config config)
        : ctx(ctx)
        , conf(config)
{
    ctx.data.subscribe<BtcPrice>(*this);
}

void WindowTrader::process(
    const BtcPrice &price)
{
    // Sane validate
    if (!ctx.data.get<CoinbaseInit>())
        return;
    if (price.getPrice() <= 10'000_Dollars)
        return;

    // Check/Update candle
    checkCandle(price);

    // Not enough data yet
    if (candles.size() < 2)
        return;

    // Do nothing during a pause period
    requeueSale(price);
    if (isPaused())
        return;

    // If we fall below the market fire sale and pause
    if (checkMarketBottom(price))
        return;

    handleBuy(price);
    handleSell(price);
}

void WindowTrader::checkCandle(
    const BtcPrice &price)
{
    // Time for next candle
    if (candles.empty() || (candles.back().start.time + conf.candleSize) < SteadyClock::now().time)
        candles.push_back(Candle());

    // Update min/max for this candle
    Candle &c = candles.back();
    if (!c.minPrice || c.minPrice > price.getPrice())
        c.minPrice = price.getPrice();

    if (!c.maxPrice || c.maxPrice < price.getPrice())
        c.maxPrice = price.getPrice();

    // Too many candles
    if (candles.size() > utime_t(conf.windowSize / conf.candleSize).value())
        candles.erase(candles.begin());
}

bool WindowTrader::isPaused() const
{
    return pauseTimer > SteadyClock::now();
}

bool WindowTrader::fireSale(
    const BtcPrice &price)
{
    btc_t btc = ctx.data.get<CoinbaseWallet>().getAvailBtc();
#if 0
    if (btc > holding)
        btc = holding;
#endif
    if (!btc)
    {
        // TODO: Subroutine window reset
        totalSpent = BigInt();
        totalPurchased = BigInt();

        while (candles.size() > 1)
            candles.erase(candles.begin());

        return true;
    }
    // Already fire saling
    if (!fireSaleUuid.empty())
        return true;

    // Try to sell everything we got
    CoinbaseOrder order;
    order.buy = false;
    order.price = price.getPrice() + 2_Dollars;
    order.quantity = btc;
    order.createdTime = ctx.data.get<Time>().getTime();
    if (ctx.coinbase().submitOrder(order))
    {
        log::trade("Window trader '%s' fire sale %s BTC.",
            conf.name.c_str(),
            IntegerUtils::toBtcString(btc).c_str());
        holding = btc_t();

        fireSaleUuid = order.uuid;
        ctx.data.get<Profits>().addOrderPair(usd_t(), order.value(), usd_t(), usd_t());
        return true;
    }

    return false;
}

void WindowTrader::pauseTrading()
{
    log::info("Window trader '%s' pausing trading.", conf.name.c_str());
    pauseTimer = SteadyClock::now() + std::chrono::minutes(conf.pauseDuration / 1_Minutes);
    prevActionPrice = usd_t();
}

bool WindowTrader::checkMarketBottom(
    const BtcPrice &price)
{
    // We have fallen below the window minimum
    if (price.getPrice() < getFireMin())
    {
        if (fireSale(price))
            pauseTrading();
        return true;
    }

    return false;
}

usd_t WindowTrader::getFireMin() const
{
    usd_t minPrice;
    for (size_t ui = 0; ui < candles.size() / 2; ui++)
    {
        const Candle &c = candles[ui];
        if (!minPrice || minPrice > c.minPrice)
            minPrice = c.minPrice;
    }

    usd_t buffer = getWindowSize() * conf.fireWindowBuffer;
    minPrice -= buffer;
    return minPrice;
}

usd_t WindowTrader::getWindowMin() const
{
    usd_t minPrice;
    for (size_t ui = 0; ui < candles.size() / 2; ui++)
    {
        const Candle &c = candles[ui];
        if (!minPrice || minPrice > c.minPrice)
            minPrice = c.minPrice;
    }

    usd_t buffer = getWindowSize() * conf.lowWindowBuffer;
    minPrice -= buffer;
    return minPrice;
}

usd_t WindowTrader::getWindowMax() const
{
    usd_t maxPrice;
    for (size_t ui = 0; ui < candles.size() / 2; ui++)
    {
        const Candle &c = candles[ui];
        if (!maxPrice || maxPrice < c.maxPrice)
            maxPrice = c.maxPrice;
    }

    usd_t buffer = getWindowSize() * conf.highWindowBuffer;
    maxPrice -= buffer;
    return maxPrice;
}

usd_t WindowTrader::getWindowSize() const
{
    usd_t minPrice;
    usd_t maxPrice;
    for (size_t ui = 0; ui < candles.size() / 2; ui++)
    {
        const Candle &c = candles[ui];
        if (!maxPrice || maxPrice < c.maxPrice)
            maxPrice = c.maxPrice;
        if (!minPrice || minPrice > c.minPrice)
            minPrice = c.minPrice;
    }
    return maxPrice - minPrice;
}

void WindowTrader::handleBuy(
    const BtcPrice &price)
{
#if 0
    // Not enough change
    if (prevActionPrice && IntegerUtils::difference(prevActionPrice, price.getPrice()) < conf.buyDelta)
        return;
#else
    // Too soon
    if (prevActionTime.time && IntegerUtils::difference(prevActionTime.time, SteadyClock::now().time) < conf.sellFrequency)
        return;
#endif

    // Max invested
    if (conf.spendLimit && conf.spendLimit <= IntegerUtils::getValue(price.getPrice(), holding))
        return;

    // Don't increase our buy average
    if (totalSpent && totalPurchased)
    {
        usd_t avgPrice = IntegerUtils::getPrice(totalSpent, totalPurchased);
        if (avgPrice && price.getPrice() > avgPrice + conf.buyDelta)
            return;
    }

    // Not in the buy window
    if (price.getPrice() < getWindowMin() || price.getPrice() > getWindowMax())
        return;

    // Place another bet
    CoinbaseOrder order;
    order.buy = true;
    order.price = price.getPrice() - 2_Dollars;
    order.quantity = IntegerUtils::getSatoshiForPrice(price.getPrice(), conf.betSize);
    order.createdTime = ctx.data.get<Time>().getTime();

    // Enough money?
    usd_t wallet = ctx.data.get<CoinbaseWallet>().getAvailUsd();
    if (order.value() > wallet - 20_Dollars)
        return;

    // TODO: Always assumes success
    if (ctx.coinbase().submitOrder(order))
    {
        setAction(price);
        holding += order.quantity;
#if 0
        Candle &c = candles.back();
        c.totalSpent += conf.betSize;
        c.totalPurchased += order.quantity;
#else
        totalSpent += BigInt(order.value().value());
        totalPurchased += BigInt(order.quantity.value());
#if 0
        log::info("%s|%s|%s",
            IntegerUtils::toUsdString(price.getPrice()).c_str(),
            IntegerUtils::toUsdString(order.value()).c_str(),
            IntegerUtils::toBtcString(order.quantity).c_str());
#endif
#endif

        ctx.data.get<Profits>().addOrderPair(order.value(), usd_t(), usd_t(), usd_t());
        log::trade("Window trader '%s' BUY %s BTC @ %s (value %s).",
            conf.name.c_str(),
            IntegerUtils::toBtcString(order.quantity).c_str(),
            IntegerUtils::toUsdString(order.price).c_str(),
            IntegerUtils::toUsdString(order.value()).c_str());
    }
}

void WindowTrader::setAction(
    const BtcPrice &price)
{
    prevActionTime = SteadyClock::now();
    prevActionPrice = price.getPrice();
}

void WindowTrader::handleSell(
    const BtcPrice &price)
{
    // Too soon
    if (prevActionTime.time && IntegerUtils::difference(prevActionTime.time, SteadyClock::now().time) < conf.sellFrequency)
        return;

    // Nothing to sell
    if (!holding)
        return;

    if (!totalSpent)
        return;
    if (!totalPurchased)
        return;

#if 0
    // Calculate the average buy price from the previous candles
    usd_t totalSpent;
    btc_t totalPurchased;
    for (size_t ui = 0; ui < conf.avgBuyPriceCandleCount; ui++)
    {
        if (ui + 1 >= candles.size())
            break;
        const Candle &c = candles[candles.size() - ui - 1];
        totalSpent += c.totalSpent;
        totalPurchased += c.totalPurchased;
    }
#endif

    usd_t avgPrice = IntegerUtils::getPrice(totalSpent, totalPurchased);
    if (!avgPrice)
        return;
#if 0
    if (!avgPrice)
    {
        log::error("Failed to calculate average buy price");
        return;
    }
#endif

    if (price.getPrice() < (avgPrice + conf.takeProfitDelta))
    {
        //log::info("%s|%s", IntegerUtils::toUsdString(price.getPrice()).c_str(), IntegerUtils::toUsdString(avgPrice).c_str());
        return;
    }

    btc_t amount = IntegerUtils::getSatoshiForPrice(avgPrice, conf.betSize);
    if (amount > holding)
        amount = holding;

    btc_t have = ctx.data.get<CoinbaseWallet>().getAvailBtc();
    if (!have)
        return;
    if (amount > have)
        amount = have;

    CoinbaseOrder order;
    order.buy = false;
    order.price = price.getPrice() + 2_Dollars;
    order.quantity = amount;
    order.createdTime = ctx.data.get<Time>().getTime();
    // TODO: Always assumes success
    if (ctx.coinbase().submitOrder(order))
    {
        setAction(price);
        holding -= amount;
        ctx.data.get<Profits>().addOrderPair(usd_t(), order.value(), usd_t(), usd_t());
        log::trade("Window trader '%s' SELL %s BTC @ %s (value %s).",
            conf.name.c_str(),
            IntegerUtils::toBtcString(order.quantity).c_str(),
            IntegerUtils::toUsdString(order.price).c_str(),
            IntegerUtils::toUsdString(order.value()).c_str());

        // TODO: Adjusting buy price on sale?
        totalSpent += BigInt(order.value().value() / 2);
        totalPurchased += BigInt(order.quantity.value() / 2);
    }
}

void WindowTrader::requeueSale(
    const BtcPrice &price)
{
    if (fireSaleUuid.empty())
        return;

    CoinbaseOrder order = ctx.data.get<CoinbaseOrderBook>().getOrder(fireSaleUuid);
    if (!order || order.state == CoinbaseOrder::Filled)
    {
        log::trade("Window trader '%s' fire sale complete.", conf.name.c_str());
        fireSaleUuid.clear();

        totalSpent = BigInt();
        totalPurchased = BigInt();

        while (candles.size() > 1)
            candles.erase(candles.begin());
        return;
    }

    // This order is still good
    if (order.state == CoinbaseOrder::Open && order.price < price.getPrice() + 2_Dollars)
        return;

    // Cancel open order
    if (order.state == CoinbaseOrder::Open && !ctx.coinbase().cancelOrder(fireSaleUuid))
        return;

    // Add back to holding
    holding += order.quantity;
    // Requeue
    fireSaleUuid.clear();
    if (isPaused())
        fireSale(price);
}
