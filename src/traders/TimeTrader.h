#pragma once

#include <gtb/OrderPairTrader.h>

namespace gtb
{

/**
 * Finds the lowest and the highest values during a timeframe and uses those as a trade pair
 */
class TimeTrader : public OrderPairTrader
{
    public:
        struct Config : public BaseTraderConfig
        {
            // How long to sample for the high/low window
            utime_t sampleSize;
            // The difference between low and high that must exist to mark a trade
            pp_t minSpread = 10_Percent;
            // Amount of spread to pad from the min/max (trade spread = minSpread - paddingSpread)
            pp_t paddingSpread = 1_Percent;
            // How many spreads to maintain
            uint32_t numPairs = 1;
        };

        TimeTrader(
            BotContext &ctx,
            Config conf);
        TimeTrader(TimeTrader &&) = delete;
        TimeTrader(const TimeTrader &) = delete;
        TimeTrader &operator=(TimeTrader &&) = delete;
        TimeTrader &operator=(const TimeTrader &) = delete;
        ~TimeTrader() = default;

    protected:
        void handleNewPair(
            const BtcPrice &price) final;

    private:
        void reset();

        Config conf;

        utime_t startTime;
        usd_t lowest;
        usd_t highest;
};

}
