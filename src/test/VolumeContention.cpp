#include <gtest/gtest.h>

#include <gtb/WindowTrader.h>
#include <gtb/VolumeTrader.h>
#include <gtb/MockCoinbase.h>
#include <gtb/MockMarket.h>
#include <gtb/MockUserTrades.h>
#include <gtb/CoinbaseWallet.h>
#include <gtb/CoinbaseOrderBook.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/CoinbaseFeeTier.h>
#include <gtb/Time.h>
#include <gtb/BtcPrice.h>
#include <gtb/MockMode.h>
#include <gtb/SteadyClock.h>

using namespace gtb;

// Helper to run both traders against a single price tick.
static void tickAll(
    WindowTrader &wt,
    VolumeTrader &vt,
    usd_t price)
{
    BtcPrice bp;
    bp.setPrice(price);
    wt.process(bp);
    vt.process(bp);
}

TEST(VolumeContention, WindowTraderRespectsAvailBtcWhenContended)
{
    BotContext ctx;
    ctx.data.initData(MockMode(true));
    SteadyClock::setMockTime(ctx.data.get<Time>());
    ctx.data.get<CoinbaseInit>().setFullInit();
    ctx.data.get<CoinbaseWallet>().update(1'000_Dollars, 0_Bitcoins, 0_Dollars, 0_Bitcoins);
    ctx.setCoinbase(std::make_unique<MockCoinbase>(ctx));
    ctx.data.get<CoinbaseFeeTier>().setFeeTier(ctx.coinbase().getFeeTier());

    // Set a starting price
    ctx.data.get<BtcPrice>().setPrice(30'000_Dollars);
    ctx.data.get<Time>().setTime(utime_t(0));

    WindowTrader::Config wconf;
    wconf.name = "W";
    wconf.betSize = 100_Dollars;
    wconf.sellFrequency = 1_Seconds;
    wconf.buyDelta = 0_Dollars;
    wconf.highWindowBuffer = 0_Percent;
    wconf.lowWindowBuffer = 0_Percent;
    wconf.windowSize = 2_Hours;
    wconf.candleSize = 5_Minutes;

    VolumeTrader::Config vconf;
    vconf.name = "V";
    vconf.betSize = 10_Dollars;

    WindowTrader wt(ctx, wconf);
    VolumeTrader vt(ctx, vconf);
    ctx.actionPool.waitComplete([]() noexcept {});

    // Seed both traders with initial buys so they both hold some BTC.
    tickAll(wt, vt, 30'000_Dollars);
    ctx.actionPool.waitComplete([]() noexcept {});
    // Force fills by marking orders as filled in the order book.
    auto ordersMap = ctx.data.get<CoinbaseOrderBook>().getOrders();
    std::list<CoinbaseOrder> filled;
    for (auto &kv : ordersMap)
    {
        kv.second.state = CoinbaseOrder::Filled;
        filled.push_back(kv.second);
    }
    ctx.data.get<CoinbaseOrderBook>().update(std::move(filled));
    ctx.actionPool.waitComplete([]() noexcept {});

    // Advance time and price, then let both try to sell simultaneously.
    ctx.data.get<Time>().setTime(utime_t(10));
    tickAll(wt, vt, 31'000_Dollars);
    ctx.actionPool.waitComplete([]() noexcept {});

    // Ensure we did not log a "Not enough BTC to submit order." (MockCoinbase error)
    // by asserting the order book only contains as many open sells as holdings allow.
    auto afterOrders = ctx.data.get<CoinbaseOrderBook>().getOrders();
    btc_t onHoldBtc = ctx.data.get<CoinbaseWallet>().getData().onHoldBtc;
    btc_t availBtc = ctx.data.get<CoinbaseWallet>().getAvailBtc();
    // Ensure we didn't over-submit: sum of open sells should not exceed holding.
    btc_t openSells;
    for (const auto &kv : afterOrders)
    {
        if (!kv.second.buy && kv.second.state == CoinbaseOrder::Open)
            openSells += kv.second.quantity;
    }
    EXPECT_LE(openSells.value(), (onHoldBtc + availBtc).value());
}
