#pragma once

#include <gtb/CoinbaseOrder.h>
#include <gtb/CoinbaseWallet.h>

namespace gtb
{

/**
 * Interface for coinbase interactions
 */
class CoinbaseInterface
{
    public:
        virtual ~CoinbaseInterface() = default;

        virtual CoinbaseOrder getOrder(
            const std::string &uuid) = 0;

        virtual bool submitOrder(
            CoinbaseOrder &order) = 0;

        virtual bool cancelOrder(
            const std::string &uuid) = 0;

        virtual CoinbaseWallet::Data getWallet() = 0;

        virtual uint32_t getFeeTier() = 0;

    protected:
        CoinbaseInterface() = default;
        CoinbaseInterface(CoinbaseInterface &&) = default;
        CoinbaseInterface(const CoinbaseInterface &) = default;
        CoinbaseInterface &operator=(CoinbaseInterface &&) = default;
        CoinbaseInterface &operator=(const CoinbaseInterface &) = default;
};

}
