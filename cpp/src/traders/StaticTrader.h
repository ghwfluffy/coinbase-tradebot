#pragma once

#include <gtb/OrderPairTrader.h>

namespace gtb
{

/**
 * Waits for bitcoin to fluctuate and tries to buy low and sell high
 */
class StaticTrader : public OrderPairTrader
{
    public:
        struct Config
        {
            // Name for algorithm
            std::string name;
            // How much to buy
            uint32_t cents = 0;
            // Static buy price
            uint32_t buyCents = 100'000'00;
            // Static sell price
            uint32_t sellCents = 200'000'00;
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
