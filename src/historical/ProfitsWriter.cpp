#include <gtb/ProfitsWriter.h>
#include <gtb/Profits.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

#include <sstream>

using namespace gtb;

ProfitsWriter::ProfitsWriter(
    BotContext &ctx)
        : ctx(ctx)
{
    ctx.data.subscribe<Profits>(*this);
}

void ProfitsWriter::process(
    const Profits &profits)
{
    (void)profits;
#if 0
    Profits::TraderData data = profits.getTotals();
    utime_t curTime = ctx.data.get<Time>().getTime();

    std::lock_guard<std::mutex> lock(mtx);
    if (curTime > prevTime)
    {
        prevTime = curTime;
        std::stringstream query;
        query << "INSERT INTO profits_and_losses "
            << "(time, purchased, sold, buy_fees, sell_fees, profit) "
            << "VALUES ("
            << curTime.value() << ","
            << data.purchased.value().toUint64() << ","
            << data.sold.value().toUint64() << ","
            << data.buyFees.value().toUint64() << ","
            << data.sellFees.value().toUint64() << ","
            << data.getProfit().value().toInt64()
            << ")";
        if (!ctx.historicalDb.getConn().execute(query.str()))
            log::error("Failed to record historical profits.");
    }
#endif
}
