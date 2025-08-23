#include <gtb/BtcPrice.h>

using namespace gtb;

BtcPrice::BtcPrice()
{
    cents = 0;
}

uint32_t BtcPrice::getCents() const
{
    return cents;
}

void BtcPrice::setCents(uint32_t cents)
{
    if (this->cents != cents)
    {
        this->cents = cents;
        updated();
    }
}
