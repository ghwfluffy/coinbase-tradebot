#include <gtb/VolumeTrader.h>
#include <gtb/BtcPrice.h>
#include <gtb/CoinbaseOrderBook.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/Time.h>
#include <gtb/Log.h>
#include <gtb/Uuid.h>

using namespace gtb;

namespace
{

BaseTraderConfig toBaseConf(
    const VolumeTrader::Config &conf)
{
    BaseTraderConfig baseConf;
    baseConf.name = conf.name;
    baseConf.bet = conf.betSize;
    baseConf.maxValue = 200'000_Dollars;
    baseConf.pendingPairExpiration = conf.orderTtl;
    baseConf.patienceOverride = 0_Seconds;
    return baseConf;
}

}

VolumeTrader::VolumeTrader(
    BotContext &ctx,
    Config config)
        : ctx(ctx)
        , conf(config)
        , stateMachine(ctx, toBaseConf(config))
{
    resetState();
    ctx.data.subscribe<BtcPrice>(*this);
}

void VolumeTrader::process(
    const BtcPrice &price)
{
    if (!ctx.data.get<CoinbaseInit>())
        return;
    if (price.getPrice() <= 10'000_Dollars)
        return;

    ensurePair(price.getPrice());

    // Progress pair via state machine
    OrderPair::State beforeState = pair.state;
    stateMachine.churn(pair, false);
    if (pair.state > OrderPair::State::Pending && beforeState != pair.state)
        orderCreated = SteadyClock::now();

    manageBuyCancel(price);
    manageSellReprice(price);
    manageHoldingDecay(price);
}

void VolumeTrader::ensurePair(
    usd_t price)
{
    if (pair.state == OrderPair::State::None ||
        pair.state == OrderPair::State::Pending ||
        pair.state == OrderPair::State::Complete ||
        pair.state == OrderPair::State::Canceled ||
        pair.state == OrderPair::State::Error)
    {
        pair = OrderPair();
        pair.uuid = Uuid::generate();
        pair.algo = conf.name;
        pair.bet = conf.betSize;
        pair.buyPrice = price;
        pair.sellPrice = price + conf.minProfitDelta;
        pair.state = OrderPair::State::Pending;
        pair.created = ctx.data.get<Time>().getTime();
        pair.nextTry = SteadyClock::now();
        orderCreated = SteadyClock::TimePoint();
    }
}

void VolumeTrader::manageBuyCancel(
    const BtcPrice &price)
{
    if (pair.state != OrderPair::State::BuyActive)
        return;

    // Passed TTL yet?
    if (conf.orderTtl && orderCreated.time)
    {
        bool expired = SteadyClock::now() > orderCreated + std::chrono::seconds(conf.orderTtl / 1_Seconds);
        if (!expired)
            return;
    }

    // Outside of reprice band?
    if (pair.buyPrice >= price.getPrice() - conf.repriceBand)
        return;

    // Cancel buy and reset state
    if (canCancel(pair.buyOrder) && ctx.coinbase().cancelOrder(pair.buyOrder))
        resetState();
}

void VolumeTrader::manageSellReprice(
    const BtcPrice &price)
{
    if (pair.state != OrderPair::State::SellActive)
        return;

    // Passed TTL yet?
    if (conf.orderTtl && orderCreated.time)
    {
        bool expired = SteadyClock::now() > orderCreated + std::chrono::seconds(conf.orderTtl / 1_Seconds);
        if (!expired)
            return;
    }

    // Get order info
    CoinbaseOrderBook &book = ctx.data.get<CoinbaseOrderBook>();
    CoinbaseOrder sellOrder = book.getOrder(pair.sellOrder);
    if (!sellOrder)
        return;

    // Outside of reprice band?
    if (sellOrder.price <= price.getPrice() + conf.repriceBand)
        return;

    // Move back to holding so the state machine posts a fresh sell
    if (canCancel(pair.sellOrder) && ctx.coinbase().cancelOrder(pair.sellOrder))
    {
        pair.state = OrderPair::State::Holding;
        pair.sellOrder.clear();
        orderCreated = SteadyClock::TimePoint();
        manageHoldingDecay(price);
    }
}

void VolumeTrader::manageHoldingDecay(
    const BtcPrice &price)
{
    if (pair.state != OrderPair::State::Holding)
        return;

    // Passed TTL yet?
    if (conf.orderTtl && orderCreated.time)
    {
        bool expired = SteadyClock::now() > orderCreated + std::chrono::seconds(conf.orderTtl / 1_Seconds);
        if (!expired)
            return;
    }

    pair.sellPrice = price.getPrice() + conf.minProfitDelta;
    orderCreated = SteadyClock::now();
}

bool VolumeTrader::canCancel(
    const std::string &uuid) const
{
    if (uuid.empty())
        return false;
    CoinbaseOrder current = ctx.data.get<CoinbaseOrderBook>().getOrder(uuid);
    return current && current.state == CoinbaseOrder::State::Open;
}

void VolumeTrader::resetState()
{
    pair = OrderPair();
    orderCreated = SteadyClock::TimePoint();
}
