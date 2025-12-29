#include <gtb/AlgorithmFactory.h>
#include <gtb/Log.h>

#include <gtb/Version1.h>
#include <gtb/Version2.h>

using namespace gtb;

bool AlgorithmFactory::provision(
    gtb::TradeBot &bot,
    unsigned int version,
    bool mock)
{
    log::info("Initializing Ghw Trade Bot version 2%s.", mock ? " - Mock Test" : "");

    switch (version)
    {
        case 1:
            log::error("Version 1 is deprecated.");
            return false;
            Version1::init(bot, mock);
            return true;
        case 2:
            Version2::init(bot, mock);
            return true;
        default:
            log::error("Invalid algorithm version %u.", version);
            break;
    }

    return false;
}
