#pragma once

#include <gtb/DataModel.h>

#include <stdint.h>

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

        int32_t getProfit() const;
        void setProfit(int32_t profit);

    private:
        int32_t profit;
};

}
