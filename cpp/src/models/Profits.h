#pragma once

#include <gtb/DataModel.h>

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

        int32_t getProfit() const;

        void addOrderPair(
            uint64_t purchased,
            uint64_t sold,
            uint64_t buyFees,
            uint64_t sellFees);

        struct Data
        {
            // In pico dollars (10^-13)
            uint64_t purchased = 0;
            uint64_t sold = 0;
            uint64_t buyFees = 0;
            uint64_t sellFees = 0;
            // Profit (cents) = (sold - sellFees - purchased - buyFees)
            int32_t getProfit() const;
        };

        void addOrderPair(
            Data data);

        Data getData() const;

    private:
        std::mutex mtx;
        Data data;
};

}
