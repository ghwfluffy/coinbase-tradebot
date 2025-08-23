#pragma once

#include <gtb/DataModel.h>
#include <gtb/CoinbaseOrder.h>

#include <map>
#include <list>
#include <mutex>
#include <stdint.h>

namespace gtb
{

class CoinbaseOrderBook : public DataModel
{
    public:
        CoinbaseOrderBook() = default;
        CoinbaseOrderBook(CoinbaseOrderBook &&);
        CoinbaseOrderBook(const CoinbaseOrderBook &) = delete;
        CoinbaseOrderBook &operator=(CoinbaseOrderBook &&) = delete;
        CoinbaseOrderBook &operator=(const CoinbaseOrderBook &) = delete;
        ~CoinbaseOrderBook() final = default;

        CoinbaseOrder getOrder(
            const std::string &uuid) const;

        std::map<std::string, CoinbaseOrder> getOrders() const;

        void update();

        void update(
            CoinbaseOrder order);

        void update(
            std::list<CoinbaseOrder> orders,
            bool reset = false);

    private:
        void cleanup(
            const std::lock_guard<std::mutex> &lock);

        std::mutex mtx;
        std::map<std::string, CoinbaseOrder> orders;
};

}
