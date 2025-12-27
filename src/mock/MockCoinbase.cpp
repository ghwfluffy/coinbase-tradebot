#include <gtb/MockCoinbase.h>
#include <gtb/MockLock.h>

#include <gtb/Log.h>
#include <gtb/Uuid.h>

#include <gtb/CoinbaseOrderBook.h>
#include <gtb/CoinbaseWallet.h>
#include <gtb/Time.h>

using namespace gtb;

MockCoinbase::MockCoinbase(
    BotContext &ctx,
    big_usd_t dayZeroVolume)
        : ctx(ctx)
        , dayZeroVolume(std::move(dayZeroVolume))
{
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
        if (wallet.getAvailUsd() < order.value())
        {
            log::error("Not enough USD to submit order.");
            return false;
        }
        else if (!order.value())
        {
            log::error("Cannot submit invalid null buy.");
            return false;
        }
    }
    else
    {
        if (wallet.getAvailBtc() < order.quantity)
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
        walletData.onHoldUsd += order.value();
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
        if (walletData.onHoldUsd >= order.value())
        {
            walletData.onHoldUsd -= order.value();
        }
        else
        {
            log::error("Wallet does not have matching on hold USD for canceled order.");
            walletData.onHoldUsd = {};
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
            walletData.onHoldBtc = {};
        }
    }
    wallet.update(walletData);

    // Update orderbook
    order.state = CoinbaseOrder::State::Canceled;
    order.cleanupTime = SteadyClock::now() + std::chrono::seconds(20);
    //order.cleanupTime = SteadyClock::now() + std::chrono::minutes(2);
    orderBook.update(std::move(order));

    return true;
}

CoinbaseWallet::Data MockCoinbase::getWallet()
{
    auto lock = ctx.data.get<MockLock>().lock();
    return ctx.data.get<CoinbaseWallet>().getData();
}

pp_t MockCoinbase::getFeeTier()
{
    return feeTierForVolume(getVolume());
}

big_usd_t MockCoinbase::getVolume()
{
    std::lock_guard<std::mutex> lock(mtxVolume);
    pruneBuckets(lock);

    big_usd_t total;
    for (const auto &b : buckets)
        total += b.volume;
    return total;
}

void MockCoinbase::recordVolume(
    big_usd_t amount,
    utime_t time)
{
    std::lock_guard<std::mutex> lock(mtxVolume);
    utime_t hour = getHourStart(time);

    // Initialize dayZeroVolume once time is known
    if (buckets.empty() && dayZeroVolume)
    {
        VolumeBucket seed;
        seed.hour = hour - utime_t(1);
        seed.volume = dayZeroVolume;
        buckets.push_back(std::move(seed));
        dayZeroVolume = big_usd_t();
    }

    pruneBuckets(lock);

    if (!buckets.empty() && buckets.back().hour == hour)
    {
        buckets.back().volume += amount;
    }
    else
    {
        VolumeBucket b;
        b.hour = hour;
        b.volume = amount;
        buckets.push_back(std::move(b));
    }
}

utime_t MockCoinbase::getHourStart(utime_t time) const
{
    return (time / 1_Hours) * 1_Hours;
}

void MockCoinbase::pruneBuckets(
    const std::lock_guard<std::mutex> &lock)
{
    (void)lock;

    utime_t nowTime = ctx.data.get<Time>().getTime();
    utime_t nowHour = getHourStart(nowTime);
    utime_t cutoff = nowHour - (24_Hours * 30);
    while (!buckets.empty() && buckets.front().hour < cutoff)
        buckets.pop_front();
}

pp_t MockCoinbase::feeTierForVolume(big_usd_t vol) const
{
    BigInt dollarsBn = vol.value() / BigInt(1'000'000'000'000ULL); // convert to whole dollars
    uint64_t volDollars = dollarsBn.toUint64();
    return feeTierForVolume(volDollars);
}

pp_t MockCoinbase::feeTierForVolume(uint64_t dollars) const
{
    // Maker fee tiers (bps) per docs/coinbase/fees.md
    // Only maker fee is modeled; taker ignored for now.
    static const struct Tier { uint64_t threshold; uint32_t bps; } tiers[] = {
        {250'000'000ULL, 0},    // VIP 8
        {100'000'000ULL, 0},    // VIP 7 (maker 0.000%)
        {50'000'000ULL, 0},     // VIP 6 (maker 0.000%)
        {20'000'000ULL, 1},     // VIP 5 (0.010%)
        {10'000'000ULL, 2},     // VIP 4 (0.025% -> 2.5bps rounded down to 2 bps)
        {5'000'000ULL, 4},      // VIP 3 (0.040%)
        {1'000'000ULL, 5},      // VIP 2 (0.050%)
        {500'000ULL, 6},        // VIP 1 (0.060%)
        {250'000ULL, 7},        // Advanced 3 (0.075%)
        {75'000ULL, 12},        // Advanced 2 (0.125%)
        {25'000ULL, 25},        // Advanced 1 (0.250%)
        {10'000ULL, 40},        // Intro 2 (0.400%)
        {0ULL, 60},             // Intro 1 (0.600%)
    };

    for (const auto &t : tiers)
    {
        if (dollars >= t.threshold)
            return pp_t(static_cast<uint32_t>(t.bps));
    }
    return pp_t(60);
}
