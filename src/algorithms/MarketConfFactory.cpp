#include <gtb/MarketConfFactory.h>

using namespace gtb;

// Only create new order pairs during normal stock market hours
// No discounting or increasing-spread requirements in lead up to off hours
MarketTimeTraderConfig MarketConfFactory::onlyNormalHours()
{
    return {
        .market = MarketInfo::Market::StockMarket,

        // Open ----->
        .openMarket = {
            .hot = true,
        },
        // Open -> Closed
        .closingMarket = {
            .hot = true,
        },
        // Closed ----->
        .closedMarket = {
            .hot = false,
        },
        // Closed -> Open
        .openingMarket = {
            .hot = false,
        },
        // Open -> Weekend
        .weekendingMarket = {
            .hot = true,
        },
        // Weekend ---->
        .weekendMarket = {
            .hot = true,
        },
        // Weekend -> Open
        .weekStartingMarket = {
            .hot = true,
        },
    };
}

// Only create new order pairs during normal stock market hours
// Discounts active pairs in lead up to stock market closure so we hold less BTC in off hours
// Increases spread requirements in early stock market hours so we don't risk early hour negative volatility
MarketTimeTraderConfig MarketConfFactory::rampedStockHours()
{
    return {
        .market = MarketInfo::Market::StockMarket,

        // Open ----->
        .openMarket = {
            // We will trade when market opens
            .hot = true,
            // Excluding the first 20 minutes of market open, we won't trade
            .pausePeriod = 20_Minutes,
            // Accept 0.30% less on existing spreads during the pause period
            .pauseAcceptLoss = 30_PercentagePoints,
            // Then we warm up for 1 hour
            .rampPeriod = 1_Hours,
            // During 1 hour warmup we will require +100% (double) spread for the trades,
            // And that will linearly diminish down to +0% over thecourse of the hour
            .rampGrade = 100_Percent,
        },
        // Open -> Closed
        .closingMarket = {
            // We will trade as market is closing
            .hot = true,
            // Excluding the last 20 minutes of market close, we won't trade
            .pausePeriod = 20_Minutes,
            // Accept 100% less on existing spreads during the pause period (sell for original buy price)
            .pauseAcceptLoss = 100_Percent,
            // Before we hit the pause period, we will ramp down for 1 hour
            .rampPeriod = 1_Hours,
            // During 1 hour cooldown we will discount starting at +0% (normal sell price),
            // And linearly incrase the discount to +100% (sell for original buy price) by the end of the cooldown (start of pause period)
            .rampGrade = 100_Percent,
        },
        // Closed ----->
        .closedMarket = {
            // No new trades while closed, no discounts
            .hot = false,
        },
        // Closed -> Open
        .openingMarket = {
            // No new trades while closed and entering open period
            .hot = false,
        },
        // Open -> Weekend
        .weekendingMarket = {
            // We will trade as market is closing for the weekend
            .hot = true,
            // Excluding the last 20 minutes of market close, we won't trade
            .pausePeriod = 20_Minutes,
            // Accept 200% less on existing spreads during the pause period (sell for negative the original spread)
            .pauseAcceptLoss = 200_Percent,
            // Before we hit the pause period, we will ramp down for 2 hours
            .rampPeriod = 2_Hours,
            // During 2 hour cooldown we will discount starting at +0% (normal sell price),
            // And linearly incrase the discount to +100% (sell for original buy price) by the end of the cooldown (start of pause period)
            .rampGrade = 100_Percent,
        },
        // Weekend ---->
        .weekendMarket = {
            // We will not trade on the weekend
            .hot = false,
        },
        // Weekend -> Open
        .weekStartingMarket = {
            // We will trade as market is opening from the weekend
            .hot = true,
            .pausePeriod = 0_Minutes,
            .pauseAcceptLoss = 0_Percent,
            // We will accept losses for 2 hours before open
            .rampPeriod = 2_Hours,
            // During 2 hour we will accept 100%->0% losses linearly
            .rampGrade = 100_Percent,
        },
    };
}

// Trade at all hours (no gating/ramping); useful for coverage during off-hours/weekends.
MarketTimeTraderConfig MarketConfFactory::allHours()
{
    MarketPeriodConfig hotAllHours = {
        .hot = true,
        .pausePeriod = 0_Seconds,
        .pauseAcceptLoss = 0_Percent,
        .rampPeriod = 0_Seconds,
        .rampGrade = 0_Percent,
    };

    return {
        .market = MarketInfo::Market::None,
        .openMarket = hotAllHours,
        .closingMarket = hotAllHours,
        .closedMarket = hotAllHours,
        .openingMarket = hotAllHours,
        .weekendingMarket = hotAllHours,
        .weekendMarket = hotAllHours,
        .weekStartingMarket = hotAllHours,
    };
}

// Trade only during bitcoin futures hours
// And heavily discount/caution as we enter/leave those hours
MarketTimeTraderConfig MarketConfFactory::preferBitcoinHours()
{
    auto getRampedPeriodConf = [](bool hot) -> MarketPeriodConfig {
        return {
            .hot = hot,
            .pausePeriod = 2_Hours,
            .pauseAcceptLoss = (hot ? 10_PercentagePoints : 100_Percent),
            .rampPeriod = 2_Hours,
            .rampGrade = 200_Percent,
        };
    };

    auto getPausedPeriodConf = []() -> MarketPeriodConfig {
        return {
            .hot = false,
            .pausePeriod = 0_Seconds,
            .pauseAcceptLoss = 0_Percent,
            .rampPeriod = 0_Seconds,
            .rampGrade = 0_Percent,
        };
    };

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
