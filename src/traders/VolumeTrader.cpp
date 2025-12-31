#include <gtb/VolumeTrader.h>
#include <gtb/BtcPrice.h>
#include <gtb/Profits.h>
#include <gtb/CoinbaseOrderBook.h>
#include <gtb/CoinbaseWallet.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/Log.h>

using namespace gtb;

VolumeTrader::VolumeTrader(
    BotContext &ctx,
    Config config)
        : ctx(ctx)
        , conf(config)
{
    resetState();
    ctx.data.subscribe<BtcPrice>(*this);
}

void VolumeTrader::process(
    const BtcPrice &price)
{
    // Sane validate
    if (!ctx.data.get<CoinbaseInit>())
        return;
    if (price.getPrice() <= 10'000_Dollars)
        return;

    if (phase == Phase::Idle)
    {
        startBuy(price.getPrice());
        return;
    }

    CoinbaseOrder updated = ctx.data.get<CoinbaseOrderBook>().getOrder(order.uuid);
    if (!updated)
    {
        resetState();
        return;
    }

    if (updated.state == CoinbaseOrder::Filled)
    {
        handleFilled(updated, price.getPrice());
        return;
    }

    if (updated.state != CoinbaseOrder::Open)
    {
        resetState();
        return;
    }

    handleOpen(updated, price.getPrice());
}

bool VolumeTrader::startBuy(
    usd_t price)
{
    usd_t wallet = ctx.data.get<CoinbaseWallet>().getAvailUsd();
    if (wallet < conf.betSize)
        return false;

    order = CoinbaseOrder();
    order.buy = true;
    order.price = IntegerUtils::makerBuyPrice(price);
    order.quantity = IntegerUtils::getSatoshiForPrice(price, conf.betSize);
    order.createdTime = ctx.data.get<Time>().getTime();
    order.trader = conf.name;
    // Re-read availability right before submit to avoid racing other traders.
    usd_t availNow = ctx.data.get<CoinbaseWallet>().getAvailUsd();
    if (order.value() > availNow)
    {
        order = CoinbaseOrder();
        phase = Phase::Idle;
        return false;
    }
    if (!ctx.coinbase().submitOrder(order))
    {
        order = CoinbaseOrder();
        phase = Phase::Idle;
        return false;
    }

    phase = Phase::BuyPending;
    log::trade("Volume trader '%s' BUY order queued %s BTC @ %s (value %s).",
        conf.name.c_str(),
        IntegerUtils::toBtcString(order.quantity).c_str(),
        IntegerUtils::toUsdString(order.price).c_str(),
        IntegerUtils::toUsdString(order.value()).c_str());
    return true;
}

bool VolumeTrader::startSell(
    usd_t price)
{
    btc_t btc = ctx.data.get<CoinbaseWallet>().getAvailBtc();
    if (!btc || btc < pendingQty)
        return false;

    order = CoinbaseOrder();
    order.buy = false;
    order.price = IntegerUtils::makerSellPrice(price);
    order.quantity = pendingQty;
    order.createdTime = ctx.data.get<Time>().getTime();
    order.trader = conf.name;

    // Re-read availability right before submit to avoid racing other traders.
    btc_t haveNow = ctx.data.get<CoinbaseWallet>().getAvailBtc();
    if (order.quantity > haveNow)
        return false;

    if (!ctx.coinbase().submitOrder(order))
    {
        resetState();
        return false;
    }

    phase = Phase::SellPending;
    log::trade("Volume trader '%s' SELL order queued %s BTC @ %s (value %s).",
        conf.name.c_str(),
        IntegerUtils::toBtcString(order.quantity).c_str(),
        IntegerUtils::toUsdString(order.price).c_str(),
        IntegerUtils::toUsdString(order.value()).c_str());
    return true;
}

void VolumeTrader::handleFilled(
    const CoinbaseOrder &updated,
    usd_t price)
{
    if (phase == Phase::BuyPending && updated.buy)
    {
        pendingQty = updated.quantity;
        if (!startSell(price))
            resetState();
        return;
    }

    if (phase == Phase::SellPending && !updated.buy)
    {
        log::trade("Volume trader '%s' SELL filled.", conf.name.c_str());
        resetState();
        return;
    }

    resetState();
}

void VolumeTrader::handleOpen(
    const CoinbaseOrder &updated,
    usd_t price)
{
    // Buy price rose too far, cancel buy
    if (phase == Phase::BuyPending && updated.price < price - 20_Dollars)
    {
        if (ctx.coinbase().cancelOrder(updated.uuid))
            resetState();
        return;
    }

    // Sell price fell, requeue sell
    if (phase == Phase::SellPending && updated.price > price + 20_Dollars)
    {
        if (ctx.coinbase().cancelOrder(updated.uuid))
        {
            startSell(price);
        }
        return;
    }
}

void VolumeTrader::resetState()
{
    order = CoinbaseOrder();
    pendingQty = btc_t();
    phase = Phase::Idle;
}
