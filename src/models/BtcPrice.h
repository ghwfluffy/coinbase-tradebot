#pragma once

#include <gtb/DataModel.h>
#include <gtb/IntLiterals.h>

namespace gtb
{

class BtcPrice : public DataModel
{
    public:
        BtcPrice();
        BtcPrice(BtcPrice &&) = default;
        BtcPrice(const BtcPrice &) = delete;
        BtcPrice &operator=(BtcPrice &&) = delete;
        BtcPrice &operator=(const BtcPrice &) = delete;
        ~BtcPrice() final = default;

        usd_t getPrice() const;
        void setPrice(usd_t price);

    private:
        usd_t price;
};

}
