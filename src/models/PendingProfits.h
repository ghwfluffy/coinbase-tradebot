#pragma once

#include <gtb/DataModel.h>

#include <gtb/IntLiterals.h>

namespace gtb
{

/**
 * Track profits and losses for pairs that are still in progress
 */
class PendingProfits : public DataModel
{
    public:
        PendingProfits();
        PendingProfits(PendingProfits &&) = default;
        PendingProfits(const PendingProfits &) = delete;
        PendingProfits &operator=(PendingProfits &&) = delete;
        PendingProfits &operator=(const PendingProfits &) = delete;
        ~PendingProfits() final = default;

        int64_t getProfit() const;
        void setProfit(usd_t spent, usd_t assets);

    private:
        int64_t profit;
};

}
