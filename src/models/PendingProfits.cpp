#include <gtb/PendingProfits.h>

using namespace gtb;

PendingProfits::PendingProfits()
{
    profit = 0;
}

int64_t PendingProfits::getProfit() const
{
    return profit;
}

void PendingProfits::setProfit(usd_t spent, usd_t assets)
{
    this->profit = static_cast<int64_t>(assets.value()) - static_cast<int64_t>(spent.value());
    updated();
}
