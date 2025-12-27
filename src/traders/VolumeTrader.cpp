#include <gtb/VolumeTrader.h>
#include <gtb/BtcPrice.h>
#include <gtb/Profits.h>
#include <gtb/CoinbaseOrderBook.h>
#include <gtb/CoinbaseWallet.h>
#include <gtb/CoinbaseInit.h>

using namespace gtb;

VolumeTrader::VolumeTrader(
    BotContext &ctx,
    Config config)
        : ctx(ctx)
        , conf(config)
{
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

    // Queue purchase
    if (order.uuid.empty())
    {
        // Enough money?
        usd_t wallet = ctx.data.get<CoinbaseWallet>().getAvailUsd();
        if (wallet < conf.betSize)
            return;

        order.buy = true;
        order.price = price.getPrice() - 2_Dollars;
        order.quantity = IntegerUtils::getSatoshiForPrice(price.getPrice(), conf.betSize);
        order.createdTime = ctx.data.get<Time>().getTime();
        if (!ctx.coinbase().submitOrder(order))
            order = CoinbaseOrder();
        return;
    }

    CoinbaseOrder updated = ctx.data.get<CoinbaseOrderBook>().getOrder(order.uuid);
    if (!updated)
    {
        order = CoinbaseOrder();
        return;
    }

    // Complete
    if (updated.state == CoinbaseOrder::Filled)
    {
        // Sell complete, do next buy
        if (order.buy == false)
        {
            ctx.data.get<Profits>().addOrderPair(usd_t(), order.value(), usd_t(), usd_t());
            order = CoinbaseOrder();
        }
        else
        {
            // Buy complete, queue sell
            ctx.data.get<Profits>().addOrderPair(order.value(), usd_t(), usd_t(), usd_t());
            order.uuid.clear();
            order.buy = false;
            order.price = price.getPrice() + 2_Dollars;

            btc_t btc = ctx.data.get<CoinbaseWallet>().getAvailBtc();
            if (btc < order.quantity)
                order.quantity = btc;

            if (!btc || !ctx.coinbase().submitOrder(order))
                order = CoinbaseOrder();
        }
        return;
    }

    // Error state
    if (updated.state != CoinbaseOrder::Open)
    {
        order = CoinbaseOrder();
        return;
    }

    // Buy price rose, cancel buy
    if (updated.buy && updated.price < price.getPrice() - 20_Dollars)
    {
        if (ctx.coinbase().cancelOrder(order.uuid))
            order = CoinbaseOrder();
        return;
    }

    // Sell price fell, requeue sell
    if (!updated.buy && updated.price > price.getPrice() + 20_Dollars)
    {
        if (ctx.coinbase().cancelOrder(order.uuid))
        {
            order.buy = false;
            order.price = price.getPrice() + 2_Dollars;
            if (!ctx.coinbase().submitOrder(order))
                order = CoinbaseOrder();
        }
    }
}
