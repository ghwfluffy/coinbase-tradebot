#pragma once

#include <gtb/ThreadedDataSource.h>
#include <gtb/DatabaseConnection.h>
#include <gtb/BotContext.h>

#include <mutex>
#include <condition_variable>

namespace gtb
{

/**
 * Reads in historical bitcoin prices and replays it as if it were happening in realtime.
 * This churns the rest of the Mocks by feeding the time/price data models and triggering
 * downstream operationse.
 */
class MockMarket : public ThreadedDataSource
{
    public:
        MockMarket(
            BotContext &ctx,
            const std::string &startDate = std::string(),
            const std::string &endDate = std::string());
        MockMarket(MockMarket &&) = delete;
        MockMarket(const MockMarket &) = delete;
        MockMarket &operator=(MockMarket &&) = delete;
        MockMarket &operator=(const MockMarket &) = delete;
        ~MockMarket() final = default;

    protected:
        void process() final;

    private:
        BotContext &ctx;

        uint64_t curTime;
        uint64_t endTime;
        DatabaseConnection conn;

        std::mutex mtxChurn;
        std::condition_variable condChurn;
};

}
