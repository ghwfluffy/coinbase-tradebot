#include <gtb/PeriodicPrinter.h>
#include <gtb/Log.h>

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
        log::info("Time is %llu", (unsigned long long)time.getTime());
        nextPrint = now + std::chrono::seconds(10);
    }
}
