#pragma once

#include <gtb/BotContext.h>
#include <gtb/BtcPrice.h>
#include <gtb/CoinbaseOrder.h>
#include <gtb/IntLiterals.h>

#include <deque>

namespace gtb
{

/**
 * Simple moving-average crossover trader.
 * Long-only: buys when short MA cleanly crosses above long MA,
 * sells on cross-under, take-profit, or stop-loss.
 */
class MovingAverageTrader
{
    public:
        struct Config
        {
            std::string name;
            size_t shortWindow = 12; // candles
            size_t longWindow = 48;
            utime_t candleSize = 5_Minutes;
            pp_t entryBuffer = 20_PercentagePoints;  // 0.20% over long to enter
            pp_t exitBuffer = 10_PercentagePoints;   // 0.10% under long to exit
            pp_t takeProfitPct = 150_PercentagePoints; // 1.50%
            pp_t stopLossPct = 80_PercentagePoints;    // 0.80%
            pp_t trailingDrop = 60_PercentagePoints;   // 0.60%
            utime_t minSpacing = 10_Seconds;
            usd_t betSize = 250_Dollars;
            usd_t capitalCap = 0_Dollars; // 0 = unlimited
        };

        MovingAverageTrader(
            BotContext &ctx,
            Config conf);
        MovingAverageTrader(MovingAverageTrader &&) = delete;
        MovingAverageTrader(const MovingAverageTrader &) = delete;
        MovingAverageTrader &operator=(MovingAverageTrader &&) = delete;
        MovingAverageTrader &operator=(const MovingAverageTrader &) = delete;
        ~MovingAverageTrader() = default;

        void process(
            const BtcPrice &price);

    private:
        void checkCandle(
            const BtcPrice &price);
        double getShortAvg() const;
        double getLongAvg() const;
        bool buy(
            usd_t price);
        bool sell(
            usd_t price);

        BotContext &ctx;
        Config conf;

        struct Candle
        {
            SteadyClock::TimePoint start;
            usd_t closePrice;
        };
        std::deque<Candle> candles;
        btc_t holding;
        usd_t lastBuy;
        usd_t highWater;
        SteadyClock::TimePoint lastAction;
        std::string openUuid;
    };

}
