#pragma once

#include <gtb/BotContext.h>
#include <gtb/BtcPrice.h>
#include <gtb/Database.h>
#include <gtb/CoinbaseOrderBook.h>

#include <chrono>

namespace gtb
{

/**
 * Updates the PendingProfits data model on changes to orderbook or BTC price.
 */
class PendingProfitsCalc
{
    public:
        PendingProfitsCalc(
            BotContext &ctx);
        PendingProfitsCalc(PendingProfitsCalc &&) = delete;
        PendingProfitsCalc(const PendingProfitsCalc &) = delete;
        PendingProfitsCalc &operator=(PendingProfitsCalc &&) = delete;
        PendingProfitsCalc &operator=(const PendingProfitsCalc &) = delete;
        ~PendingProfitsCalc() = default;

        void process(
            const CoinbaseOrderBook &orderbook);

        void process(
            const BtcPrice &price);

    private:
        void update();

        BotContext &ctx;
        Database db;

        std::chrono::steady_clock::time_point nextUpdate;
};

}
