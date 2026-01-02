#pragma once

#include <gtb/BotContext.h>
#include <gtb/BtcPrice.h>
#include <gtb/IntLiterals.h>

namespace gtb
{

/**
 * Dollar-cost-average style buyer.
 * Buys a small amount once per day when price falls to (or below) the prior day's low,
 * and never sells (pending PnL is reflected via Profits).
 */
class HodlTrader
{
    public:
        struct Config
        {
            std::string name = "HodlTrader";
            usd_t betSize = 5_Dollars;
            pp_t buyBelowPct = 50_PercentagePoints; // require prior-day low minus 0.50%
        };

        HodlTrader(
            BotContext &ctx,
            Config conf);
        HodlTrader(HodlTrader &&) = delete;
        HodlTrader(const HodlTrader &) = delete;
        HodlTrader &operator=(HodlTrader &&) = delete;
        HodlTrader &operator=(const HodlTrader &) = delete;
        ~HodlTrader() = default;

        void process(
            const BtcPrice &price);

    private:
        void rollDay(
            usd_t price,
            utime_t dayStart);
        void updateOrderState();
        bool hasPrevDayLow() const;
        utime_t getDayStart(
            utime_t now) const;

        BotContext &ctx;
        Config conf;

        usd_t currentLow;
        usd_t previousLow;
        utime_t currentDay;
        bool purchasedToday;
        std::string openOrder;
};

}
