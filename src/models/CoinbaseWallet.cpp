#include <gtb/CoinbaseWallet.h>

using namespace gtb;

CoinbaseWallet::Data::operator bool() const
{
    return usd || btc;
}

CoinbaseWallet::operator bool() const
{
    return bool(values);
}

usd_t CoinbaseWallet::getUsd() const
{
    return values.usd;
}

btc_t CoinbaseWallet::getBtc() const
{
    return values.btc;
}

usd_t CoinbaseWallet::getAvailUsd() const
{
    return values.usd - values.onHoldUsd;
}

btc_t CoinbaseWallet::getAvailBtc() const
{
    return values.btc - values.onHoldBtc;
}

void CoinbaseWallet::update(
    usd_t usd,
    btc_t btc,
    usd_t onHoldUsd,
    btc_t onHoldBtc)
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
    if (data.usd || data.btc)
        update(data.usd, data.btc, data.onHoldUsd, data.onHoldBtc);
}

CoinbaseWallet::Data CoinbaseWallet::getData() const
{
    return values;
}
