#include <gtb/MockCoinbase.h>
#include <gtb/MockLock.h>

#include <gtb/Log.h>
#include <gtb/Uuid.h>

#include <gtb/CoinbaseOrderBook.h>
#include <gtb/CoinbaseWallet.h>

using namespace gtb;

MockCoinbase::MockCoinbase(
    BotContext &ctx,
    uint32_t feeTier)
        : ctx(ctx)
{
    this->feeTier = feeTier;
}

CoinbaseOrder MockCoinbase::getOrder(
    const std::string &uuid)
{
    auto lock = ctx.data.get<MockLock>().lock();

    CoinbaseOrderBook &orderBook = ctx.data.get<CoinbaseOrderBook>();

    // Get order
    CoinbaseOrder order = orderBook.getOrder(uuid);
    if (!order)
    {
        log::error("No such order to retrieve '%s'.", uuid.c_str());
        return CoinbaseOrder();
    }

    return order;
}

bool MockCoinbase::submitOrder(
    CoinbaseOrder &order)
{
    auto lock = ctx.data.get<MockLock>().lock();

    CoinbaseWallet &wallet = ctx.data.get<CoinbaseWallet>();
    CoinbaseOrderBook &orderBook = ctx.data.get<CoinbaseOrderBook>();

    // Verify there is enough in the wallet
    if (order.buy)
    {
        if (wallet.getAvailUsdCents() < order.valueCents())
        {
            log::error("Not enough USD to submit order.");
            return false;
        }
        else if (!order.valueCents())
        {
            log::error("Cannot submit invalid null buy.");
            return false;
        }
    }
    else
    {
        if (wallet.getAvailBtcSatoshi() < order.quantity)
        {
            log::error("Not enough BTC to submit order.");
            return false;
        }
        else if (!order.quantity)
        {
            log::error("Cannot submit invalid null sell.");
            return false;
        }
    }

    // Update on hold amounts
    CoinbaseWallet::Data walletData = wallet.getData();
    if (order.buy)
        walletData.onHoldUsd += order.valueCents();
    else
        walletData.onHoldBtc += order.quantity;
    wallet.update(walletData);

    // Save order
    order.state = CoinbaseOrder::State::Open;
    order.uuid = Uuid::generate();
    orderBook.update(order);

    return true;
}

bool MockCoinbase::cancelOrder(
    const std::string &uuid)
{
    auto lock = ctx.data.get<MockLock>().lock();

    CoinbaseOrderBook &orderBook = ctx.data.get<CoinbaseOrderBook>();

    // Get order
    CoinbaseOrder order = orderBook.getOrder(uuid);
    if (!order)
    {
        log::error("No such order to cancel '%s'.", uuid.c_str());
        return false;
    }

    // Check it's cancelable
    if (order.state != CoinbaseOrder::State::Open)
    {
        log::error("Can't cancel order '%s' is not active.", uuid.c_str());
        return false;
    }

    // Update wallet on hold amount
    CoinbaseWallet &wallet = ctx.data.get<CoinbaseWallet>();
    CoinbaseWallet::Data walletData = wallet.getData();
    if (order.buy)
    {
        if (walletData.onHoldUsd >= order.valueCents())
        {
            walletData.onHoldUsd -= order.valueCents();
        }
        else
        {
            log::error("Wallet does not have matching on hold USD for canceled order.");
            walletData.onHoldUsd = 0;
        }
    }
    else
    {
        if (walletData.onHoldBtc >= order.quantity)
        {
            walletData.onHoldBtc -= order.quantity;
        }
        else
        {
            log::error("Wallet does not have matching on hold BTC for canceled order.");
            walletData.onHoldBtc = 0;
        }
    }
    wallet.update(walletData);

    // Update orderbook
    order.state = CoinbaseOrder::State::Canceled;
    order.cleanupTime = SteadyClock::now() + std::chrono::minutes(2);
    orderBook.update(std::move(order));

    return true;
}

CoinbaseWallet::Data MockCoinbase::getWallet()
{
    auto lock = ctx.data.get<MockLock>().lock();
    return ctx.data.get<CoinbaseWallet>().getData();
}

uint32_t MockCoinbase::getFeeTier()
{
    return feeTier;
}
