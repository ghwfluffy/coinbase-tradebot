#pragma once

#include <gtb/OrderPairTrader.h>

namespace gtb
{

/**
 * Waits for bitcoin to fluctuate and tries to buy low and sell high
 */
class SpreadTrader : public OrderPairTrader
{
    public:
        struct Config
        {
            // Name for algorithm
            std::string name;
            // Percentage points spread (10 = 0.1%)
            uint32_t spread = 1000;
            // Cents to buy each spread
            uint32_t cents = 0;
            // How many spreads to maintain
            uint32_t num_pairs = 1;
            // How much to buffer between each spred (25 = 25% of spread value)
            uint32_t buffer_percent = 25;
        };

        SpreadTrader(
            BotContext &ctx,
            Config conf);
        SpreadTrader(SpreadTrader &&) = delete;
        SpreadTrader(const SpreadTrader &) = delete;
        SpreadTrader &operator=(SpreadTrader &&) = delete;
        SpreadTrader &operator=(const SpreadTrader &) = delete;
        ~SpreadTrader() = default;

    protected:
        void handleNewPair(
            const BtcPrice &price) final;

    private:
        bool cancelPending(
            const BtcPrice &price);

        bool patientOverride() const;

        Config conf;
};

}
