#pragma once

#include <gtb/DataModel.h>

#include <stdint.h>

namespace gtb
{

// 1/100 of a percent (35 = 0.35%)
class CoinbaseFeeTier : public DataModel
{
    public:
        CoinbaseFeeTier();
        CoinbaseFeeTier(CoinbaseFeeTier &&) = default;
        CoinbaseFeeTier(const CoinbaseFeeTier &) = delete;
        CoinbaseFeeTier &operator=(CoinbaseFeeTier &&) = delete;
        CoinbaseFeeTier &operator=(const CoinbaseFeeTier &) = delete;
        ~CoinbaseFeeTier() final = default;

        uint32_t getFeeTier() const;
        void setFeeTier(uint32_t fees);

    private:
        uint32_t fees;
};

}
