#pragma once

#include <gtb/DataModel.h>
#include <gtb/IntLiterals.h>
#include <gtb/StrongTypedBigInt.h>

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

        big_usd_t getProfit() const;
        big_usd_t getVolume() const;

        void addOrderPair(
            big_usd_t purchased,
            big_usd_t sold,
            big_usd_t buyFees,
            big_usd_t sellFees);

        void addOrderPair(
            usd_t purchased,
            usd_t sold,
            usd_t buyFees,
            usd_t sellFees);

        struct Data
        {
            big_usd_t purchased;
            big_usd_t sold;
            big_usd_t buyFees;
            big_usd_t sellFees;
            // Profit = (sold - sellFees - purchased - buyFees)
            big_usd_t getProfit() const;
        };

        void addOrderPair(
            Data data);

        Data getData() const;

    private:
        std::mutex mtx;
        Data data;
};

}
