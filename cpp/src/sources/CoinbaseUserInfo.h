#pragma once

#include <gtb/ThreadedDataSource.h>
#include <gtb/CoinbaseRestClient.h>
#include <gtb/CoinbaseOrderBook.h>
#include <gtb/BotContext.h>

namespace gtb
{

/**
 * Query what's in the coinbase user's wallet
 */
class CoinbaseUserInfo : public ThreadedDataSource
{
    public:
        CoinbaseUserInfo(
            BotContext &ctx);
        CoinbaseUserInfo(CoinbaseUserInfo &&) = delete;
        CoinbaseUserInfo(const CoinbaseUserInfo &) = delete;
        CoinbaseUserInfo &operator=(CoinbaseUserInfo &&) = delete;
        CoinbaseUserInfo &operator=(const CoinbaseUserInfo &) = delete;
        ~CoinbaseUserInfo() final = default;

        void process() final;
        void process(
            const CoinbaseOrderBook &book);

    private:
        void query();

        BotContext &ctx;

        CoinbaseRestClient client;
};

}
