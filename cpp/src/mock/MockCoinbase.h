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
            uint32_t feeTier = 35);
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

        uint32_t getFeeTier() final;

    private:
        BotContext &ctx;
        uint32_t feeTier;
};

}
