#pragma once

#include <gtb/BotContext.h>

#include <gtb/BtcPrice.h>

#include <mutex>

namespace gtb
{

/**
 * Write BTC price updates to the database
 */
class BtcHistoricalWriter
{
    public:
        BtcHistoricalWriter(
            BotContext &ctx);
        BtcHistoricalWriter(BtcHistoricalWriter &&) = delete;
        BtcHistoricalWriter(const BtcHistoricalWriter &) = delete;
        BtcHistoricalWriter &operator=(BtcHistoricalWriter &&) = delete;
        BtcHistoricalWriter &operator=(const BtcHistoricalWriter &) = delete;
        ~BtcHistoricalWriter() = default;

        void process(
            const BtcPrice &price);

    private:
        BotContext &ctx;

        std::mutex mtx;
        uint64_t prevTime;
};

}
