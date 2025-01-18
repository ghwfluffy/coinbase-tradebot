#pragma once

#include <gtb/DataProcessor.h>
#include <gtb/Time.h>
#include <gtb/BotContext.h>

#include <chrono>
#include <mutex>

namespace gtb
{

class PeriodicPrinter : public DataProcessor
{
    public:
        PeriodicPrinter(
            BotContext &ctx);
        PeriodicPrinter(PeriodicPrinter &&) = delete;
        PeriodicPrinter(const PeriodicPrinter &) = delete;
        PeriodicPrinter &operator=(PeriodicPrinter &&) = delete;
        PeriodicPrinter &operator=(const PeriodicPrinter &) = delete;
        ~PeriodicPrinter() final = default;

        void process(
            const Time &time);

    private:
        BotContext &ctx;

        std::mutex mtx;
        std::chrono::steady_clock::time_point nextPrint;
};

}
