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

        uint32_t getAvailUsdCents() const;
        uint64_t getAvailBtcSatoshi() const;

        void update(
            uint32_t usd,
            uint64_t btc,
            uint32_t onHoldUsd,
            uint64_t onHoldBtc);

        struct Data
        {
            operator bool() const;

            // Includes on hold amounts
            uint32_t usd = 0;
            uint64_t btc = 0;

            uint32_t onHoldUsd = 0;
            uint64_t onHoldBtc = 0;
        };

        void update(
            Data data);

        Data getData() const;

    private:
        Data values;
};

}
