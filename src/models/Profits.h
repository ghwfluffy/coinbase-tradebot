#pragma once

#include <gtb/DataModel.h>
#include <gtb/IntLiterals.h>
#include <gtb/StrongTypedBigInt.h>

#include <map>
#include <mutex>
#include <stdint.h>
#include <string>

namespace gtb
{

/**
 * Track profits and losses
 */
class Profits : public DataModel
{
    public:
        Profits() = default;
        Profits(Profits &&);
        Profits(const Profits &) = delete;
        Profits &operator=(Profits &&) = delete;
        Profits &operator=(const Profits &) = delete;
        ~Profits() final = default;

        struct TraderData
        {
            big_btc_t buyBtc;
            big_usd_t buyUsd;
            big_btc_t sellBtc;
            big_usd_t sellUsd;
            big_usd_t buyFees;
            big_usd_t sellFees;

            btc_t getPending() const;
            big_usd_t getVolume() const;
            big_usd_t getProfit(usd_t curPrice) const;
        };

        big_usd_t getVolume() const;
        big_usd_t getProfit(usd_t curPrice) const;

        // TODO: Remove?
        // Per-trader view; returns zeroed data if no such trader exists.
        TraderData getTraderData(
            const std::string &trader) const;

        // Full per-trader breakdown.
        std::map<std::string, TraderData> getAllTraderData() const;

        void recordBuyFill(
            const std::string &trader,
            btc_t quantity,
            usd_t beforeFees,
            usd_t fees);

        void recordSellFill(
            const std::string &trader,
            btc_t quantity,
            usd_t beforeFees,
            usd_t fees);

    private:
        std::mutex mtx;
        std::map<std::string, TraderData> traders;
};

}
