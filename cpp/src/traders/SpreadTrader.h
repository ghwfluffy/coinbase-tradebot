#pragma once

#include <gtb/Database.h>
#include <gtb/BotContext.h>

#include <gtb/BtcPrice.h>
#include <gtb/CoinbaseOrderBook.h>

#include <gtb/OrderPair.h>
#include <gtb/OrderPairStateMachine.h>

#include <list>
#include <mutex>

namespace gtb
{

/**
 * Waits for bitcoin to fluctuate and tries to buy low and sell high
 */
class SpreadTrader
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

        void process(
            const BtcPrice &price);

        void process(
            const CoinbaseOrderBook &orderbook);

    private:
        void loadDatabase();

        void handleExistingPairs(
            bool force = false);

        void handleNewPair(
            const BtcPrice &price);

        bool cancelPending(
            const BtcPrice &price);

        bool patientOverride() const;

        BotContext &ctx;
        Config conf;
        Database db;
        OrderPairStateMachine stateMachine;

        std::mutex mtx;
        std::list<OrderPair> orderPairs;
        std::chrono::steady_clock::time_point nextTrade;
};

}
