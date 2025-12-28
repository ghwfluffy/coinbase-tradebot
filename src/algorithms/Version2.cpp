#include <gtb/Version2.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

#include <gtb/OrderPairDb.h>

#include <gtb/PeriodicPrinter.h>

#include <gtb/CoinbaseInit.h>
#include <gtb/CoinbaseFeeTier.h>

#include <gtb/PendingProfitsCalc.h>

#include <gtb/SpreadTrader.h>
#include <gtb/ConstantTrader.h>
#include <gtb/ConstantSpreadTrader.h>
#include <gtb/WindowTrader.h>
#include <gtb/VolumeTrader.h>

#include <gtb/MockMode.h>
#include <gtb/MockMarket.h>
#include <gtb/MockCoinbase.h>
#include <gtb/MockUserTrades.h>
#include <gtb/SteadyClock.h>

using namespace gtb;

namespace
{

void initProd(
    TradeBot &bot)
{
    (void)bot;

    log::error("v2 production not hooked up");
}

void initMock(
    TradeBot &bot)
{
    BotContext &ctx = bot.getCtx();

    ctx.data.initData(MockMode(true));
    SteadyClock::setMockTime(ctx.data.get<Time>());
    unlink("data/mock_trader.sqlite");
    OrderPairDb::setDbFile("data/mock_trader.sqlite");
    // XXX: Use a copy of the historical database
    // so our fast reads dont interrupt the active tradebot by holding a read lock
    [[maybe_unused]] int x = system("cp data/historical.sqlite data/mock_historical.sqlite");
    ctx.historicalDb.init("data/mock_historical.sqlite", "./schema/historical.sql");

    // Mock coinbase API
    ctx.setCoinbase(std::make_unique<MockCoinbase>(ctx, 0_MillionDollars));

    // Initial state
    ctx.data.get<CoinbaseInit>().setFullInit();
    ctx.data.get<CoinbaseWallet>().update(50'000_Dollars, 0_Bitcoins, 0_Dollars, 0_Bitcoins);
    ctx.data.get<CoinbaseFeeTier>().setFeeTier(ctx.coinbase().getFeeTier());

    // Source: Historical market data
    std::vector<std::string> range = {
        //"2025-05-01", "",//"2025-06-30",
        //"2025-01-25", "2025-01-27",
        "2025-01-25", "2025-05-01",
        "", ""
    };
    bot.addSource(std::make_unique<MockMarket>(ctx, range[0], range[1]));

    // Processor: Emulate trades according to current BTC price
    bot.addProcessor(std::make_unique<MockUserTrades>(ctx));

    // Processor: Print periodic status updates
    bot.addProcessor(std::make_unique<PeriodicPrinter>(ctx));

    // Processor: Pending Profits
    // TODO: Make the tracked order pair class keep track of this
    //bot.addProcessor(std::make_unique<PendingProfitsCalc>(ctx));
}

#if 0
MarketPeriodConfig getRampedPeriodConf(bool hot)
{
    return {
        .hot = hot,
        .pausePeriod = 2_Hours,
        .pauseAcceptLoss = (hot ? 10_PercentagePoints : 100_Percent),
        .rampPeriod = 2_Hours,
        .rampGrade = 200_Percent,
    };
}

MarketPeriodConfig getPausedPeriodConf()
{
    return {
        .hot = false,
        .pausePeriod = 0_Seconds,
        .pauseAcceptLoss = 0_Percent,
        .rampPeriod = 0_Seconds,
        .rampGrade = 0_Percent,
    };
}

MarketTimeTraderConfig getMarketConf()
{
    return {
        .market = MarketInfo::Market::BitcoinFutures,

        // Open ----->
        .openMarket = getRampedPeriodConf(true),
        // Open -> Closed
        .closingMarket = getPausedPeriodConf(),
        // Closed ----->
        .closedMarket = getPausedPeriodConf(),
        // Closed -> Open
        .openingMarket = getRampedPeriodConf(false),
        // Open -> Weekend
        .weekendingMarket = getPausedPeriodConf(),
        // Weekend ---->
        .weekendMarket = getPausedPeriodConf(),
        // Weekend -> Open
        .weekStartingMarket = getRampedPeriodConf(true),
    };
}
#endif

}

void Version2::init(
    TradeBot &bot,
    bool mock)
{
    log::info("Initializing Ghw Trade Bot version 2%s.", mock ? " - Mock Test" : "");

    // Setup sources, processors, and initial state
    if (mock)
        initMock(bot);
    else
        initProd(bot);

    BotContext &ctx = bot.getCtx();

    // Trader: Window
    auto addWindow = [&](WindowTrader::Config conf) {
        bot.addProcessor(std::make_unique<WindowTrader>(ctx, conf));
    };

    // Core mean reversion bucket
    {
        WindowTrader::Config conf;
        conf.name = "Window-Core";
        conf.takeProfitDelta = 8_Dollars;
        conf.windowSize = 48_Hours;
        conf.candleSize = 20_Minutes;
        conf.pauseDuration = 12_Hours;
        conf.highWindowBuffer = 20_Percent;
        conf.lowWindowBuffer = 20_Percent;
        conf.fireWindowBuffer = 10_PercentagePoints;
        conf.betSize = 500_Dollars;
        conf.buyDelta = 0_Dollars;
        conf.buyBelowPct = 100_PercentagePoints;
        conf.buySpacingPct = 30_PercentagePoints;
        conf.sellFrequency = 6_Seconds;
        conf.takeProfitDelta = 300_Dollars;
        conf.takeProfitPct = 80_PercentagePoints;
        conf.trailingDrop = 50_PercentagePoints;
        conf.partialSellRatio = 70_Percent;
        conf.stopLossPct = 150_PercentagePoints;
        conf.trendGuardDelta = 500_Dollars;
        conf.capitalCap = 20'000_Dollars;
        addWindow(conf);
    }

    // Shorter-term, tighter bands for churn
    {
        WindowTrader::Config conf;
        conf.name = "Window-Short";
        conf.takeProfitDelta = 5_Dollars;
        conf.windowSize = 24_Hours;
        conf.candleSize = 10_Minutes;
        conf.pauseDuration = 6_Hours;
        conf.highWindowBuffer = 15_Percent;
        conf.lowWindowBuffer = 15_Percent;
        conf.fireWindowBuffer = 5_PercentagePoints;
        conf.betSize = 320_Dollars;
        conf.buyDelta = 0_Dollars;
        conf.buyBelowPct = 60_PercentagePoints;
        conf.buySpacingPct = 20_PercentagePoints;
        conf.sellFrequency = 6_Seconds;
        conf.takeProfitDelta = 220_Dollars;
        conf.takeProfitPct = 60_PercentagePoints;
        conf.trailingDrop = 40_PercentagePoints;
        conf.partialSellRatio = 60_Percent;
        conf.stopLossPct = 120_PercentagePoints;
        conf.trendGuardDelta = 300_Dollars;
        conf.capitalCap = 10'000_Dollars;
        conf.highPausePercentile = 93;
        conf.lowExitPercentile = 3;
        conf.buyBandLowerPercentile = 20;
        conf.buyBandUpperPercentile = 65;
        addWindow(conf);
    }

    // Longer-term, wider bands for crash riding
    {
        WindowTrader::Config conf;
        conf.name = "Window-Long";
        conf.takeProfitDelta = 11_Dollars;
        conf.windowSize = 72_Hours;
        conf.candleSize = 30_Minutes;
        conf.pauseDuration = 18_Hours;
        conf.highWindowBuffer = 25_Percent;
        conf.lowWindowBuffer = 25_Percent;
        conf.fireWindowBuffer = 10_PercentagePoints;
        conf.betSize = 350_Dollars;
        conf.buyDelta = 0_Dollars;
        conf.buyBelowPct = 120_PercentagePoints;
        conf.buySpacingPct = 30_PercentagePoints;
        conf.sellFrequency = 8_Seconds;
        conf.takeProfitDelta = 400_Dollars;
        conf.takeProfitPct = 100_PercentagePoints;
        conf.trailingDrop = 80_PercentagePoints;
        conf.partialSellRatio = 60_Percent;
        conf.stopLossPct = 200_PercentagePoints;
        conf.trendGuardDelta = 600_Dollars;
        conf.capitalCap = 10'000_Dollars;
        conf.highPausePercentile = 96;
        conf.lowExitPercentile = 2;
        conf.buyBandLowerPercentile = 25;
        conf.buyBandUpperPercentile = 70;
        addWindow(conf);
    }

    {
        VolumeTrader::Config conf;
        conf.name = "Volume";
        bot.addProcessor(std::make_unique<VolumeTrader>(ctx, conf));
    }
}
