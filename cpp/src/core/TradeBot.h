#pragma once

#include <gtb/DataSource.h>
#include <gtb/BotContext.h>
#include <gtb/Any.h>

#include <list>
#include <mutex>
#include <atomic>
#include <memory>
#include <condition_variable>

namespace gtb
{

class TradeBot
{
    public:
        TradeBot();
        TradeBot(TradeBot &&) = delete;
        TradeBot(const TradeBot &) = delete;
        TradeBot &operator=(TradeBot &&) = delete;
        TradeBot &operator=(const TradeBot &) = delete;
        ~TradeBot();

        BotContext &getCtx();

        void addSource(std::unique_ptr<DataSource> source);

        template<typename T>
        void addProcessor(std::unique_ptr<T> processor)
        {
            processors.emplace_back(std::move(processor));
        }

        int run();
        void stop();

    private:
        void initSignals();
        void cleanupSignals();

        BotContext ctx;
        std::list<std::unique_ptr<DataSource>> sources;
        std::list<Any> processors;

        std::mutex mtx;
        std::atomic<bool> running;
        std::condition_variable cond;

        unsigned int interrupts;
};

}
