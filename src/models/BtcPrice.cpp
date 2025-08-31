#include <gtb/BtcPrice.h>

using namespace gtb;

BtcPrice::BtcPrice()
{
}

usd_t BtcPrice::getPrice() const
{
    return price;
}

void BtcPrice::setPrice(usd_t price)
{
    if (this->price != price)
    {
        this->price = price;
        updated();
    }
}
