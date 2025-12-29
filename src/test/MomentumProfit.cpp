#include <gtest/gtest.h>

#include <gtb/MomentumTrader.h>
#include <gtb/MockCoinbase.h>
#include <gtb/MockUserTrades.h>
#include <gtb/CoinbaseWallet.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/CoinbaseFeeTier.h>
#include <gtb/Time.h>
#include <gtb/BtcPrice.h>
#include <gtb/MockMode.h>
#include <gtb/SteadyClock.h>
#include <gtb/IntegerUtils.h>

#include <cmath>

#include "GTestPrinters.h"

using namespace gtb;

// Helper to tick both the trader and the mock fills.
static void tick(
    MomentumTrader &mt,
    MockUserTrades &fills,
    BotContext &ctx,
    utime_t t,
    usd_t price)
{
    ctx.data.get<Time>().setTime(t);
    BtcPrice bp;
    bp.setPrice(price);
    mt.process(bp);
    fills.process(bp);
    ctx.actionPool.waitComplete([]() noexcept {});
}

TEST(MomentumTraderProfit, ProfitsOnCleanBreakoutAndExit)
{
    BotContext ctx;
    ctx.data.initData(MockMode(true));
    SteadyClock::setMockTime(ctx.data.get<Time>());
    ctx.data.get<CoinbaseInit>().setFullInit();
    ctx.setCoinbase(std::make_unique<MockCoinbase>(ctx, 50_MillionDollars));
    ctx.data.get<CoinbaseWallet>().update(1'000_Dollars, 0_Bitcoins, 0_Dollars, 0_Bitcoins);

    MomentumTrader::Config conf;
    conf.name = "MTest";
    conf.breakoutPct = 0_PercentagePoints;     // trigger immediately
    conf.takeProfitPct = 1_PercentagePoints;   // 0.01% take-profit
    conf.trailingDrop = 0_Percent;
    conf.stopLossPct = 300_PercentagePoints;   // wide stop
    conf.minActionSpacing = 0_Seconds;
    conf.reentryCooldown = 0_Seconds;
    conf.betSize = 500_Dollars;
    conf.capitalCap = 1'000_Dollars;

    MomentumTrader mt(ctx, conf);
    MockUserTrades fills(ctx);

    // Price path:
    //  - 100: establish baseline high
    //  - 103: breakout triggers buy (order priced mid-2, fills when price dips)
    //  - 100: price dips to fill maker buy at ~101
    //  - 115: triggers take-profit sell
    //  - 117: fills maker sell at ~+2 above mid, locking profit
    std::vector<usd_t> prices = {
        100'000_Dollars, 103'000_Dollars, 100'000_Dollars, 115'000_Dollars, 117'000_Dollars
    };

    utime_t t = 0_Seconds;
    for (usd_t p : prices)
    {
        tick(mt, fills, ctx, t, p);
        t += 1_Seconds;
    }

    // Final pass to clear any pending orders at last price.
    tick(mt, fills, ctx, t, prices.back());

    CoinbaseWallet &wallet = ctx.data.get<CoinbaseWallet>();
    usd_t portfolio = wallet.getUsd() + IntegerUtils::getValue(prices.back(), wallet.getBtc());
    EXPECT_GT(portfolio, 1'000_Dollars) << "portfolio=" << IntegerUtils::toUsdString(portfolio);
}

TEST(MomentumTraderProfit, ProfitsInSinusoidalRange)
{
    BotContext ctx;
    ctx.data.initData(MockMode(true));
    SteadyClock::setMockTime(ctx.data.get<Time>());
    ctx.data.get<CoinbaseInit>().setFullInit();
    // High volume so zero fee for deterministic profit
    ctx.setCoinbase(std::make_unique<MockCoinbase>(ctx, 50_MillionDollars));
    ctx.data.get<CoinbaseWallet>().update(1'000_Dollars, 0_Bitcoins, 0_Dollars, 0_Bitcoins);

    MomentumTrader::Config conf;
    conf.name = "MSine";
    conf.breakoutPct = 0_PercentagePoints;     // trigger immediately
    conf.takeProfitPct = 1_PercentagePoints;   // 0.01% take-profit
    conf.trailingDrop = 0_Percent;
    conf.stopLossPct = 0_PercentagePoints;
    conf.minActionSpacing = 0_Seconds;
    conf.reentryCooldown = 0_Seconds;
    conf.betSize = 500_Dollars;
    conf.capitalCap = 1'000_Dollars;

    MomentumTrader mt(ctx, conf);
    MockUserTrades fills(ctx);

    // Deterministic oscillation: programmatic series of rising peaks/troughs to force repeated
    // entries and exits while trending upward overall.
    std::vector<usd_t> prices;
    usd_t base = 200'000_Dollars;
    usd_t amp = 5'000_Dollars;
    for (int i = 0; i < 6; ++i)
    {
        usd_t center = usd_t(base.value() + static_cast<uint64_t>(i) * (1'000_Dollars).value());
        usd_t higher = center + amp;
        usd_t lower = center - usd_t(amp.value() / 2);
        usd_t peak = higher + 2'000_Dollars;
        prices.push_back(center);
        prices.push_back(higher);
        prices.push_back(lower);
        prices.push_back(peak);
    }
    // Final elevated price to clear any pending sells and value holdings generously.
    prices.push_back(base + amp + 30'000_Dollars);

    utime_t t = 0_Seconds;
    for (usd_t p : prices)
    {
        tick(mt, fills, ctx, t, p);
        t += 1_Seconds;
    }
    // Final pass to resolve any open orders at the last observed price.
    tick(mt, fills, ctx, t, prices.back());

    CoinbaseWallet &wallet = ctx.data.get<CoinbaseWallet>();
    usd_t portfolio = wallet.getUsd() + IntegerUtils::getValue(prices.back(), wallet.getBtc());
    EXPECT_GT(portfolio, 1'000_Dollars);
}
