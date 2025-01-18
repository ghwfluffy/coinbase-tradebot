#include <gtb/PeriodicTimeUpdater.h>

using namespace gtb;

PeriodicTimeUpdater::PeriodicTimeUpdater(
    BotContext &ctx)
        : ThreadedDataSource("time-updater")
        , time(ctx.data.get<Time>())
{
}

void PeriodicTimeUpdater::process()
{
    time.setNow();
    sleep(std::chrono::seconds(1));
}
