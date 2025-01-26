#include <gtb/ProfitsReader.h>
#include <gtb/IntegerUtils.h>
#include <gtb/Profits.h>
#include <gtb/Log.h>

using namespace gtb;

namespace
{

constexpr const char *READ_QUERY =
    "SELECT purchased, sold, buy_fees, sell_fees FROM profits_and_losses ORDER BY time DESC LIMIT 1";

}

void ProfitsReader::initProfits(
    BotContext &ctx)
{
    DatabaseResult result = ctx.historicalDb.getConn().query(READ_QUERY);
    if (result.next())
    {
        ctx.data.get<Profits>().addOrderPair(
            result[0].getUInt64(),
            result[1].getUInt64(),
            result[2].getUInt64(),
            result[3].getUInt64());
        log::info("Initialized profits to %s.",
            IntegerUtils::centsToUsd(ctx.data.get<Profits>().getProfit()).c_str());
    }
}
