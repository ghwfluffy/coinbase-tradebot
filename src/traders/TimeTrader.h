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
            // How many seconds to sample for the high/low window
            uint32_t seconds = 0;
            // Minimum spread (10 = 0.1%)
            uint32_t minSpread = 1000;
            // Amount of spread to pad from the min/max
            uint32_t paddingSpread = 100;
            // How many spreads to maintain
            uint32_t numPairs = 1;
            // The maximum price we will queue a sale for
            uint32_t maxValue = 105'000'00;
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

        bool patientOverride() const;

        Config conf;

        uint64_t startTime;
        uint32_t lowest;
        uint32_t highest;
};

}
