#include <gtb/Profits.h>

using namespace gtb;

Profits::Profits(Profits &&rhs)
    : data(rhs.data)
{
}

int32_t Profits::getProfit() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mtx));
    return data.getProfit();
}

Profits::Data Profits::getData() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mtx));
    return data;
}

void Profits::addOrderPair(
    uint64_t purchased,
    uint64_t sold,
    uint64_t buyFees,
    uint64_t sellFees)
{
    if (purchased == 0 || sold == 0)
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

int32_t Profits::Data::getProfit() const
{
    uint64_t aboveZero = sold;
    uint64_t belowZero = 0;

    #define SUBTRACT(x) \
    if (aboveZero >= x) \
        aboveZero -= x; \
    else \
    { \
        belowZero += (x - aboveZero); \
        aboveZero = 0; \
    }

    SUBTRACT(sellFees)
    SUBTRACT(purchased)
    SUBTRACT(buyFees)

    if (belowZero > 0)
        return static_cast<int32_t>(static_cast<int32_t>(belowZero / 100'000'000'000ULL) * -1);

    return static_cast<int32_t>(aboveZero / 100'000'000'000ULL);
}
