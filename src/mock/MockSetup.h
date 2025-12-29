#pragma once

#include <gtb/TradeBot.h>
#include <gtb/IntLiterals.h>

namespace gtb
{

/**
 * Setup the TradeBot in mock mode
 */
namespace MockSetup
{
    struct Config
    {
        // Start date to start emulation
        std::string startDate;
        // End date to emulate until
        std::string endDate;
        // Start mock with a high volume (low fee tier)
        bool initHighVolume = false;
        // File with historical data (btc_price table)
        std::string dataFile = "data/historical.sqlite";
        // How much to start in your wallet
        usd_t startWallet = 10'000_Dollars;
    };

    void init(
        TradeBot &bot,
        Config conf);
}

}
