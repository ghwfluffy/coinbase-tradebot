#pragma once

#include <gtb/BotContext.h>
#include <gtb/BtcPrice.h>
#include <gtb/CoinbaseOrder.h>
#include <gtb/IntLiterals.h>

#include <deque>

namespace gtb
{

/**
 * Simple breakout/trend-following long trader.
 * - Buys when price breaks above a rolling high by breakoutPct.
 * - Sells when take-profit reached, trailing drop hit, or stop-loss hit.
 */
class MomentumTrader
{
    public:
        struct Config
        {
            std::string name;
            utime_t windowSize = 6_Hours;
            pp_t breakoutPct = 20_PercentagePoints; // 0.20%
            pp_t takeProfitPct = 50_PercentagePoints; // 0.50%
            pp_t trailingDrop = 50_PercentagePoints; // 0.50%
            pp_t stopLossPct = 100_PercentagePoints; // 1.0%
            utime_t minActionSpacing = 10_Seconds;
            utime_t reentryCooldown = 10_Minutes;
            usd_t betSize = 200_Dollars;
            usd_t capitalCap = 0_Dollars; // 0 = unlimited
        };

        MomentumTrader(
            BotContext &ctx,
            Config conf);
        MomentumTrader(MomentumTrader &&) = delete;
        MomentumTrader(const MomentumTrader &) = delete;
        MomentumTrader &operator=(MomentumTrader &&) = delete;
        MomentumTrader &operator=(const MomentumTrader &) = delete;
        ~MomentumTrader() = default;

        void process(
            const BtcPrice &price);

    private:
        void prune(
            SteadyClock::TimePoint now);
        usd_t getHigh() const;
        usd_t getLow() const;
        bool buy(
            usd_t price);
        bool sell(
            usd_t price);

        BotContext &ctx;
        Config conf;

        std::deque<std::pair<SteadyClock::TimePoint, usd_t>> window;
        btc_t holding;
        usd_t lastBuyPrice;
        usd_t highWater;
        SteadyClock::TimePoint lastAction;
        SteadyClock::TimePoint lastExit;
        std::string openUuid;
    };

}
