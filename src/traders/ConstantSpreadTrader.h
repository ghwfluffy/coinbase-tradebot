#pragma once

#include <gtb/OrderPairTrader.h>
#include <gtb/BaseTraderConfig.h>
#include <gtb/IntLiterals.h>

namespace gtb
{

/**
 * Waits for bitcoin to fluctuate and tries to buy low and sell high.
 * Based on a dollar value of bitcoin change instead of percent.
 */
class ConstantSpreadTrader : public OrderPairTrader
{
    public:
        struct Config : public BaseTraderConfig
        {
            usd_t windowSize = 20_Dollars;
            // How much to buffer between each spread (25 = 25% of window value)
            pp_t buffer = 25_Percent;
        };

        ConstantSpreadTrader(
            BotContext &ctx,
            Config conf);
        ConstantSpreadTrader(ConstantSpreadTrader &&) = delete;
        ConstantSpreadTrader(const ConstantSpreadTrader &) = delete;
        ConstantSpreadTrader &operator=(ConstantSpreadTrader &&) = delete;
        ConstantSpreadTrader &operator=(const ConstantSpreadTrader &) = delete;
        ~ConstantSpreadTrader() = default;

    protected:
        void handleNewPair(
            const BtcPrice &price) final;

        void handleComplete(
            OrderPair &pair) final;

    private:
        Config conf;
        usd_t lastBuyPrice;
};

}
