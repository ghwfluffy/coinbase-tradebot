#include <gtb/CoinbaseWallet.h>

using namespace gtb;

CoinbaseWallet::CoinbaseWallet()
{
    usd = 0;
    btc = 0;
}

uint32_t CoinbaseWallet::getUsdCents() const
{
    return usd;
}

uint64_t CoinbaseWallet::getBtcSatoshi() const
{
    return btc;
}

void CoinbaseWallet::update(
    uint32_t usd,
    uint64_t btc)
{
    if (usd != this->usd || btc != this->btc)
    {
        this->usd = usd;
        this->btc = btc;
        updated();
    }
}
