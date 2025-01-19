#pragma once

#include <gtb/DataSource.h>
#include <gtb/DataProcessor.h>
#include <gtb/BotContext.h>

#include <list>
#include <memory>

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
        ~TradeBot() = default;

        BotContext &getCtx();

        void addSource(std::unique_ptr<DataSource> source);
        void addProcessor(std::unique_ptr<DataProcessor> processor);

        int run();

    private:
        BotContext ctx;
        std::list<std::unique_ptr<DataSource>> sources;
        std::list<std::unique_ptr<DataProcessor>> processors;
};

}
