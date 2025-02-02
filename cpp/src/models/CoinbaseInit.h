#pragma once

#include <gtb/DataModel.h>

namespace gtb
{

/**
 * Use this to check if the current wallet and BTC price are initialized
 * so you don't try to start performing calculations on uninitialized values
 */
class CoinbaseInit : public DataModel
{
    public:
        CoinbaseInit();
        CoinbaseInit(CoinbaseInit &&) = default;
        CoinbaseInit(const CoinbaseInit &) = delete;
        CoinbaseInit &operator=(CoinbaseInit &&) = delete;
        CoinbaseInit &operator=(const CoinbaseInit &) = delete;
        ~CoinbaseInit() final = default;

        operator bool() const;

        void setFullInit();

        void setBtcInit();
        void setWalletInit();
        void setFeeTierInit();
        void setOrderBookInit();

    private:
        bool walletInit;
        bool feeTierInit;
        bool btcPriceInit;
        bool orderBookInit;
};

}
