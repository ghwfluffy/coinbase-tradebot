#include <gtb/BtcHistoricalWriter.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

#include <sstream>

using namespace gtb;

BtcHistoricalWriter::BtcHistoricalWriter(
    BotContext &ctx)
        : ctx(ctx)
{
    ctx.data.subscribe<BtcPrice>(*this);
}

void BtcHistoricalWriter::process(
    const BtcPrice &price)
{
    usd_t curPrice = price.getPrice();
    utime_t curTime = ctx.data.get<Time>().getTime();

    std::lock_guard<std::mutex> lock(mtx);
    if (curTime > prevTime)
    {
        prevTime = curTime;
        std::stringstream query;
        query << "INSERT INTO btc_price (time, price) VALUES (" << curTime.value() << "," << curPrice.value() << ")";
        if (!ctx.historicalDb.getConn().execute(query.str()))
            log::error("Failed to record BTC historical data.");
    }
}
