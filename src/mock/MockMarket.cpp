#include <gtb/MockMarket.h>
#include <gtb/MockLock.h>
#include <gtb/BtcPrice.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

#include <chrono>

#include <time.h>
#include <signal.h>

using namespace gtb;

namespace
{

// Convert YYYY-mm-dd into time-since-epoch in microseconds
uint64_t dateToMicro(
    const std::string &date)
{
    struct tm tm = {};
    if (date.empty() || !strptime(date.c_str(), "%Y-%m-%d", &tm))
        log::error("Invalid date format '%s' (expected YYYY-mm-dd)", date.c_str());

    // Use timegm to interpret tm as UTC.
    time_t seconds = timegm(&tm);
    return static_cast<uint64_t>(seconds) * 1000000ULL;
}

}

MockMarket::MockMarket(
    BotContext &ctx,
    const std::string &start,
    const std::string &end)
        : ThreadedDataSource("historical-market")
        , ctx(ctx)
{
    if (start.empty())
        curTime = 0;
    else
        curTime = dateToMicro(start);

    if (end.empty())
        endTime = 4102444800000000ULL; // 2100 AD
    else
        endTime = dateToMicro(end);
}

void MockMarket::process()
{
    // Initialize database connection
    if (!conn)
        conn = ctx.historicalDb.getConn();

    // Query next market data value
    char szQuery[512] = {};
    snprintf(szQuery, sizeof(szQuery),
        "SELECT time, price FROM btc_price "
        "WHERE time>=%llu AND time<=%llu "
        "ORDER BY time ASC LIMIT 1",
        static_cast<unsigned long long>(curTime),
        static_cast<unsigned long long>(endTime));

    DatabaseResult res = conn.query(szQuery);
    if (!res)
    {
        log::error("Failed to query historical market data.");
        sleep(std::chrono::seconds(10));
    }
    else if (!res.next())
    {
        log::info("Simulation complete.");
        sleep(std::chrono::seconds(2));
        raise(SIGTERM);
    }
    else
    {
        // Update market value atomically
        {
            auto lock = ctx.data.get<MockLock>().lock();

            uint64_t time = res[0].getUInt64();
            uint32_t price = res[1].getUInt32();
            ctx.data.get<Time>().setTime(time);
            ctx.data.get<BtcPrice>().setCents(price);

            curTime = time + 1;
        }

        // Wait for the action pool thread to finish executing all downstream actions
        auto wakeup = [this]() mutable -> void
        {
            std::lock_guard<std::mutex> lock(mtxChurn);
            condChurn.notify_one();
        };

        auto queueWakeup = [this, wakeup = std::move(wakeup)]() mutable -> void
        {
            ctx.actionPool.waitComplete(std::move(wakeup));
        };

        std::unique_lock<std::mutex> lock(mtxChurn);
        ctx.actionPool.queue(std::move(queueWakeup));
        condChurn.wait_for(lock, std::chrono::seconds(1));
    }
}
