#pragma once

#include <gtb/DataModel.h>

#include <stdint.h>

namespace gtb
{

class CoinbaseWallet : public DataModel
{
    public:
        CoinbaseWallet() = default;
        CoinbaseWallet(CoinbaseWallet &&) = default;
        CoinbaseWallet(const CoinbaseWallet &) = delete;
        CoinbaseWallet &operator=(CoinbaseWallet &&) = delete;
        CoinbaseWallet &operator=(const CoinbaseWallet &) = delete;
        ~CoinbaseWallet() final = default;

        operator bool() const;

        uint32_t getUsdCents() const;
        uint64_t getBtcSatoshi() const;

        void update(
            uint32_t usd,
            uint64_t btc);

        struct Data
        {
            operator bool() const;

            uint32_t usd = 0;
            uint64_t btc = 0;
        };

        void update(
            Data data);

    private:
        Data values;
};

}
