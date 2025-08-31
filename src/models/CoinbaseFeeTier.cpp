#include <gtb/CoinbaseFeeTier.h>

using namespace gtb;

CoinbaseFeeTier::CoinbaseFeeTier()
{
}

pp_t CoinbaseFeeTier::getFeeTier() const
{
    return fees;
}

void CoinbaseFeeTier::setFeeTier(pp_t time)
{
    if (this->fees != time)
    {
        this->fees = time;
        updated();
    }
}
