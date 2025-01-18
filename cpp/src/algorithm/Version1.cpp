#include <gtb/Version1.h>
#include <gtb/PeriodicTimeUpdater.h>
#include <gtb/PeriodicPrinter.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

using namespace gtb;

void Version1::init(TradeBot &bot)
{
    log::info("Initializing Ghw Trade Bot version 1.");

    BotContext &ctx = bot.getCtx();

    // Initial state
    ctx.data.get<Time>().setTime(1337);

    // Source: Time updater
    {
        auto source = std::make_unique<PeriodicTimeUpdater>(ctx);
        bot.addSource(std::move(source));
    }

    // Processor: Printer
    {
        auto printer = std::make_unique<PeriodicPrinter>(ctx);
        ctx.data.subscribe<Time>(*printer);
        bot.addProcessor(std::move(printer));
    }
}
