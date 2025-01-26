#pragma once

#include <gtb/IntegerUtils.h>

#include <string>
#include <chrono>
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
    uint32_t betCents = 0;
    uint32_t buyPrice = 0;
    uint32_t sellPrice = 0;
    uint64_t quantity = 0;
    uint64_t created = 0;
    State state = State::None;
    std::chrono::steady_clock::time_point nextTry;

    uint32_t buyValue() const
    {
        return IntegerUtils::getValueCents(buyPrice, quantity);
    }

    uint32_t sellValue() const
    {
        return IntegerUtils::getValueCents(sellPrice, quantity);
    }
};

std::string to_string(OrderPair::State);
void from_string(const std::string &, OrderPair::State &);

}
