#pragma once

#include <gtb/DataModel.h>

#include <stdint.h>

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

        uint32_t getCents() const;
        void setCents(uint32_t cents);

    private:
        uint32_t cents;
};

}
