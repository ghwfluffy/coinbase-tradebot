#pragma once

#include <gtb/DataController.h>
#include <gtb/Database.h>
#include <gtb/ActionThreadPool.h>
#include <gtb/CoinbaseInterface.h>

namespace gtb
{

struct BotContext
{
    ActionThreadPool actionPool;
    DataController data;
    Database historicalDb;
    CoinbaseInterface &coinbase();

    BotContext();
    void setCoinbase(std::unique_ptr<CoinbaseInterface> coinbase);

    private:
    std::unique_ptr<CoinbaseInterface> coinbaseIfx;
};

}
