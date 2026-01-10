#include <gtb/Profits.h>
#include <gtb/Log.h>
#include <gtb/IntegerUtils.h>

#include <algorithm>

using namespace gtb;

Profits::Profits(Profits &&rhs)
    : traders(std::move(rhs.traders))
{
}

big_usd_t Profits::getProfit(
    usd_t curPrice) const
{
    big_usd_t ret;
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mtx));
    for (auto &[trader, data] : traders)
        ret += data.getProfit(curPrice);
    return ret;
}

big_usd_t Profits::getVolume() const
{
    big_usd_t ret;
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mtx));
    for (auto &[trader, data] : traders)
        ret += data.getVolume();
    return ret;
}

std::map<std::string, Profits::TraderData> Profits::getAllTraderData() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mtx));
    return traders;
}

btc_t Profits::TraderData::getPending() const
{
    big_btc_t pending = buyBtc - sellBtc;
    if (pending.value().isNegative())
        return {};
    return btc_t(pending.value().toUint64());
}

big_usd_t Profits::TraderData::getVolume() const
{
    return buyUsd + sellUsd + buyFees + sellFees;
}

big_usd_t Profits::TraderData::getProfit(
    usd_t curPrice) const
{
    big_btc_t pending = buyBtc - sellBtc;
    BigInt pendingVal;
    if (!!pending && curPrice)
    {
        pendingVal = BigInt(curPrice.value()) * pending.value();
        pendingVal /= BigInt(btc_t(1_Bitcoins).value());
    }

    BigInt total = sellUsd.value();
    total += pendingVal;
    total -= buyUsd.value();
    total -= buyFees.value();
    total -= sellFees.value();
    return big_usd_t(std::move(total));
}

void Profits::recordBuyFill(
    const std::string &trader,
    btc_t quantity,
    usd_t beforeFees,
    usd_t fees)
{
    if (!quantity || !beforeFees)
        return;

    {
        std::lock_guard<std::mutex> lock(mtx);
        TraderData &d = traders[trader];
        d.buyBtc += quantity;
        d.buyUsd += beforeFees;
        d.buyFees += fees;
        d.buys++;
    }

    updated();
}

void Profits::recordSellFill(
    const std::string &trader,
    btc_t quantity,
    usd_t beforeFees,
    usd_t fees)
{
    if (!quantity || !beforeFees)
        return;

    {
        std::lock_guard<std::mutex> lock(mtx);
        TraderData &d = traders[trader];
        d.sellBtc += quantity;
        d.sellUsd += beforeFees;
        d.sellFees += fees;
        d.sells++;
    }

    updated();
}
