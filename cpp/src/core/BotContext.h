#pragma once

#include <gtb/DataController.h>
#include <gtb/Database.h>
#include <gtb/ActionThreadPool.h>

namespace gtb
{

struct BotContext
{
    ActionThreadPool actionPool;
    DataController data;
    Database algoDb;
    Database historicalDb;

    BotContext();
};

}
