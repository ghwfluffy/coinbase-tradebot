#pragma once

#include <gtb/Profits.h>
#include <gtb/SteadyClock.h>
#include <gtb/IntegerUtils.h>

#include <vector>
#include <string>
#include <stdint.h>

namespace gtb
{

struct OrderPair
{
    enum class State
    {
        None = 0,
        Pending,
        BuyActive,
        Holding,
        SellActive,
        Complete,
        Canceled,
        Error,
    };

    std::string uuid;
    std::string algo;
    std::string buyOrder;
    std::string sellOrder;
    usd_t bet;
    usd_t buyPrice;
    usd_t sellPrice;
    usd_t origSellPrice;
    btc_t quantity;
    utime_t created;
    State state = State::None;
    SteadyClock::TimePoint nextTry;

    Profits::Data profit;

    // TODO: Add to database
    std::vector<std::string> modifiers;

    std::string getModifiers() const
    {
        std::string ret;
        for (const std::string &m : modifiers)
        {
            if (!ret.empty())
                ret += ",";
            ret += m;
        }

        return ret;
    }

    operator bool() const
    {
        return bet && buyPrice && sellPrice && state > State::None;
    };

    usd_t sellValue() const
    {
        return IntegerUtils::getValue(sellPrice, quantity);
    }
};

std::string to_string(OrderPair::State);
void from_string(const std::string &, OrderPair::State &);

}
