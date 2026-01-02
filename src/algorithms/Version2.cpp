#include <gtb/Version2.h>
#include <gtb/Log.h>

#include <gtb/BtcHistoricalWriter.h>
#include <gtb/CoinbaseMarket.h>
#include <gtb/CoinbaseRestClient.h>
#include <gtb/CoinbaseUserInfo.h>
#include <gtb/CoinbaseUserTrades.h>
#include <gtb/PeriodicPrinter.h>
#include <gtb/PeriodicTimeUpdater.h>
#include <gtb/WalletHistoricalWriter.h>

#include <gtb/MockSetup.h>

#include <gtb/SpreadTrader.h>
#include <gtb/ConstantTrader.h>
#include <gtb/ConstantSpreadTrader.h>
#include <gtb/WindowTrader.h>
#include <gtb/VolumeTrader.h>
#include <gtb/MomentumTrader.h>
#include <gtb/MovingAverageTrader.h>

using namespace gtb;

namespace
{

struct TraderPlan
{
    std::vector<WindowTrader::Config> windows;
    std::vector<MomentumTrader::Config> momentums;
    std::vector<MovingAverageTrader::Config> movingAverages;
    std::vector<VolumeTrader::Config> volumes;
};

TraderPlan buildTraderPlan()
{
    TraderPlan plan;

    // Mean-reversion buckets (use the profitable, simple mock configs)
#if 1
    {
        WindowTrader::Config conf;
        conf.name = "Window-Quick";
        conf.windowSize = 6_Hours;
        conf.candleSize = 5_Minutes;
        conf.pauseDuration = 30_Minutes;
        conf.highWindowBuffer = 0_Percent;
        conf.lowWindowBuffer = 0_Percent;
        conf.fireWindowBuffer = 4_PercentagePoints;
        conf.betSize = 350_Dollars;
        conf.buyDelta = 0_Dollars;
        conf.buyBelowPct = 15_PercentagePoints;
        conf.buySpacingPct = 10_PercentagePoints;
        conf.takeProfitDelta = 50_Dollars;
        conf.takeProfitPct = 60_PercentagePoints;
        conf.trailingDrop = 5_PercentagePoints;
        conf.partialSellRatio = 50_Percent;
        conf.stopLossPct = 8_PercentagePoints;
        conf.trendGuardDelta = 0_Dollars;
        conf.sellFrequency = 1_Seconds;
        conf.capitalCap = 10'000_Dollars;
        conf.enableAdaptiveBands = false;
        conf.usePercentileBands = true;
        conf.lowerPercentile = 20;
        conf.upperPercentile = 60;
        plan.windows.push_back(conf);
    }
#endif
#if 1
    {
        WindowTrader::Config conf;
        conf.name = "Window-Core";
        conf.windowSize = 12_Hours;
        conf.candleSize = 5_Minutes;
        conf.pauseDuration = 20_Minutes;
        conf.highWindowBuffer = 1_Percent;
        conf.lowWindowBuffer = 1_Percent;
        conf.fireWindowBuffer = 2_Percent;
        conf.betSize = 800_Dollars;
        conf.buyDelta = 0_Dollars;
        conf.buyBelowPct = 15_PercentagePoints;   // 0.15%
        conf.buySpacingPct = 15_PercentagePoints; // 0.15%
        conf.takeProfitDelta = 0_Dollars;
        conf.takeProfitPct = 75_PercentagePoints;   // 0.75%
        conf.trailingDrop = 25_PercentagePoints;    // 0.25%
        conf.partialSellRatio = 80_Percent;
        conf.stopLossPct = 0_Percent;               // disable tight stop-loss
        conf.trendGuardDelta = 60_Dollars;
        conf.sellFrequency = 1_Seconds;
        conf.capitalCap = 10'000_Dollars;
        conf.enableAdaptiveBands = true;
        conf.highPausePercentile = 98;
        conf.lowExitPercentile = 0; // disable defensive exits
        conf.buyBandLowerPercentile = 8;
        conf.buyBandUpperPercentile = 70;
        conf.enableDefensiveExit = false;
        conf.usePercentileBands = true;
        conf.lowerPercentile = 5;
        conf.upperPercentile = 95;
        conf.softExitPercentile = 0;
        conf.softExitSellRatio = 0_Percent;
        plan.windows.push_back(conf);
    }
#endif
#if 0
    {
        WindowTrader::Config conf;
        conf.name = "Window-Drift";
        conf.windowSize = 48_Hours;
        conf.candleSize = 15_Minutes;
        conf.pauseDuration = 8_Hours;
        conf.highWindowBuffer = 5_Percent;
        conf.lowWindowBuffer = 5_Percent;
        conf.fireWindowBuffer = 8_PercentagePoints;
        conf.betSize = 300_Dollars;
        conf.buyDelta = 0_Dollars;
        conf.buyBelowPct = 20_PercentagePoints;
        conf.buySpacingPct = 15_PercentagePoints;
        conf.takeProfitDelta = 40_Dollars;
        conf.takeProfitPct = 20_PercentagePoints;
        conf.trailingDrop = 15_PercentagePoints;
        conf.partialSellRatio = 100_Percent;
        conf.stopLossPct = 80_PercentagePoints;
        conf.trendGuardDelta = 200_Dollars;
        conf.sellFrequency = 4_Seconds;
        conf.capitalCap = 12'000_Dollars;
        conf.enableAdaptiveBands = false;
        conf.usePercentileBands = false;
        plan.windows.push_back(conf);
    }

    // Momentum breakouts (use mock-profitable thresholds)
    {
        MomentumTrader::Config conf;
        conf.name = "Momentum-Fast";
        conf.windowSize = 6_Hours;
        conf.breakoutPct = 0_PercentagePoints;    // trigger immediately on new highs
        conf.takeProfitPct = 1_PercentagePoints;  // 0.01%
        conf.trailingDrop = 0_Percent;
        conf.stopLossPct = 0_PercentagePoints;
        conf.minActionSpacing = 1_Seconds;
        conf.reentryCooldown = 0_Seconds;
        conf.betSize = 500_Dollars;
        conf.capitalCap = 3'000_Dollars;
        plan.momentums.push_back(conf);
    }
    {
        MomentumTrader::Config conf;
        conf.name = "Momentum-Swing";
        conf.windowSize = 24_Hours;
        conf.breakoutPct = 10_PercentagePoints;   // 0.10%
        conf.takeProfitPct = 1_PercentagePoints;
        conf.trailingDrop = 20_PercentagePoints;
        conf.stopLossPct = 0_PercentagePoints;
        conf.minActionSpacing = 4_Seconds;
        conf.reentryCooldown = 2_Minutes;
        conf.betSize = 400_Dollars;
        conf.capitalCap = 4'000_Dollars;
        plan.momentums.push_back(conf);
    }

    // Moving-average trend followers
    {
        MovingAverageTrader::Config conf;
        conf.name = "MA-Medium";
        conf.shortWindow = 3;
        conf.longWindow = 6;
        conf.candleSize = 1_Minutes;
        conf.entryBuffer = 0_Percent;
        conf.exitBuffer = 10_PercentagePoints;
        conf.takeProfitPct = 100_PercentagePoints;
        conf.stopLossPct = 500_PercentagePoints;
        conf.trailingDrop = 0_Percent;
        conf.minSpacing = 0_Seconds;
        conf.betSize = 500_Dollars;
        conf.capitalCap = 3'000_Dollars;
        plan.movingAverages.push_back(conf);
    }
    {
        MovingAverageTrader::Config conf;
        conf.name = "MA-Long";
        conf.shortWindow = 3;
        conf.longWindow = 5;
        conf.candleSize = 1_Minutes;
        conf.entryBuffer = 0_Percent;
        conf.exitBuffer = 10_PercentagePoints;
        conf.takeProfitPct = 80_PercentagePoints;
        conf.stopLossPct = 200_PercentagePoints;
        conf.trailingDrop = 50_PercentagePoints;
        conf.minSpacing = 0_Seconds;
        conf.betSize = 400_Dollars;
        conf.capitalCap = 3'000_Dollars;
        plan.movingAverages.push_back(conf);
    }
#endif
#if 1
    // Volume churners to reach fee tiers
    {
        VolumeTrader::Config conf;
        conf.name = "Volume-Churn";
        conf.betSize = 25_Dollars;
        conf.minProfitDelta = 1_Dollars;
        conf.repriceBand = 2_Dollars;
        conf.orderTtl = 10_Seconds;
        plan.volumes.push_back(conf);
    }
#endif
    return plan;
}

void addTraders(
    TradeBot &bot,
    const TraderPlan &plan)
{
    BotContext &ctx = bot.getCtx();

    for (const auto &conf : plan.windows)
        bot.addProcessor(std::make_unique<WindowTrader>(ctx, conf));
    for (const auto &conf : plan.momentums)
        bot.addProcessor(std::make_unique<MomentumTrader>(ctx, conf));
    for (const auto &conf : plan.movingAverages)
        bot.addProcessor(std::make_unique<MovingAverageTrader>(ctx, conf));
    for (const auto &conf : plan.volumes)
        bot.addProcessor(std::make_unique<VolumeTrader>(ctx, conf));
}

void initProd(
    TradeBot &bot)
{
    BotContext &ctx = bot.getCtx();

    ctx.historicalDb.init("data/v2_historical.sqlite", "./schema/v2_historical.sql");

    // Real coinbase API
    ctx.setCoinbase(std::make_unique<CoinbaseRestClient>());

    // Source: Time updater
    bot.addSource(std::make_unique<PeriodicTimeUpdater>(ctx));

    // Source: Coinbase market
    bot.addSource(std::make_unique<CoinbaseMarket>(ctx));

    // Source: Coinbase user trades
    bot.addSource(std::make_unique<CoinbaseUserTrades>(ctx));

    // Source: Coinbase user info
    bot.addSource(std::make_unique<CoinbaseUserInfo>(ctx));

    // Processor: Record BTC prices
    bot.addProcessor(std::make_unique<BtcHistoricalWriter>(ctx));

    // Processor: Record wallet value
    bot.addProcessor(std::make_unique<WalletHistoricalWriter>(ctx));

    // Processor: Print periodic status updates
    bot.addProcessor(std::make_unique<PeriodicPrinter>(ctx));
}

MockSetup::Config mockConf()
{
    return {
        //.startDate = "2025-02-25",
        //.endDate = "2025-04-25",
        .initHighVolume = true,
        .startWallet = 22'000_Dollars,
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

    // Traders
    addTraders(bot, buildTraderPlan());
}
