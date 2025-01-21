#pragma once

#include <gtb/DataModel.h>

#include <stdint.h>

namespace gtb
{

class CoinbaseWallet : public DataModel
{
    public:
        CoinbaseWallet();
        CoinbaseWallet(CoinbaseWallet &&) = default;
        CoinbaseWallet(const CoinbaseWallet &) = delete;
        CoinbaseWallet &operator=(CoinbaseWallet &&) = delete;
        CoinbaseWallet &operator=(const CoinbaseWallet &) = delete;
        ~CoinbaseWallet() final = default;

        uint32_t getUsdCents() const;
        uint64_t getBtcSatoshi() const;

        void update(
            uint32_t usd,
            uint64_t btc);

    private:
        uint32_t usd;
        uint64_t btc;
};

}
