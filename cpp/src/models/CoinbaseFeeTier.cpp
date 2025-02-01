#include <gtb/CoinbaseFeeTier.h>

using namespace gtb;

CoinbaseFeeTier::CoinbaseFeeTier()
{
    fees = 0;
}

uint32_t CoinbaseFeeTier::getFeeTier() const
{
    return fees;
}

void CoinbaseFeeTier::setFeeTier(uint32_t time)
{
    if (this->fees != time)
    {
        this->fees = time;
        updated();
    }
}
