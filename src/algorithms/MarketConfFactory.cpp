#include <gtb/MarketConfFactory.h>

using namespace gtb;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

namespace
{

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

}

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

MarketTimeTraderConfig MarketConfFactory::preferNormalHours()
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
