#include <gtb/Profits.h>
#include <gtb/Log.h>
#include <gtb/IntegerUtils.h>

using namespace gtb;

Profits::Profits(Profits &&rhs)
    : data(rhs.data)
{
    volumeCents = 0;
}

int64_t Profits::getProfit() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mtx));
    return data.getProfit();
}

usd_t Profits::getVolume() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mtx));
    return data.purchased + data.sold;
}

uint64_t Profits::getVolumeCents() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mtx));
    return volumeCents;
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
    if (!purchased && !sold)
        return;

    // Atomic
    {
        std::lock_guard<std::mutex> lock(mtx);
        volumeCents += purchased / 1_Cents;
        volumeCents += sold / 1_Cents;

        data.purchased += purchased;
        data.sold += sold;
        data.buyFees += buyFees;
        data.sellFees += sellFees;

        // Normalize (prevent rollovers)
        if (data.purchased > data.sold)
        {
            data.purchased = data.purchased - data.sold;
            data.sold = 0_Dollars;
        }
        else
        {
            data.sold = data.sold - data.purchased;
            data.purchased = 0_Dollars;
        }
        if (data.buyFees > data.sellFees)
        {
            data.buyFees = data.buyFees - data.sellFees;
            data.sellFees = 0_Dollars;
        }
        else
        {
            data.sellFees = data.sellFees - data.buyFees;
            data.buyFees = 0_Dollars;
        }
    }

    updated();
}

void Profits::addOrderPair(
    Data data)
{
    addOrderPair(data.purchased, data.sold, data.buyFees, data.sellFees);
}

int64_t Profits::Data::getProfit() const
{
    usd_t aboveZero = sold;
    usd_t belowZero = {};

    #define SUBTRACT(x) \
    if (aboveZero >= x) \
        aboveZero -= x; \
    else \
    { \
        belowZero += (x - aboveZero); \
        aboveZero = {}; \
    }

    //SUBTRACT(sellFees)
    SUBTRACT(purchased)
    SUBTRACT(buyFees)

    if (belowZero)
        return static_cast<int64_t>(belowZero.value()) * -1L;

    return static_cast<int64_t>(aboveZero.value());
}
