#include <gtb/TraderConfFactory.h>
#include <gtb/MarketConfFactory.h>

using namespace gtb;

// Fast, short-window scalper for volatile periods; small bet, narrow bands, high sell cadence.
WindowTrader::Config TraderConfFactory::Window::quick()
{
    WindowTrader::Config conf;
    conf.name = "Window-Quick";
    conf.windowSize = 6_Hours;
    conf.candleSize = 5_Minutes;
    conf.pauseDuration = 45_Minutes;
    conf.highWindowBuffer = 0_Percent;
    conf.lowWindowBuffer = 0_Percent;
    conf.fireWindowBuffer = 4_PercentagePoints;
    conf.betSize = 50_Dollars;
    conf.buyDelta = 0_Dollars;
    conf.buyBelowPct = 45_PercentagePoints;
    conf.buySpacingPct = 55_PercentagePoints;
    conf.takeProfitDelta = 20_Dollars;
    conf.takeProfitPct = 120_PercentagePoints;
    conf.trailingDrop = 30_PercentagePoints;
    conf.partialSellRatio = 50_Percent;
    conf.stopLossPct = 20_PercentagePoints;
    conf.trendGuardDelta = 0_Dollars;
    conf.sellFrequency = 1_Seconds;
    conf.capitalCap = 1'500_Dollars;
    conf.spendLimit = 1'000_Dollars;
    conf.enableAdaptiveBands = true;
    conf.highPausePercentile = 95;
    conf.lowExitPercentile = 8;
    conf.buyBandLowerPercentile = 15;
    conf.buyBandUpperPercentile = 60;
    conf.enableDefensiveExit = true;
    conf.softExitPercentile = 25;
    conf.softExitSellRatio = 40_Percent;
    conf.usePercentileBands = true;
    conf.lowerPercentile = 15;
    conf.upperPercentile = 75;
    conf.volatilityDampen = 35_Percent;
    conf.minBetScale = 30_Percent;
    conf.maxExposureUsd = 1'000_Dollars;
    return conf;
}

// Balanced intraday window trader; wider window, defensive exits, adaptive bands for median conditions.
WindowTrader::Config TraderConfFactory::Window::core()
{
    WindowTrader::Config conf;
    conf.name = "Window-Core";
    conf.windowSize = 12_Hours;
    conf.candleSize = 5_Minutes;
    conf.pauseDuration = 30_Minutes;
    conf.highWindowBuffer = 1_Percent;
    conf.lowWindowBuffer = 1_Percent;
    conf.fireWindowBuffer = 2_Percent;
    conf.betSize = 150_Dollars;
    conf.buyDelta = 0_Dollars;
    conf.buyBelowPct = 50_PercentagePoints;   // 0.50%
    conf.buySpacingPct = 60_PercentagePoints; // 0.60%
    conf.takeProfitDelta = 0_Dollars;
    conf.takeProfitPct = 140_PercentagePoints;   // 1.40%
    conf.trailingDrop = 60_PercentagePoints;    // 0.60%
    conf.partialSellRatio = 80_Percent;
    conf.stopLossPct = 60_PercentagePoints;      // 0.60% stop to cap drawdown
    conf.trendGuardDelta = 60_Dollars;
    conf.sellFrequency = 1_Seconds;
    conf.capitalCap = 1'800_Dollars;
    conf.spendLimit = 1'000_Dollars;
    conf.enableAdaptiveBands = true;
    conf.highPausePercentile = 98;
    conf.lowExitPercentile = 10; // defensive exits for drawdown control
    conf.buyBandLowerPercentile = 8;
    conf.buyBandUpperPercentile = 70;
    conf.enableDefensiveExit = true;
    conf.usePercentileBands = true;
    conf.lowerPercentile = 5;
    conf.upperPercentile = 95;
    conf.softExitPercentile = 20;
    conf.softExitSellRatio = 25_Percent;
    conf.volatilityDampen = 40_Percent;
    conf.minBetScale = 30_Percent;
    conf.maxExposureUsd = 1'200_Dollars;
    return conf;
}

// Slow swing profile; very wide window with higher bets meant to ride broader moves, fewer adaptive tweaks.
WindowTrader::Config TraderConfFactory::Window::drift()
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
    return conf;
}

