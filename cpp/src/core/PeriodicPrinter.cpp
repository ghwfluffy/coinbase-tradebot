#include <gtb/PeriodicPrinter.h>
#include <gtb/Log.h>

#include <gtb/BtcPrice.h>

using namespace gtb;

PeriodicPrinter::PeriodicPrinter(
    BotContext &ctx)
        : ctx(ctx)
{
}

void PeriodicPrinter::process(
    const Time &time)
{
    auto now = std::chrono::steady_clock::now();

    std::lock_guard<std::mutex> lock(mtx);
    if (now >= nextPrint)
    {
        uint32_t cents = ctx.data.get<BtcPrice>().getCents();
        if (cents)
        {
            log::info("BTC price: $%u.%02u.", cents / 100, cents % 100);
            nextPrint = now + std::chrono::seconds(10);
        }
    }
}
