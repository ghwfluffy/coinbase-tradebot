#pragma once

#include <gtb/DataModel.h>
#include <gtb/IntLiterals.h>

namespace gtb
{

class CoinbaseFeeTier : public DataModel
{
    public:
        CoinbaseFeeTier();
        CoinbaseFeeTier(CoinbaseFeeTier &&) = default;
        CoinbaseFeeTier(const CoinbaseFeeTier &) = delete;
        CoinbaseFeeTier &operator=(CoinbaseFeeTier &&) = delete;
        CoinbaseFeeTier &operator=(const CoinbaseFeeTier &) = delete;
        ~CoinbaseFeeTier() final = default;

        pp_t getFeeTier() const;
        void setFeeTier(pp_t fees);

    private:
        pp_t fees;
};

}
