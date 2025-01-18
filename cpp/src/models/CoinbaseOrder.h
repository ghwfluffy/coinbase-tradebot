#pragma once

#include <string>
#include <chrono>

#include <stdint.h>

namespace gtb
{

struct CoinbaseOrder
{
    enum State
    {
        None = 0,
        Open,
        Filled,
        Canceled,
        Error,
    };

    State state = State::None;
    std::string uuid;
    bool buy = false;
    // Price of BTC to buy/sell at
    uint32_t priceCents = 0;
    // 100-millions of a bitcoin (Satoshi)
    uint64_t quantity = 0;
    std::string createdTime;
    std::chrono::steady_clock::time_point cleanupTime;

    operator bool() const
    {
        return !uuid.empty() && state != State::None && priceCents > 0 && quantity > 0;
    }

    uint32_t valueCents() const
    {
        return static_cast<uint32_t>((quantity * priceCents) / 100'000'000ULL);
    }
};

}