// Classic short-over-long crossover tuned for moderate pace; low capital cap and gentle buffers.
MovingAverageTrader::Config TraderConfFactory::Moving::medium()
{
    MovingAverageTrader::Config conf;
    conf.name = "MA-Medium";
    conf.shortWindow = 3;
    conf.longWindow = 8;
    conf.candleSize = 5_Minutes;
    conf.entryBuffer = 0_Percent;
    conf.exitBuffer = 15_PercentagePoints;
    conf.takeProfitPct = 120_PercentagePoints;  // 1.20%
    conf.stopLossPct = 50_PercentagePoints;     // 0.50%
    conf.trailingDrop = 50_PercentagePoints;    // 0.50%
    conf.minSpacing = 60_Seconds;
    conf.betSize = 40_Dollars;
    conf.capitalCap = 150_Dollars;
    return conf;
}

// Breakout/mean-reversion swing entry with modest profit/stop bands and cooldown to avoid churn.
MomentumTrader::Config TraderConfFactory::Momentum::swing()
{
    MomentumTrader::Config conf;
    conf.name = "Momentum-Swing";
    conf.windowSize = 24_Hours;
    conf.breakoutPct = 80_PercentagePoints;   // 0.80% above recent high
    conf.takeProfitPct = 120_PercentagePoints; // 1.20%
    conf.trailingDrop = 40_PercentagePoints;   // 0.40%
    conf.stopLossPct = 40_PercentagePoints;    // 0.40%
    conf.minActionSpacing = 90_Seconds;
    conf.reentryCooldown = 45_Minutes;
    conf.betSize = 30_Dollars;
    conf.capitalCap = 150_Dollars;
    return conf;
}

// High-turnover volume churner: big bets, tight profit delta, fast TTL to drive fee-tier volume.
VolumeTrader::Config TraderConfFactory::Volume::churn()
{
    VolumeTrader::Config conf;
    conf.name = "Volume-Churn";
    conf.betSize = 10_Dollars;
    conf.minProfitDelta = 20_Dollars;
    conf.repriceBand = 100_Dollars;
    conf.orderTtl = 1_Minutes;
    conf.marketParams.push_back(MarketConfFactory::volumeStockHours());
    return conf;
}

// Stock-hour gated churner with slightly larger bets and slower cadence to reduce fee bleed.
VolumeTrader::Config TraderConfFactory::Volume::stockChurn()
{
    VolumeTrader::Config conf;
    conf.name = "Volume-StockChurn";
    conf.betSize = 30_Dollars;
    conf.minProfitDelta = 8_Dollars;
    conf.repriceBand = 60_Dollars;
    conf.orderTtl = 90_Seconds;
    conf.marketParams.push_back(MarketConfFactory::volumeStockHours());
    return conf;
}

// Low-intensity volume maintainer: smaller bets and wider repricing for steady volume without large swings.
VolumeTrader::Config TraderConfFactory::Volume::drip()
{
    VolumeTrader::Config conf;
    conf.name = "Volume-Drip";
    conf.betSize = 200_Dollars;
    conf.minProfitDelta = 8_Dollars;
    conf.repriceBand = 50_Dollars;
    conf.orderTtl = 120_Seconds;
    conf.marketParams.push_back(MarketConfFactory::preferBitcoinHours());
    return conf;
}

// Faster cadence volume booster for safer hours to raise 30-day volume without full churn loss.
VolumeTrader::Config TraderConfFactory::Volume::pulse()
{
    VolumeTrader::Config conf;
    conf.name = "Volume-Pulse";
    conf.betSize = 250_Dollars;
    conf.minProfitDelta = 6_Dollars;
    conf.repriceBand = 35_Dollars;
    conf.orderTtl = 45_Seconds;
    conf.marketParams.push_back(MarketConfFactory::volumeStockHours());
    conf.marketParams.push_back(MarketConfFactory::preferBitcoinHours());
    return conf;
}

// All-hours feeder with moderate sizing to lift 30d volume while accepting small bleed.
VolumeTrader::Config TraderConfFactory::Volume::allHoursFeeder()
{
    VolumeTrader::Config conf;
    conf.name = "Volume-Feeder";
    conf.betSize = 180_Dollars;
    conf.minProfitDelta = 3_Dollars;
    conf.repriceBand = 20_Dollars;
    conf.orderTtl = 45_Seconds;
    conf.marketParams.push_back(MarketConfFactory::gentleOpenHours());
    return conf;
}

