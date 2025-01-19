#pragma once

#include <gtb/DataController.h>
#include <gtb/Database.h>

namespace gtb
{

struct BotContext
{
    DataController data;
    //ActionPool actionPool;
    Database algoDb;
    Database historicalDb;
};

}
