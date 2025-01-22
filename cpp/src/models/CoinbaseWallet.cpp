#include <gtb/CoinbaseWallet.h>

using namespace gtb;

CoinbaseWallet::Data::operator bool() const
{
    return usd > 0 || btc > 0;
}

CoinbaseWallet::operator bool() const
{
    return bool(values);
}

uint32_t CoinbaseWallet::getUsdCents() const
{
    return values.usd;
}

uint64_t CoinbaseWallet::getBtcSatoshi() const
{
    return values.btc;
}

void CoinbaseWallet::update(
    uint32_t usd,
    uint64_t btc)
{
    if (usd != values.usd || btc != values.btc)
    {
        values.usd = usd;
        values.btc = btc;
        updated();
    }
}

void CoinbaseWallet::update(
    Data data)
{
    if (data.usd > 0 || data.btc > 0)
        update(data.usd, data.btc);
}
