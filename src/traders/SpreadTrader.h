#pragma once

#include <gtb/OrderPairTrader.h>
#include <gtb/BaseTraderConfig.h>
#include <gtb/IntLiterals.h>

namespace gtb
{

/**
 * Waits for bitcoin to fluctuate and tries to buy low and sell high
 */
class SpreadTrader : public OrderPairTrader
{
    public:
        struct Config : public BaseTraderConfig
        {
            // Percentage points spread (10 = 0.1%)
            pp_t spread = 10_Percent;
            // How many spreads to maintain
            uint32_t num_pairs = 1;
            // How much to buffer between each spred (25 = 25% of spread value)
            pp_t buffer_percent = 25_Percent;
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
        Config conf;
};

}
