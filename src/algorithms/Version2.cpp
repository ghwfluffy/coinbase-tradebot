#include <gtb/Version2.h>
#include <gtb/Log.h>

#include <gtb/SpreadTrader.h>
#include <gtb/ConstantTrader.h>
#include <gtb/ConstantSpreadTrader.h>
#include <gtb/WindowTrader.h>
#include <gtb/VolumeTrader.h>
#include <gtb/MomentumTrader.h>
#include <gtb/MovingAverageTrader.h>

#include <gtb/MockSetup.h>

using namespace gtb;

namespace
{

void initProd(
    TradeBot &bot)
{
    (void)bot;

    log::error("v2 production not hooked up");
}

MockSetup::Config mockConf()
{
    return {
        .startDate = "2025-01-25",
        .endDate = "2025-05-01",
        .initHighVolume = true,
        .startWallet = 50'000_Dollars,
    };
}

}

void Version2::init(
    TradeBot &bot,
    bool mock)
{
    // Setup sources, processors, and initial state
    if (mock)
        MockSetup::init(bot, mockConf());
    else
        initProd(bot);

    BotContext &ctx = bot.getCtx();

    // Trader: Window
    auto addWindow = [&](WindowTrader::Config conf) {
        bot.addProcessor(std::make_unique<WindowTrader>(ctx, conf));
    };
    auto addMomentum = [&](MomentumTrader::Config conf) {
        bot.addProcessor(std::make_unique<MomentumTrader>(ctx, conf));
    };
    auto addMA = [&](MovingAverageTrader::Config conf) {
        bot.addProcessor(std::make_unique<MovingAverageTrader>(ctx, conf));
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

    // Trend-follow breakout scalpers to complement mean reversion.
    {
        MomentumTrader::Config conf;
        conf.name = "Momentum-Fast";
        conf.windowSize = 6_Hours;
        conf.breakoutPct = 30_PercentagePoints;
        conf.takeProfitPct = 80_PercentagePoints;
        conf.trailingDrop = 50_PercentagePoints;
        conf.stopLossPct = 120_PercentagePoints;
        conf.betSize = 250_Dollars;
        conf.capitalCap = 6'000_Dollars;
        addMomentum(conf);
    }
    {
        MomentumTrader::Config conf;
        conf.name = "Momentum-Swing";
        conf.windowSize = 24_Hours;
        conf.breakoutPct = 50_PercentagePoints;
        conf.takeProfitPct = 120_PercentagePoints;
        conf.trailingDrop = 70_PercentagePoints;
        conf.stopLossPct = 200_PercentagePoints;
        conf.betSize = 300_Dollars;
        conf.capitalCap = 8'000_Dollars;
        addMomentum(conf);
    }

    // Moving-average crossover to ride medium-term trends.
    {
        MovingAverageTrader::Config conf;
        conf.name = "MA-Long";
        conf.shortWindow = 24;
        conf.longWindow = 96;
        conf.candleSize = 15_Minutes;
        conf.entryBuffer = 15_PercentagePoints;
        conf.exitBuffer = 10_PercentagePoints;
        conf.takeProfitPct = 200_PercentagePoints;
        conf.stopLossPct = 120_PercentagePoints;
        conf.trailingDrop = 80_PercentagePoints;
        conf.betSize = 400_Dollars;
        conf.capitalCap = 10'000_Dollars;
        addMA(conf);
    }
    {
        MovingAverageTrader::Config conf;
        conf.name = "MA-Short";
        conf.shortWindow = 12;
        conf.longWindow = 36;
        conf.candleSize = 5_Minutes;
        conf.entryBuffer = 20_PercentagePoints;
        conf.exitBuffer = 15_PercentagePoints;
        conf.takeProfitPct = 150_PercentagePoints;
        conf.stopLossPct = 100_PercentagePoints;
        conf.trailingDrop = 70_PercentagePoints;
        conf.betSize = 250_Dollars;
        conf.capitalCap = 6'000_Dollars;
        addMA(conf);
    }
}
