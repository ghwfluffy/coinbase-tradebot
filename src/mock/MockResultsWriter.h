#pragma once

#include <gtb/Time.h>
#include <gtb/BotContext.h>
#include <gtb/SteadyClock.h>

#include <mutex>

namespace gtb
{

/**
 * Save the results of the mock test to a JSONL file.
 * This JSONL file will contain updates about the status of each
 * trader every 10 mocked minutes.
 */
class MockResultsWriter
{
    public:
        MockResultsWriter(
            BotContext &ctx,
            std::string filename);
        MockResultsWriter(MockResultsWriter &&) = delete;
        MockResultsWriter(const MockResultsWriter &) = delete;
        MockResultsWriter &operator=(MockResultsWriter &&) = delete;
        MockResultsWriter &operator=(const MockResultsWriter &) = delete;
        ~MockResultsWriter() = default;

        void process(
            const Time &time);

    private:
        BotContext &ctx;

        std::mutex mtx;
        std::string filename;
        SteadyClock::TimePoint prevPrint;
        SteadyClock::TimePoint nextPrint;
};

}
