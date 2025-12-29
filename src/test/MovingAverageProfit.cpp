#include <gtest/gtest.h>

#include <gtb/MovingAverageTrader.h>
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

#include "GTestPrinters.h"

using namespace gtb;

static void tick(
    MovingAverageTrader &ma,
    MockUserTrades &fills,
    BotContext &ctx,
    utime_t t,
    usd_t price)
{
    ctx.data.get<Time>().setTime(t);
    BtcPrice bp;
    bp.setPrice(price);
    ma.process(bp);
    fills.process(bp);
    ctx.actionPool.waitComplete([]() noexcept {});
}

TEST(MovingAverageTraderProfit, ProfitsOnTrendFollow)
{
    BotContext ctx;
    ctx.data.initData(MockMode(true));
    SteadyClock::setMockTime(ctx.data.get<Time>());
    ctx.data.get<CoinbaseInit>().setFullInit();
    ctx.setCoinbase(std::make_unique<MockCoinbase>(ctx));
    ctx.data.get<CoinbaseFeeTier>().setFeeTier(pp_t()); // zero fee for deterministic profit
    ctx.data.get<CoinbaseWallet>().update(1'000_Dollars, 0_Bitcoins, 0_Dollars, 0_Bitcoins);

    MovingAverageTrader::Config conf;
    conf.name = "MA-Test";
    conf.shortWindow = 3;
    conf.longWindow = 6;
    conf.candleSize = 1_Minutes;
    conf.entryBuffer = 0_Percent; // trigger as soon as short > long
    conf.exitBuffer = 10_PercentagePoints;
    conf.takeProfitPct = 100_PercentagePoints; // 1.0%
    conf.stopLossPct = 500_PercentagePoints;   // wide
    conf.trailingDrop = 0_Percent;
    conf.minSpacing = 0_Seconds;
    conf.betSize = 500_Dollars;
    conf.capitalCap = 1'000_Dollars;

    MovingAverageTrader ma(ctx, conf);
    MockUserTrades fills(ctx);

    // Uptrend then flatten -> should buy and sell for profit.
    std::vector<usd_t> prices = {
        100'000_Dollars, 101'000_Dollars, 102'000_Dollars, 103'000_Dollars,
        104'000_Dollars, 106'000_Dollars, 105'000_Dollars, 110'000_Dollars, 111'000_Dollars
    };

    utime_t t = 0_Seconds;
    for (usd_t p : prices)
    {
        tick(ma, fills, ctx, t, p);
        t += 1_Minutes;
    }

    CoinbaseWallet &wallet = ctx.data.get<CoinbaseWallet>();
    usd_t portfolio = wallet.getUsd() + IntegerUtils::getValue(prices.back(), wallet.getBtc());
    EXPECT_GT(portfolio, 1'000_Dollars) << "portfolio=" << IntegerUtils::toUsdString(portfolio);
}

TEST(MovingAverageTraderProfit, ProfitsInChoppyRangeWithBuffers)
{
    BotContext ctx;
    ctx.data.initData(MockMode(true));
    SteadyClock::setMockTime(ctx.data.get<Time>());
    ctx.data.get<CoinbaseInit>().setFullInit();
    ctx.setCoinbase(std::make_unique<MockCoinbase>(ctx));
    ctx.data.get<CoinbaseFeeTier>().setFeeTier(pp_t()); // zero fee for deterministic profit
    ctx.data.get<CoinbaseWallet>().update(1'000_Dollars, 0_Bitcoins, 0_Dollars, 0_Bitcoins);

    MovingAverageTrader::Config conf;
    conf.name = "MA-Chop";
    conf.shortWindow = 3;
    conf.longWindow = 5;
    conf.candleSize = 1_Minutes;
    conf.entryBuffer = 0_Percent; // trigger as soon as short > long
    conf.exitBuffer = 10_PercentagePoints;
    conf.takeProfitPct = 80_PercentagePoints;
    conf.stopLossPct = 200_PercentagePoints;
    conf.trailingDrop = 50_PercentagePoints;
    conf.minSpacing = 0_Seconds;
    conf.betSize = 400_Dollars;
    conf.capitalCap = 1'000_Dollars;

    MovingAverageTrader ma(ctx, conf);
    MockUserTrades fills(ctx);

    // Mildly oscillating path that crosses long MA occasionally.
    std::vector<usd_t> prices = {
        100'000_Dollars, 99'000_Dollars, 100'000_Dollars, 101'000_Dollars, 103'000_Dollars,
        105'000_Dollars, 102'000_Dollars, 107'000_Dollars, 108'000_Dollars
    };

    utime_t t = 0_Seconds;
    for (usd_t p : prices)
    {
        tick(ma, fills, ctx, t, p);
        t += 1_Minutes;
    }

    CoinbaseWallet &wallet = ctx.data.get<CoinbaseWallet>();
    usd_t portfolio = wallet.getUsd() + IntegerUtils::getValue(prices.back(), wallet.getBtc());
    EXPECT_GT(portfolio, 1'000_Dollars) << "portfolio=" << IntegerUtils::toUsdString(portfolio);
}
