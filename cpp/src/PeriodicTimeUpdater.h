#pragma once

#include <gtb/ThreadedDataSource.h>
#include <gtb/BotContext.h>
#include <gtb/Time.h>

namespace gtb
{

/**
 * Periodically updates the Time data model which will
 * trigger downstream time-based events.
 */
class PeriodicTimeUpdater : public ThreadedDataSource
{
    public:
        PeriodicTimeUpdater(
            BotContext &ctx);
        PeriodicTimeUpdater(PeriodicTimeUpdater &&) = delete;
        PeriodicTimeUpdater(const PeriodicTimeUpdater &) = delete;
        PeriodicTimeUpdater &operator=(PeriodicTimeUpdater &&) = delete;
        PeriodicTimeUpdater &operator=(const PeriodicTimeUpdater &) = delete;
        ~PeriodicTimeUpdater() final = default;

    protected:
        void process() final;

    private:
        Time &time;
};

}
