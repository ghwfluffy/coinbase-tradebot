#include <gtb/MomentumTrader.h>

#include <gtb/IntegerUtils.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/CoinbaseOrderBook.h>
#include <gtb/CoinbaseWallet.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

using namespace gtb;

MomentumTrader::MomentumTrader(
    BotContext &ctx,
    Config conf)
        : ctx(ctx)
        , conf(conf)
        , holding()
        , lastBuyPrice()
        , highWater()
        , lastAction()
        , lastExit()
        , openUuid()
{
    ctx.data.subscribe<BtcPrice>(*this);
}

void MomentumTrader::process(
    const BtcPrice &price)
{
    if (!ctx.data.get<CoinbaseInit>())
        return;
    if (price.getPrice() <= 10'000_Dollars)
        return;

    auto now = SteadyClock::now();
    prune(now);

    // Capture the pre-tick high so breakout logic compares against prior history
    // rather than the just-seen price.
    usd_t prevHigh = getHigh();
    addTick(now, price.getPrice());

    // If an order is still open, wait.
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
                lastBuyPrice = existing.price;
                highWater = existing.price;
            }
            else
            {
                holding -= existing.quantity;
                if (!holding)
                    highWater = usd_t();
                lastExit = now;
            }
        }
        openUuid.clear();
    }

    // Space out actions
    if (lastAction.time && IntegerUtils::difference(lastAction.time, now.time) < conf.minActionSpacing)
        return;

    // If holding, manage exits
    if (holding)
    {
        if (!highWater || price.getPrice() > highWater)
            highWater = price.getPrice();

        bool tp = false;
        bool sl = false;
        usd_t tpThresh = lastBuyPrice + (lastBuyPrice * conf.takeProfitPct);
        usd_t stopThresh = lastBuyPrice - (lastBuyPrice * conf.stopLossPct);
        usd_t trailThresh = highWater - (highWater * conf.trailingDrop);
        if (conf.takeProfitPct && price.getPrice() >= tpThresh)
            tp = true;
        if (conf.stopLossPct && price.getPrice() <= stopThresh)
            sl = true;
        bool trail = conf.trailingDrop && price.getPrice() <= trailThresh && price.getPrice() > lastBuyPrice;

        if (tp || sl || trail)
        {
            if (sell(price.getPrice()))
                lastAction = now;
            return;
        }
        return;
    }

    // Cooldown after exit
    if (lastExit.time && IntegerUtils::difference(lastExit.time, now.time) < conf.reentryCooldown)
        return;

    // Look for breakout
    if (!prevHigh)
        return;

    usd_t trigger = prevHigh + (prevHigh * conf.breakoutPct);
    if (price.getPrice() < trigger)
        return;

    if (buy(price.getPrice()))
        lastAction = now;
}

void MomentumTrader::prune(
    SteadyClock::TimePoint now)
{
    while (!window.empty() && IntegerUtils::difference(window.front().first.time, now.time) > conf.windowSize)
    {
        auto removed = window.front();
        window.pop_front();
        if (!maxWindow.empty() && maxWindow.front().first == removed.first)
            maxWindow.pop_front();
    }
}

usd_t MomentumTrader::getHigh() const
{
    if (maxWindow.empty())
        return usd_t();
    return maxWindow.front().second;
}

usd_t MomentumTrader::getLow() const
{
    usd_t low;
    for (const auto &p : window)
    {
        if (!low || p.second < low)
            low = p.second;
    }
    return low;
}

void MomentumTrader::addTick(
    SteadyClock::TimePoint now,
    usd_t price)
{
    window.emplace_back(now, price);

    // Maintain monotonic decreasing queue for highs
    while (!maxWindow.empty() && price >= maxWindow.back().second)
        maxWindow.pop_back();
    maxWindow.emplace_back(now, price);
}

bool MomentumTrader::buy(
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
        usd_t room = conf.capitalCap - deployed;
        if (room < conf.betSize)
            return false;
    }

    CoinbaseOrder order;
    order.buy = true;
    order.price = IntegerUtils::makerBuyPrice(price);
    order.quantity = IntegerUtils::getSatoshiForPrice(price, conf.betSize);
    order.createdTime = ctx.data.get<Time>().getTime();
    order.trader = conf.name;

    usd_t availNow = ctx.data.get<CoinbaseWallet>().getAvailUsd();
    if (order.value() > availNow)
        return false;

    if (!ctx.coinbase().submitOrder(order))
        return false;

    openUuid = order.uuid;
    log::trade("Momentum trader '%s' BUY %s BTC @ %s (value %s).",
        conf.name.c_str(),
        IntegerUtils::toBtcString(order.quantity).c_str(),
        IntegerUtils::toUsdString(order.price).c_str(),
        IntegerUtils::toUsdString(order.value()).c_str());
    return true;
}

bool MomentumTrader::sell(
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
    order.trader = conf.name;

    btc_t haveNow = ctx.data.get<CoinbaseWallet>().getAvailBtc();
    if (order.quantity > haveNow)
        return false;

    if (!ctx.coinbase().submitOrder(order))
        return false;

    openUuid = order.uuid;
    log::trade("Momentum trader '%s' SELL %s BTC @ %s (value %s).",
        conf.name.c_str(),
        IntegerUtils::toBtcString(order.quantity).c_str(),
        IntegerUtils::toUsdString(order.price).c_str(),
        IntegerUtils::toUsdString(order.value()).c_str());
    return true;
}
