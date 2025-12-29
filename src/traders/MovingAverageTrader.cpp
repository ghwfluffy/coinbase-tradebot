#include <gtb/MovingAverageTrader.h>

#include <gtb/IntegerUtils.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/CoinbaseOrderBook.h>
#include <gtb/CoinbaseWallet.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

using namespace gtb;

MovingAverageTrader::MovingAverageTrader(
    BotContext &ctx,
    Config conf)
        : ctx(ctx)
        , conf(conf)
        , holding()
        , lastBuy()
        , highWater()
        , lastAction()
        , openUuid()
{
    ctx.data.subscribe<BtcPrice>(*this);
}

void MovingAverageTrader::process(
    const BtcPrice &price)
{
    if (!ctx.data.get<CoinbaseInit>())
        return;
    if (price.getPrice() <= 10'000_Dollars)
        return;

    checkCandle(price);

    // Wait for open order to resolve
    if (!openUuid.empty())
    {
        CoinbaseOrder existing = ctx.data.get<CoinbaseOrderBook>().getOrder(openUuid);
        if (existing && existing.state == CoinbaseOrder::Open)
            return;
        if (existing && existing.state == CoinbaseOrder::Filled)
        {
            if (existing.buy)
            {
                holding += existing.quantity;
                lastBuy = existing.price;
                highWater = existing.price;
            }
            else
            {
                if (holding >= existing.quantity)
                    holding -= existing.quantity;
                else
                    holding = btc_t();
                if (!holding)
                    highWater = usd_t();
            }
        }
        openUuid.clear();
    }

    auto now = SteadyClock::now();
    if (lastAction.time && IntegerUtils::difference(lastAction.time, now.time) < conf.minSpacing)
        return;

    double shortAvg = getShortAvg();
    double longAvg = getLongAvg();
    if (longAvg <= 0.0)
        return;

    usd_t cur = price.getPrice();

    if (holding)
    {
        if (!highWater || cur > highWater)
            highWater = cur;
        usd_t tp = lastBuy + (lastBuy * conf.takeProfitPct);
        usd_t sl = lastBuy - (lastBuy * conf.stopLossPct);
        usd_t trail = highWater - (highWater * conf.trailingDrop);

        bool crossDown = shortAvg < longAvg * (1.0 - (static_cast<double>(conf.exitBuffer.value()) / PP_SCALE));
        bool takeProfit = conf.takeProfitPct && cur >= tp;
        bool stop = conf.stopLossPct && cur <= sl;
        bool trailing = conf.trailingDrop && cur <= trail && cur > lastBuy;

        if (crossDown || takeProfit || stop || trailing)
        {
            if (sell(cur))
                lastAction = now;
            return;
        }
        return;
    }

    // Entry
    bool crossUp = shortAvg > longAvg * (1.0 + (static_cast<double>(conf.entryBuffer.value()) / PP_SCALE));
    if (crossUp && buy(cur))
        lastAction = now;
}

void MovingAverageTrader::checkCandle(
    const BtcPrice &price)
{
    auto now = SteadyClock::now();
    if (candles.empty() || (candles.back().start.time + conf.candleSize) < now.time)
    {
        Candle c;
        c.start = now;
        c.closePrice = price.getPrice();
        candles.push_back(c);
    }
    else
    {
        candles.back().closePrice = price.getPrice();
    }

    while (candles.size() > conf.longWindow + 2)
        candles.pop_front();
}

double MovingAverageTrader::getShortAvg() const
{
    if (candles.size() < conf.shortWindow)
        return 0.0;
    uint64_t sum = 0;
    size_t count = 0;
    for (auto it = candles.rbegin(); it != candles.rend() && count < conf.shortWindow; ++it, ++count)
        sum += it->closePrice.value();
    return static_cast<double>(sum) / static_cast<double>(count);
}

double MovingAverageTrader::getLongAvg() const
{
    if (candles.size() < conf.longWindow)
        return 0.0;
    uint64_t sum = 0;
    size_t count = 0;
    for (auto it = candles.rbegin(); it != candles.rend() && count < conf.longWindow; ++it, ++count)
        sum += it->closePrice.value();
    return static_cast<double>(sum) / static_cast<double>(count);
}

bool MovingAverageTrader::buy(
    usd_t price)
{
    usd_t avail = ctx.data.get<CoinbaseWallet>().getAvailUsd();
    if (avail < conf.betSize)
        return false;
    if (conf.capitalCap)
    {
        usd_t deployed = IntegerUtils::getValue(price, holding);
        if (deployed >= conf.capitalCap)
            return false;
        if (deployed + conf.betSize > conf.capitalCap)
            return false;
    }

    CoinbaseOrder order;
    order.buy = true;
    order.price = IntegerUtils::makerBuyPrice(price);
    order.quantity = IntegerUtils::getSatoshiForPrice(price, conf.betSize);
    order.createdTime = ctx.data.get<Time>().getTime();

    usd_t availNow = ctx.data.get<CoinbaseWallet>().getAvailUsd();
    if (order.value() > availNow)
        return false;

    if (!ctx.coinbase().submitOrder(order))
        return false;

    openUuid = order.uuid;
    log::trade("MovingAvg trader '%s' BUY %s BTC @ %s (value %s).",
        conf.name.c_str(),
        IntegerUtils::toBtcString(order.quantity).c_str(),
        IntegerUtils::toUsdString(order.price).c_str(),
        IntegerUtils::toUsdString(order.value()).c_str());
    return true;
}

bool MovingAverageTrader::sell(
    usd_t price)
{
    btc_t have = ctx.data.get<CoinbaseWallet>().getAvailBtc();
    if (!have || !holding)
        return false;
    btc_t amount = holding;
    if (amount > have)
        amount = have;
    if (!amount)
        return false;

    CoinbaseOrder order;
    order.buy = false;
    order.price = IntegerUtils::makerSellPrice(price);
    order.quantity = amount;
    order.createdTime = ctx.data.get<Time>().getTime();

    btc_t haveNow = ctx.data.get<CoinbaseWallet>().getAvailBtc();
    if (order.quantity > haveNow)
        return false;

    if (!ctx.coinbase().submitOrder(order))
        return false;

    openUuid = order.uuid;
    log::trade("MovingAvg trader '%s' SELL %s BTC @ %s (value %s).",
        conf.name.c_str(),
        IntegerUtils::toBtcString(order.quantity).c_str(),
        IntegerUtils::toUsdString(order.price).c_str(),
        IntegerUtils::toUsdString(order.value()).c_str());
    return true;
}
