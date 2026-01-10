#pragma once

#include <gtb/Database.h>
#include <gtb/OrderPair.h>

#include <unordered_map>

namespace gtb
{

/**
 * A set of order pairs being tracked.
 * This class is not reentrant.
 */
class TrackedOrderPairs
{
    public:
        TrackedOrderPairs(bool save);
        TrackedOrderPairs(TrackedOrderPairs &&) = delete;
        TrackedOrderPairs(const TrackedOrderPairs &) = delete;
        TrackedOrderPairs &operator=(TrackedOrderPairs &&) = delete;
        TrackedOrderPairs &operator=(const TrackedOrderPairs &) = delete;
        ~TrackedOrderPairs() = default;

        size_t size() const;

        void init(
            const std::string &algorithm);

        bool insert(
            OrderPair pair);

        bool update(
            OrderPair pair);

        bool cancelPair(
            const std::string &uuid);

        bool cancelPending(
            usd_t currentPrice);
        std::vector<OrderPair> popComplete();

        std::string getFurthestPending(
            usd_t currentPrice) const;

        bool patientOverride(
            utime_t conf,
            utime_t curTime) const;

        const std::unordered_map<std::string, OrderPair> &getPairs() const;

    private:
        bool save;
        Database db;
        std::string algorithm;
        std::unordered_map<std::string, OrderPair> orderPairs;
};

}
