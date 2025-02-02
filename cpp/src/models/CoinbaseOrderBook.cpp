#include <gtb/CoinbaseOrderBook.h>
#include <gtb/Log.h>

using namespace gtb;

CoinbaseOrderBook::CoinbaseOrderBook(CoinbaseOrderBook &&rhs)
{
    this->orders = std::move(rhs.orders);
}

CoinbaseOrder CoinbaseOrderBook::getOrder(
    const std::string &uuid) const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mtx));
    auto iter = orders.find(uuid);
    if (iter != orders.end())
        return iter->second;
    return CoinbaseOrder();
}

std::map<std::string, CoinbaseOrder> CoinbaseOrderBook::getOrders() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mtx));
    return orders;
}

void CoinbaseOrderBook::update(
    CoinbaseOrder order)
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::string uuid = order.uuid;
        orders[uuid] = std::move(order);

        cleanup(lock);
    }

    updated();
}

void CoinbaseOrderBook::update(
    std::list<CoinbaseOrder> updates,
    bool reset)
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (reset)
            orders.clear();
        for (CoinbaseOrder &order : updates)
        {
            std::string uuid = order.uuid;
            orders[uuid] = std::move(order);
        }

        if (!reset)
            cleanup(lock);
    }

    updated();
}

void CoinbaseOrderBook::cleanup(
    const std::lock_guard<std::mutex> &lock)
{
    (void)lock;

    auto now = SteadyClock::now();

    auto iter = orders.begin();
    while (iter != orders.end())
    {
        CoinbaseOrder &order = iter->second;
        if (!order || (order.state != CoinbaseOrder::State::Open && order.cleanupTime <= now))
            iter = orders.erase(iter);
        else
            ++iter;
    }
}
