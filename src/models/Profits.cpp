#include <gtb/Profits.h>
#include <gtb/Log.h>
#include <gtb/IntegerUtils.h>

using namespace gtb;

Profits::Profits(Profits &&rhs)
    : data(rhs.data)
{
}

big_usd_t Profits::getProfit() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mtx));
    return data.getProfit();
}

big_usd_t Profits::getVolume() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mtx));
    return data.purchased + data.sold;
}

Profits::Data Profits::getData() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mtx));
    return data;
}

void Profits::addOrderPair(
    usd_t purchased,
    usd_t sold,
    usd_t buyFees,
    usd_t sellFees)
{
    addOrderPair(big_usd_t(purchased), big_usd_t(sold), big_usd_t(buyFees), big_usd_t(sellFees));
}

void Profits::addOrderPair(
    big_usd_t purchased,
    big_usd_t sold,
    big_usd_t buyFees,
    big_usd_t sellFees)
{
    if (!purchased && !sold)
        return;

    // Atomic
    {
        std::lock_guard<std::mutex> lock(mtx);
        data.purchased += purchased;
        data.sold += sold;
        data.buyFees += buyFees;
        data.sellFees += sellFees;
    }

    updated();
}

void Profits::addOrderPair(
    Data data)
{
    addOrderPair(data.purchased, data.sold, data.buyFees, data.sellFees);
}

big_usd_t Profits::Data::getProfit() const
{
    big_usd_t profit = sold;
    profit -= sellFees;
    profit -= purchased;
    profit -= buyFees;
    return profit;
}