// Dollar-cost-average buyer: once-daily small purchase at/below prior-day low, never sells.
HodlTrader::Config TraderConfFactory::Hodl::hodl()
{
    HodlTrader::Config conf;
    conf.betSize = 20_Dollars;
    return conf;
}

SpreadTrader::Config TraderConfFactory::Spread::breakEven()
{
    SpreadTrader::Config conf;
    conf.name = "Spread-BreakEven";
    conf.spread = 30_PercentagePoints;
    conf.bet = 500_Dollars;
    conf.numPairs = 4;
    conf.buffer = 25_Percent;
    conf.marketParams.push_back(MarketConfFactory::preferBitcoinHours());
    conf.marketParams.push_back(MarketConfFactory::shieldedStockOpen());
    return conf;
}

SpreadTrader::Config TraderConfFactory::Spread::small()
{
    SpreadTrader::Config conf;
    conf.name = "Spread-Small";
    conf.spread = 35_PercentagePoints;
    conf.bet = 300_Dollars;
    conf.numPairs = 6;
    conf.buffer = 10_Percent;
    conf.marketParams.push_back(MarketConfFactory::preferBitcoinHours());
    conf.marketParams.push_back(MarketConfFactory::shieldedStockOpen());
    return conf;
}

SpreadTrader::Config TraderConfFactory::Spread::medium()
{
    SpreadTrader::Config conf;
    conf.name = "Spread-Medium";
    conf.spread = 50_PercentagePoints;
    conf.bet = 50_Dollars;
    conf.numPairs = 5;
    conf.buffer = 10_Percent;
    conf.marketParams.push_back(MarketConfFactory::preferBitcoinHours());
    return conf;
}

SpreadTrader::Config TraderConfFactory::Spread::large()
{
    SpreadTrader::Config conf;
    conf.name = "Spread-Large";
    conf.spread = 75_PercentagePoints;
    conf.bet = 100_Dollars;
    conf.numPairs = 2;
    conf.buffer = 4_Percent;
    conf.marketParams.push_back(MarketConfFactory::preferBitcoinHours());
    conf.marketParams.push_back(MarketConfFactory::rampedStockHours());
    return conf;
}

// Always-on probe to capture off-hours/weekend liquidity with moderate size/spread.
SpreadTrader::Config TraderConfFactory::Spread::allHoursProbe()
{
    SpreadTrader::Config conf;
    conf.name = "Spread-AllHoursProbe";
    conf.spread = 45_PercentagePoints;
    conf.bet = 200_Dollars;
    conf.numPairs = 6;
    conf.buffer = 12_Percent;
    conf.marketParams.push_back(MarketConfFactory::gentleOpenHours());
    return conf;
}

TimeTrader::Config TraderConfFactory::Time::small()
{
    TimeTrader::Config conf;
    conf.name = "Time-Small";
    conf.bet = 300_Dollars;
    conf.sampleSize = 30_Minutes;
    conf.minSpread = 4_PercentagePoints;
    conf.paddingSpread = 1_PercentagePoints;
    conf.numPairs = 6;
    conf.marketParams.push_back(MarketConfFactory::preferBitcoinHours());
    conf.marketParams.push_back(MarketConfFactory::gentleOpenHours());
    conf.marketParams.push_back(MarketConfFactory::shieldedStockOpen());
    return conf;
}

TimeTrader::Config TraderConfFactory::Time::medium()
{
    TimeTrader::Config conf;
    conf.name = "Time-Medium";
    conf.bet = 2000_Dollars;
    conf.sampleSize = 2_Hours;
    conf.minSpread = 5_PercentagePoints;
    conf.paddingSpread = 2_PercentagePoints;
    conf.numPairs = 2;
    conf.marketParams.push_back(MarketConfFactory::onlyNormalHours());
    conf.marketParams.push_back(MarketConfFactory::gentleOpenHours());
    conf.marketParams.push_back(MarketConfFactory::shieldedStockOpen());
    return conf;
}

TimeTrader::Config TraderConfFactory::Time::large()
{
    TimeTrader::Config conf;
    conf.name = "Time-Large";
    conf.bet = 2000_Dollars;
    conf.sampleSize = 6_Hours;
    conf.minSpread = 10_PercentagePoints;
    conf.paddingSpread = 2_PercentagePoints;
    conf.numPairs = 2;
    conf.marketParams.push_back(MarketConfFactory::preferBitcoinHours());
    conf.marketParams.push_back(MarketConfFactory::rampedStockHours());
    return conf;
}
