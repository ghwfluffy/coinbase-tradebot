#pragma once

#include <gtb/BotContext.h>
#include <gtb/BtcPrice.h>

namespace gtb
{

/**
 * Executes user trades that are active in the orderbook when the BTC price
 * is in range to simulate a buy/sell being completed in the marketplace.
 */
class MockUserTrades
{
    public:
        MockUserTrades(
            BotContext &ctx);
        MockUserTrades(MockUserTrades &&) = delete;
        MockUserTrades(const MockUserTrades &) = delete;
        MockUserTrades &operator=(MockUserTrades &&) = delete;
        MockUserTrades &operator=(const MockUserTrades &) = delete;
        ~MockUserTrades() = default;

        void process(
            const BtcPrice &price);

    private:
        BotContext &ctx;
};

}
