#pragma once

#include <gtb/Database.h>
#include <gtb/BotContext.h>

#include <gtb/BtcPrice.h>
#include <gtb/SteadyClock.h>

#include <set>
#include <mutex>

namespace gtb
{

/**
 * Constantly tries to buy or sell on minor price changes
 */
class ConstantTrader
{
    public:
        ConstantTrader(
            BotContext &ctx,
            usd_t betSize,
            usd_t windowSize);
        ConstantTrader(ConstantTrader &&) = delete;
        ConstantTrader(const ConstantTrader &) = delete;
        ConstantTrader &operator=(ConstantTrader &&) = delete;
        ConstantTrader &operator=(const ConstantTrader &) = delete;
        ~ConstantTrader() = default;

        void process(
            const BtcPrice &price);

    private:
        void queueBuy(
            const BtcPrice &price);

        void queueSell(
            const BtcPrice &price);

        bool checkMax();

        BotContext &ctx;
        usd_t betSize;
        usd_t windowSize;

        std::mutex mtx;
        usd_t lastBuyPrice;
        std::set<usd_t> sellPrices;

        size_t buyCount;
        size_t sellCount;

        usd_t maxWallet;
        SteadyClock::TimePoint pauseTimer;
};

}
