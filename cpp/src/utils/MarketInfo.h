#pragma once

#include <string>

#include <stdint.h>

namespace gtb
{

/**
 * Get information about the current market times
 *
 * Stock Market (NYSE/NASDAQ):
 * Monday-Friday
 * 9:30AM - 4:00PM (Eastern)
 * 8:30AM - 3:00PM (Central)
 * 2:30PM - 9:00PM (UTC)
 * 1:30PM - 8:00PM (UTC - DST)
 *
 * Bitcoin Futures:
 * Closed Friday 5:00PM - Sunday 6:00PM (Eastern)
 * Closed Daily 5:00PM - 6:00PM (Eastern)
 */
class MarketInfo
{
    public:
        enum class Market
        {
            None = 0,
            StockMarket,
            BitcoinFutures,
        };

        MarketInfo(
            Market market = Market::None,
            uint64_t time = 0);
        MarketInfo(MarketInfo &&) = default;
        MarketInfo(const MarketInfo &) = default;
        MarketInfo &operator=(MarketInfo &&) = default;
        MarketInfo &operator=(const MarketInfo &) = default;
        ~MarketInfo() = default;

        bool isOpen() const;
        bool isClosed() const;
        bool isWeekend() const;
        bool isWeekendNext() const;

        uint64_t tillOpen() const;
        uint64_t tillClosed() const;

        uint64_t sinceOpen() const;
        uint64_t sinceClosed() const;

    private:
        Market market;
        uint64_t time;
};

std::string to_string(MarketInfo::Market);

}
