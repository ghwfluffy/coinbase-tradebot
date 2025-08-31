#pragma once

#include <gtb/DataModel.h>
#include <gtb/IntLiterals.h>

#include <mutex>
#include <stdint.h>

namespace gtb
{

/**
 * Track profits and losses
 */
class Profits : public DataModel
{
    public:
        Profits() = default;
        Profits(Profits &&);
        Profits(const Profits &) = delete;
        Profits &operator=(Profits &&) = delete;
        Profits &operator=(const Profits &) = delete;
        ~Profits() final = default;

        int64_t getProfit() const;
        usd_t getVolume() const;

        void addOrderPair(
            usd_t purchased,
            usd_t sold,
            usd_t buyFees,
            usd_t sellFees);

        struct Data
        {
            usd_t purchased;
            usd_t sold;
            usd_t buyFees;
            usd_t sellFees;
            // Profit (cents) = (sold - sellFees - purchased - buyFees)
            int64_t getProfit() const;
        };

        void addOrderPair(
            Data data);

        Data getData() const;

    private:
        std::mutex mtx;
        Data data;
};

}
