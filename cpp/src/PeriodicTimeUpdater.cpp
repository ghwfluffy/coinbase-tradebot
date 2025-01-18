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
    auto now = std::chrono::steady_clock::now();
    auto epoch = std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch();
    uint64_t microseconds = static_cast<uint64_t>(epoch.count());
    time.setTime(microseconds);

    sleep(std::chrono::seconds(1));
}
