#pragma once

#include <gtb/BotContext.h>

#include <gtb/Profits.h>

#include <mutex>

namespace gtb
{

/**
 * Write changes to P/L to the database
 */
class ProfitsWriter
{
    public:
        ProfitsWriter(
            BotContext &ctx);
        ProfitsWriter(ProfitsWriter &&) = delete;
        ProfitsWriter(const ProfitsWriter &) = delete;
        ProfitsWriter &operator=(ProfitsWriter &&) = delete;
        ProfitsWriter &operator=(const ProfitsWriter &) = delete;
        ~ProfitsWriter() = default;

        void process(
            const Profits &profits);

    private:
        BotContext &ctx;

        std::mutex mtx;
        uint64_t prevTime;
};

}
