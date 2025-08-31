#pragma once

#include <gtb/IntegerUtils.h>
#include <gtb/SteadyClock.h>
#include <gtb/IntLiterals.h>

#include <string>

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

    std::string uuid;
    // Buy or sell
    bool buy = false;
    // Price of BTC to buy/sell at
    usd_t price;
    // How many BTC to buy/sell
    btc_t quantity;
    // Final wallet change (not including fees)
    usd_t beforeFees;
    // Fees paid
    usd_t fees;

    State state = State::None;
    utime_t createdTime;
    SteadyClock::TimePoint cleanupTime;

    operator bool() const
    {
        return !uuid.empty() && state != State::None && price && quantity;
    }

    usd_t value() const
    {
        return IntegerUtils::getValue(price, quantity);
    }

    // Set quantity based on USD amount to buy at btcPrice
    void setQuantity(
        usd_t btcPrice,
        usd_t transactionSize)
    {
        if (!btcPrice || !transactionSize)
        {
            this->quantity = {};
            this->price = {};
            return;
        }

        this->price = btcPrice;
        this->quantity = IntegerUtils::getSatoshiForPrice(btcPrice, transactionSize);
    }
};

}
