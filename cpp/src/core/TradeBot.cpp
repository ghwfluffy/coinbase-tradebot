#include <gtb/TradeBot.h>
#include <gtb/Log.h>

#include <unistd.h>

using namespace gtb;

TradeBot::TradeBot()
{
    ctx.historicalDb.init("historical.sqlite", "./schema/historical.sql");
}

BotContext &TradeBot::getCtx()
{
    return ctx;
}

void TradeBot::addSource(std::unique_ptr<DataSource> source)
{
    if (source)
        sources.push_back(std::move(source));
}

int TradeBot::run()
{
    log::info("Starting Ghw Trade Bot.");

    ctx.actionPool.start();

    for (auto &source : sources)
        source->start();

    // TODO: Hook into SIGINT/TERM
    while (true)
        sleep(10);

    log::info("Stopping Ghw Trade Bot.");

    for (auto &source : sources)
        source->start();

    log::info("Ghw Trade Bot Shutdown.");

    return 0;
}
