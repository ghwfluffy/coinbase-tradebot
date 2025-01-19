#include <gtb/BtcHistoricalWriter.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

#include <sstream>

using namespace gtb;

BtcHistoricalWriter::BtcHistoricalWriter(
    BotContext &ctx)
        : ctx(ctx)
{
    prevTime = 0;
}

void BtcHistoricalWriter::process(
    const BtcPrice &price)
{
    uint64_t cents = price.getCents();
    uint64_t curTime = ctx.data.get<Time>().getTime();

    std::lock_guard<std::mutex> lock(mtx);
    if (curTime > prevTime)
    {
        prevTime = curTime;
        std::stringstream query;
        query << "INSERT INTO btc_price (time, price) VALUES (" << curTime << "," << cents << ")";
        if (!ctx.historicalDb.newConn().execute(query.str()))
            log::error("Failed to record BTC historical data.");
    }
}
