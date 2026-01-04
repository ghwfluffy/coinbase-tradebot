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

// Bias toward exiting ahead of the weekend; allow discounts near weekending while staying active otherwise.
MarketTimeTraderConfig MarketConfFactory::weekendDerisk()
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
            .hot = true,
        },
        // Closed -> Open
        .openingMarket = {
            .hot = true,
        },
        // Open -> Weekend
        .weekendingMarket = {
            .hot = true,
            // stop new entries 45m before weekend, accept up to breakeven exits
            .pausePeriod = 45_Minutes,
            .pauseAcceptLoss = 120_Percent,
            .rampPeriod = 60_Minutes,
            .rampGrade = 120_Percent,
        },
        // Weekend ----->
        .weekendMarket = {
            .hot = false,
        },
        // Weekend -> Open
        .weekStartingMarket = {
            .hot = true,
            .pausePeriod = 30_Minutes,
            .pauseAcceptLoss = 60_Percent,
            .rampPeriod = 60_Minutes,
            .rampGrade = 80_Percent,
        },
    };
}

// Reduce risk around midweek opens (Tue/Wed) where repeated losses are seen.
MarketTimeTraderConfig MarketConfFactory::midweekDerisk()
{
    return {
        .market = MarketInfo::Market::StockMarket,

        // Open ----->
        .openMarket = {
            .hot = true,
            .pausePeriod = 20_Minutes,
            .pauseAcceptLoss = 40_PercentagePoints,
            .rampPeriod = 60_Minutes,
            .rampGrade = 80_Percent,
        },
        // Open -> Closed
        .closingMarket = {
            .hot = true,
            .pausePeriod = 15_Minutes,
            .pauseAcceptLoss = 60_Percent,
            .rampPeriod = 30_Minutes,
            .rampGrade = 60_Percent,
        },
        // Closed ----->
        .closedMarket = {
            .hot = true,
        },
        // Closed -> Open
        .openingMarket = {
            .hot = true,
            .pausePeriod = 15_Minutes,
            .pauseAcceptLoss = 50_Percent,
            .rampPeriod = 45_Minutes,
            .rampGrade = 60_Percent,
        },
        // Open -> Weekend
        .weekendingMarket = {
            .hot = true,
        },
        // Weekend ----->
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

// Trade during stock hours but heavily buffer the open/close edges to avoid drawdowns from early/late whipsaws.
MarketTimeTraderConfig MarketConfFactory::shieldedStockOpen()
{
    return {
        .market = MarketInfo::Market::StockMarket,

        // Open ----->
        .openMarket = {
            .hot = true,
            // Sit out the first 45 minutes after open
            .pausePeriod = 45_Minutes,
            // Allow modest losses on existing spreads during the pause to exit risk
            .pauseAcceptLoss = 40_PercentagePoints,
            // Then warm up for 90 minutes with elevated spread requirements
            .rampPeriod = 90_Minutes,
            .rampGrade = 150_Percent,
        },
        // Open -> Closed
        .closingMarket = {
            .hot = true,
            // Stop adding new risk 30 minutes before close
            .pausePeriod = 30_Minutes,
            // Will sell down to near cost as we approach close
            .pauseAcceptLoss = 80_Percent,
            // Ramp down for an hour into the pause
            .rampPeriod = 60_Minutes,
            .rampGrade = 100_Percent,
        },
        // Closed ----->
        .closedMarket = {
            .hot = false,
        },
        // Closed -> Open
        .openingMarket = {
            // Stay cold heading into the open to avoid overnight gaps
            .hot = false,
        },
        // Open -> Weekend
        .weekendingMarket = {
            .hot = true,
            // Stop taking new trades 45 minutes before weekend close
            .pausePeriod = 45_Minutes,
            // Will take a deeper discount to shed BTC before the break
            .pauseAcceptLoss = 120_Percent,
            // Ramp into the pause for 90 minutes
            .rampPeriod = 90_Minutes,
            .rampGrade = 120_Percent,
        },
        // Weekend ----->
        .weekendMarket = {
            .hot = false,
        },
        // Weekend -> Open
        .weekStartingMarket = {
            // Warm back up from the weekend with a controlled ramp
            .hot = true,
            .pausePeriod = 30_Minutes,
            .pauseAcceptLoss = 50_Percent,
            .rampPeriod = 90_Minutes,
            .rampGrade = 100_Percent,
        },
    };
}

// Trade all week but soften the edges around regular stock opens/closes to reduce open-vol whipsaws.
MarketTimeTraderConfig MarketConfFactory::gentleOpenHours()
{
    return {
        .market = MarketInfo::Market::StockMarket,

        // Open ----->
        .openMarket = {
            .hot = true,
            .pausePeriod = 30_Minutes,
            .pauseAcceptLoss = 25_PercentagePoints,
            .rampPeriod = 1_Hours,
            .rampGrade = 50_Percent,
        },
        // Open -> Closed
        .closingMarket = {
            .hot = true,
            .pausePeriod = 30_Minutes,
            .pauseAcceptLoss = 50_Percent,
            .rampPeriod = 1_Hours,
            .rampGrade = 50_Percent,
        },
        // Closed ----->
        .closedMarket = {
            .hot = true, // allow off-hours probing
        },
        // Closed -> Open
        .openingMarket = {
            .hot = true,
            .pausePeriod = 15_Minutes,
            .pauseAcceptLoss = 25_PercentagePoints,
            .rampPeriod = 30_Minutes,
            .rampGrade = 25_Percent,
        },
        // Open -> Weekend
        .weekendingMarket = {
            .hot = true,
            .pausePeriod = 45_Minutes,
            .pauseAcceptLoss = 75_Percent,
            .rampPeriod = 90_Minutes,
            .rampGrade = 75_Percent,
        },
        // Weekend ---->
        .weekendMarket = {
            .hot = true,
        },
        // Weekend -> Open
        .weekStartingMarket = {
            .hot = true,
            .pausePeriod = 30_Minutes,
            .pauseAcceptLoss = 50_Percent,
            .rampPeriod = 90_Minutes,
            .rampGrade = 50_Percent,
        },
    };
}

// Trade during stock market hours with light ramp in/out and avoid weekends.
// Built for volume-focused traders so they don't churn during thin sessions.
MarketTimeTraderConfig MarketConfFactory::volumeStockHours()
{
    return {
        .market = MarketInfo::Market::StockMarket,

        // Open ----->
        .openMarket = {
            .hot = true,
            .pausePeriod = 15_Minutes,
            .pauseAcceptLoss = 20_PercentagePoints,
            .rampPeriod = 45_Minutes,
            .rampGrade = 50_Percent,
        },
        // Open -> Closed
        .closingMarket = {
            .hot = true,
            .pausePeriod = 30_Minutes,
            .pauseAcceptLoss = 75_Percent,
            .rampPeriod = 1_Hours,
            .rampGrade = 75_Percent,
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
            .pausePeriod = 45_Minutes,
            .pauseAcceptLoss = 100_Percent,
            .rampPeriod = 90_Minutes,
            .rampGrade = 80_Percent,
        },
        // Weekend ---->
        .weekendMarket = {
            .hot = false,
        },
        // Weekend -> Open
        .weekStartingMarket = {
            .hot = true,
            .pausePeriod = 30_Minutes,
            .pauseAcceptLoss = 50_Percent,
            .rampPeriod = 90_Minutes,
            .rampGrade = 50_Percent,
        },
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
