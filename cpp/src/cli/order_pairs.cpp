#include <gtb/Database.h>
#include <gtb/OrderPairDb.h>
#include <gtb/IntegerUtils.h>

#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

#include <stdio.h>

using namespace gtb;

namespace
{

std::string formatTime(uint64_t microseconds_since_epoch)
{
    // Convert microseconds to seconds
    auto seconds_since_epoch = std::chrono::seconds(microseconds_since_epoch / 1'000'000);

    // Create a time_point from seconds since epoch
    std::chrono::time_point<std::chrono::system_clock> tp(seconds_since_epoch);

    // Convert to time_t for formatting
    std::time_t time = std::chrono::system_clock::to_time_t(tp);

    // Format the time into the required string format
    std::tm tm = *std::localtime(&time);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%b %d %I:%M %p");
    return oss.str();
}

}

int main(int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    Database db;
    db.init("spread_trader.sqlite", "./schema/spread_trader.sql");
    std::list<OrderPair> orders = OrderPairDb::select(db);
    orders.sort(
        [](const OrderPair &lhs, const OrderPair &rhs) -> bool
        {
#if 0
            return lhs.created < rhs.created;
#endif
            return lhs.buyPrice < rhs.buyPrice;
        });

    printf("Algorithm      Status      Created          Bet        Buy         Sell\n");
    for (const OrderPair &pair : orders)
    {
        std::string bet = IntegerUtils::centsToUsd(pair.betCents);
        std::string created = formatTime(pair.created);

        printf("%-14s %-9s   %-14s  $%-8s  $%-9s  $%-9s\n",
            pair.algo.c_str(),
            gtb::to_string(pair.state).c_str(),
            created.c_str(),
            bet.c_str(),
            IntegerUtils::centsToUsd(pair.buyPrice).c_str(),
            IntegerUtils::centsToUsd(pair.sellPrice).c_str());
    }

    return 0;
}
