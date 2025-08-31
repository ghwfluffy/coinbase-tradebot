#pragma once

#include <gtb/CoinbaseInterface.h>
#include <gtb/BotContext.h>

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
            pp_t feeTier = 35_PercentagePoints);
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

    private:
        BotContext &ctx;
        pp_t feeTier;
};

}
