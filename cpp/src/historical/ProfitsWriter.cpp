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
    prevTime = 0;
    ctx.data.subscribe<Profits>(*this);
}

void ProfitsWriter::process(
    const Profits &profits)
{
    Profits::Data data = profits.getData();
    uint64_t curTime = ctx.data.get<Time>().getTime();

    std::lock_guard<std::mutex> lock(mtx);
    if (curTime > prevTime)
    {
        prevTime = curTime;
        std::stringstream query;
        query << "INSERT INTO profits_and_losses "
            << "(time, purchased, sold, buy_fees, sell_fees, profit) "
            << "VALUES ("
            << data.purchased << ","
            << data.sold << ","
            << data.buyFees << ","
            << data.sellFees << ","
            << data.getProfit()
            << ")";
        if (!ctx.historicalDb.getConn().execute(query.str()))
            log::error("Failed to record historical profits.");
    }
}
