#include <gtest/gtest.h>

#include <gtb/WindowTrader.h>
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
    WindowTrader &wt,
    MockUserTrades &fills,
    BotContext &ctx,
    utime_t t,
    usd_t price)
{
    ctx.data.get<Time>().setTime(t);
    BtcPrice bp;
    bp.setPrice(price);
    wt.process(bp);
    fills.process(bp);
    ctx.actionPool.waitComplete([]() noexcept {});
}

TEST(WindowTraderProfit, ProfitsOnMeanReversionOscillation)
{
    BotContext ctx;
    ctx.data.initData(MockMode(true));
    SteadyClock::setMockTime(ctx.data.get<Time>());
    ctx.data.get<CoinbaseInit>().setFullInit();
    ctx.setCoinbase(std::make_unique<MockCoinbase>(ctx));
    ctx.data.get<CoinbaseFeeTier>().setFeeTier(pp_t()); // zero fees for deterministic profit
    ctx.data.get<CoinbaseWallet>().update(1'000_Dollars, 0_Bitcoins, 0_Dollars, 0_Bitcoins);

    WindowTrader::Config conf;
    conf.name = "W-MeanRev";
    conf.windowSize = 30_Minutes;
    conf.candleSize = 5_Minutes;
    conf.highWindowBuffer = 0_Percent;
    conf.lowWindowBuffer = 0_Percent;
    conf.fireWindowBuffer = 5_PercentagePoints;
    conf.betSize = 500_Dollars;
    conf.buyDelta = 0_Dollars;
    conf.buyBelowPct = 0_Percent;
    conf.takeProfitDelta = 5_Dollars;
    conf.takeProfitPct = 0_Percent;
    conf.trailingDrop = 0_Percent;
    conf.sellFrequency = 1_Seconds;
    conf.partialSellRatio = 100_Percent;
    conf.capitalCap = 1'000_Dollars;
    conf.enableAdaptiveBands = false;
    conf.usePercentileBands = false;

    WindowTrader wt(ctx, conf);
    MockUserTrades fills(ctx);

    // Price oscillates sharply to force buys at lows and sells at highs.
    std::vector<usd_t> prices = {
        90'000_Dollars, 95'000_Dollars, 92'000_Dollars, 94'000_Dollars, 91'000_Dollars,
        96'000_Dollars, 99'000_Dollars, 101'000_Dollars, 103'000_Dollars
    };

    utime_t t = 0_Seconds;
    for (usd_t p : prices)
    {
        tick(wt, fills, ctx, t, p);
        t += 5_Minutes;
    }

    CoinbaseWallet &wallet = ctx.data.get<CoinbaseWallet>();
    usd_t portfolio = wallet.getUsd() + IntegerUtils::getValue(prices.back(), wallet.getBtc());
    EXPECT_GT(portfolio, 1'000_Dollars) << "portfolio=" << IntegerUtils::toUsdString(portfolio);
}

TEST(WindowTraderProfit, FireSaleStopsLosses)
{
    BotContext ctx;
    ctx.data.initData(MockMode(true));
    SteadyClock::setMockTime(ctx.data.get<Time>());
    ctx.data.get<CoinbaseInit>().setFullInit();
    ctx.setCoinbase(std::make_unique<MockCoinbase>(ctx));
    ctx.data.get<CoinbaseFeeTier>().setFeeTier(pp_t()); // zero fee for deterministic profit
    ctx.data.get<CoinbaseWallet>().update(1'000_Dollars, 0_Bitcoins, 0_Dollars, 0_Bitcoins);

    WindowTrader::Config conf;
    conf.name = "W-FireSale";
    conf.windowSize = 2_Hours;
    conf.candleSize = 5_Minutes;
    conf.highWindowBuffer = 0_Percent;
    conf.lowWindowBuffer = 0_Percent;
    conf.fireWindowBuffer = 5_PercentagePoints;
    conf.betSize = 80_Dollars;
    conf.buyDelta = 0_Dollars;
    conf.takeProfitDelta = 15_Dollars;
    conf.takeProfitPct = 10_PercentagePoints;
    conf.sellFrequency = 1_Seconds;
    conf.capitalCap = 400_Dollars;
    conf.pauseDuration = 1_Hours;

    WindowTrader wt(ctx, conf);
    MockUserTrades fills(ctx);

    // Buy on initial dip, then crash through fireSale buffer; ensure we don't go broke.
    std::vector<usd_t> prices = {
        100_Dollars, 99_Dollars, 98_Dollars, 97_Dollars, 96_Dollars, 95_Dollars,
        92_Dollars, 90_Dollars, 88_Dollars, 86_Dollars, 84_Dollars, 82_Dollars,
        80_Dollars, 78_Dollars, 76_Dollars
    };

    utime_t t = 0_Seconds;
    for (usd_t p : prices)
    {
        tick(wt, fills, ctx, t, p);
        t += 5_Minutes;
    }

    auto wallet = ctx.data.get<CoinbaseWallet>().getData();
    // Should retain significant USD (fire sale triggered, pause halted further buying)
    EXPECT_GT(wallet.usd, 900_Dollars);
}
