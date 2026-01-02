#include <gtb/HodlTrader.h>

#include <gtb/CoinbaseInit.h>
#include <gtb/CoinbaseOrder.h>
#include <gtb/CoinbaseOrderBook.h>
#include <gtb/CoinbaseWallet.h>
#include <gtb/IntegerUtils.h>
#include <gtb/Time.h>

using namespace gtb;

namespace
{

constexpr utime_t ONE_DAY = 24_Hours;

}

HodlTrader::HodlTrader(
    BotContext &ctx,
    Config conf)
        : ctx(ctx)
        , conf(std::move(conf))
        , currentLow()
        , previousLow()
        , currentDay()
        , purchasedToday(false)
{
    ctx.data.subscribe<BtcPrice>(*this);
}

void HodlTrader::process(
    const BtcPrice &price)
{
    if (!ctx.data.get<CoinbaseInit>())
        return;
    if (price.getPrice() <= 10'000_Dollars)
        return;

    utime_t now = ctx.data.get<Time>().getTime();
    utime_t dayStart = getDayStart(now);

    if (!currentDay || dayStart != currentDay)
        rollDay(price.getPrice(), dayStart);

    if (!currentLow || price.getPrice() < currentLow)
        currentLow = price.getPrice();

    updateOrderState();

    if (!hasPrevDayLow() ||
        purchasedToday ||
        !openOrder.empty() ||
        price.getPrice() > previousLow)
    {
        return;
    }

    if (ctx.data.get<CoinbaseWallet>().getAvailUsd() < conf.betSize)
        return;

    CoinbaseOrder order;
    order.buy = true;
    order.price = IntegerUtils::makerBuyPrice(previousLow);
    order.quantity = IntegerUtils::getSatoshiForPrice(order.price, conf.betSize);
    order.createdTime = now;
    order.trader = conf.name;

    if (ctx.coinbase().submitOrder(order))
    {
        purchasedToday = true;
        openOrder = order.uuid;
    }
}

void HodlTrader::rollDay(
    usd_t price,
    utime_t dayStart)
{
    if (!!currentLow)
        previousLow = currentLow;
    currentLow = price;
    currentDay = dayStart;
    purchasedToday = false;
}

void HodlTrader::updateOrderState()
{
    if (openOrder.empty())
        return;

    CoinbaseOrder order = ctx.data.get<CoinbaseOrderBook>().getOrder(openOrder);
    if (!order)
    {
        openOrder.clear();
        return;
    }

    if (order.state == CoinbaseOrder::State::Filled)
    {
        openOrder.clear();
    }
    else if (order.state == CoinbaseOrder::State::Canceled ||
        order.state == CoinbaseOrder::State::Error)
    {
        openOrder.clear();
    }
}

bool HodlTrader::hasPrevDayLow() const
{
    return !!previousLow;
}

utime_t HodlTrader::getDayStart(
    utime_t now) const
{
    uint64_t day = now.value() / ONE_DAY.value();
    return utime_t(day * ONE_DAY.value());
}
