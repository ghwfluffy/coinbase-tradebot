#pragma once

#include <gtb/ThreadedDataSource.h>
#include <gtb/WebsocketClient.h>
#include <gtb/BotContext.h>

#include <nlohmann/json.hpp>

namespace gtb
{

/**
 * Subscribe to Coinbase advanced API and maintain the list of open trades
 */
class CoinbaseUserTrades : public ThreadedDataSource
{
    public:
        CoinbaseUserTrades(
            BotContext &ctx);
        CoinbaseUserTrades(CoinbaseUserTrades &&) = delete;
        CoinbaseUserTrades(const CoinbaseUserTrades &) = delete;
        CoinbaseUserTrades &operator=(CoinbaseUserTrades &&) = delete;
        CoinbaseUserTrades &operator=(const CoinbaseUserTrades &) = delete;
        ~CoinbaseUserTrades() final = default;

    protected:
        void process() final;
        void shutdown() final;

    private:
        void handleMessage(
            nlohmann::json json);

        void reset();

        void update(
            const nlohmann::json &orders,
            bool reset = false);

        BotContext &ctx;

        WebsocketClient client;
};

}
