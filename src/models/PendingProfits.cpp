#include <gtb/PendingProfits.h>

using namespace gtb;

PendingProfits::PendingProfits()
{
    profit = 0;
}

int32_t PendingProfits::getProfit() const
{
    return profit;
}

void PendingProfits::setProfit(int32_t profit)
{
    this->profit = profit;
    updated();
}
