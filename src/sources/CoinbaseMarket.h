#pragma once

#include <gtb/ThreadedDataSource.h>
#include <gtb/WebsocketClient.h>
#include <gtb/BotContext.h>
#include <gtb/BtcPrice.h>
#include <gtb/Time.h>

namespace gtb
{

/**
 * Subscribe to Coinbase advanced API websocket and receive updates about
 * Bitcoin trades being executed.
 */
class CoinbaseMarket : public ThreadedDataSource
{
    public:
        CoinbaseMarket(
            BotContext &ctx);
        CoinbaseMarket(CoinbaseMarket &&) = delete;
        CoinbaseMarket(const CoinbaseMarket &) = delete;
        CoinbaseMarket &operator=(CoinbaseMarket &&) = delete;
        CoinbaseMarket &operator=(const CoinbaseMarket &) = delete;
        ~CoinbaseMarket() final = default;

    protected:
        void process() final;
        void shutdown() final;

    private:
        void handleMessage(
            nlohmann::json json);

        BotContext &ctx;
        BtcPrice &btc;
        Time &time;

        WebsocketClient client;
};

}
