#pragma once

#include <gtb/OrderPairTrader.h>

#include <gtb/IntLiterals.h>

namespace gtb
{

/**
 * Buys at a specific price and sells at a specific price
 */
class StaticTrader : public OrderPairTrader
{
    public:
        struct Config : public BaseTraderConfig
        {
            // Static buy price
            usd_t buy = 100'000_Dollars;
            // Static sell price
            usd_t sell = 200'000_Dollars;
        };

        StaticTrader(
            BotContext &ctx,
            Config conf);
        StaticTrader(StaticTrader &&) = delete;
        StaticTrader(const StaticTrader &) = delete;
        StaticTrader &operator=(StaticTrader &&) = delete;
        StaticTrader &operator=(const StaticTrader &) = delete;
        ~StaticTrader() = default;

    protected:
        void handleNewPair(
            const BtcPrice &price) final;

    private:
        Config conf;
};

}
