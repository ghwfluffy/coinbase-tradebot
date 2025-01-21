#include <gtb/CoinbaseInit.h>

using namespace gtb;

CoinbaseInit::CoinbaseInit()
{
    walletInit = false;
    btcPriceInit = false;
    orderBookInit = false;
}

CoinbaseInit::operator bool() const
{
    return btcPriceInit && walletInit && orderBookInit;
}

void CoinbaseInit::setBtcInit()
{
    if (!btcPriceInit)
    {
        btcPriceInit = true;
        if (*this)
            updated();
    }
}

void CoinbaseInit::setWalletInit()
{
    if (!walletInit)
    {
        walletInit = true;
        if (*this)
            updated();
    }
}

void CoinbaseInit::setOrderBookInit()
{
    if (!orderBookInit)
    {
        orderBookInit = true;
        if (*this)
            updated();
    }
}
