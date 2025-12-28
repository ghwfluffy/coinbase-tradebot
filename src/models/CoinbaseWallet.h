#pragma once

#include <gtb/DataModel.h>
#include <gtb/IntLiterals.h>

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

        usd_t getUsd() const;
        btc_t getBtc() const;

        usd_t getAvailUsd() const;
        btc_t getAvailBtc() const;
        usd_t getOnHoldUsd() const { return values.onHoldUsd; }
        btc_t getOnHoldBtc() const { return values.onHoldBtc; }

        void update(
            usd_t usd,
            btc_t btc,
            usd_t onHoldUsd,
            btc_t onHoldBtc);

        struct Data
        {
            operator bool() const;

            // Includes on hold amounts (decipicodollars)
            usd_t usd;
            btc_t btc;

            usd_t onHoldUsd;
            btc_t onHoldBtc;
        };

        void update(
            Data data);

        Data getData() const;

    private:
        Data values;
};

}
