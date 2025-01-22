#pragma once

#include <gtb/IntegerUtils.h>

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
    uint64_t createdTime = 0;
    std::chrono::steady_clock::time_point cleanupTime;

    operator bool() const
    {
        return !uuid.empty() && state != State::None && priceCents > 0 && quantity > 0;
    }

    uint32_t valueCents() const
    {
        return IntegerUtils::getValueCents(priceCents, quantity);
    }

    void setQuantity(
        uint32_t priceCents,
        uint32_t amountCents)
    {
        if (priceCents == 0 || amountCents == 0)
        {
            this->quantity = 0;
            this->priceCents = 0;
            return;
        }

        this->priceCents = priceCents;
        this->quantity = IntegerUtils::getSatoshiForPrice(priceCents, amountCents);
    }
};

}
