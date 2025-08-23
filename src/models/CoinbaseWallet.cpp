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

uint32_t CoinbaseWallet::getAvailUsdCents() const
{
    return values.usd - values.onHoldUsd;
}

uint64_t CoinbaseWallet::getAvailBtcSatoshi() const
{
    return values.btc - values.onHoldBtc;
}

void CoinbaseWallet::update(
    uint32_t usd,
    uint64_t btc,
    uint32_t onHoldUsd,
    uint64_t onHoldBtc)
{
    if (usd != values.usd || btc != values.btc || onHoldUsd != values.onHoldUsd || onHoldBtc != values.onHoldBtc)
    {
        if (onHoldUsd > usd)
            onHoldUsd = usd;
        if (onHoldBtc > btc)
            onHoldBtc = btc;

        values.usd = usd;
        values.btc = btc;
        values.onHoldUsd = onHoldUsd;
        values.onHoldBtc = onHoldBtc;
        updated();
    }
}

void CoinbaseWallet::update(
    Data data)
{
    if (data.usd > 0 || data.btc > 0)
        update(data.usd, data.btc, data.onHoldUsd, data.onHoldBtc);
}

CoinbaseWallet::Data CoinbaseWallet::getData() const
{
    return values;
}
