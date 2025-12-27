#pragma once

#include <gtb/CoinbaseInterface.h>
#include <gtb/BotContext.h>
#include <gtb/IntLiterals.h>

#include <deque>

namespace gtb
{

/**
 * Pretend coinbase that just reads/writes updates to the local state
 */
class MockCoinbase : public CoinbaseInterface
{
    public:
        MockCoinbase(
            BotContext &ctx,
            big_usd_t dayZeroVolume = big_usd_t());
        MockCoinbase(MockCoinbase &&) = default;
        MockCoinbase(const MockCoinbase &) = delete;
        MockCoinbase &operator=(MockCoinbase &&) = delete;
        MockCoinbase &operator=(const MockCoinbase &) = delete;
        ~MockCoinbase() final = default;

        CoinbaseOrder getOrder(
            const std::string &uuid) final;

        bool submitOrder(
            CoinbaseOrder &order) final;

        bool cancelOrder(
            const std::string &uuid) final;

        CoinbaseWallet::Data getWallet() final;

        pp_t getFeeTier() final;

        big_usd_t getVolume() final;

        void recordVolume(
            big_usd_t amount,
            utime_t time) final;

    private:
        BotContext &ctx;

        struct VolumeBucket
        {
            utime_t hour;
            big_usd_t volume;
        };

        void pruneBuckets(
            const std::lock_guard<std::mutex> &lock);
        utime_t getHourStart(
            utime_t time) const;
        pp_t feeTierForVolume(
            big_usd_t vol) const;
        pp_t feeTierForVolume(
            uint64_t dollars) const;

        std::mutex mtxVolume;
        big_usd_t dayZeroVolume;
        std::deque<VolumeBucket> buckets;
};

}
