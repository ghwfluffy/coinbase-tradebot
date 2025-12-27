#pragma once

#include <gtb/BotContext.h>

#include <gtb/BtcPrice.h>
#include <gtb/IntLiterals.h>

namespace gtb
{

/**
 * Trades then immediately sells just to increase trading volume
 */
class VolumeTrader
{
    public:
        struct Config
        {
            std::string name;
            usd_t betSize = 10_Dollars;
        };

        VolumeTrader(
            BotContext &ctx,
            Config config);
        VolumeTrader(VolumeTrader &&) = delete;
        VolumeTrader(const VolumeTrader &) = delete;
        VolumeTrader &operator=(VolumeTrader &&) = delete;
        VolumeTrader &operator=(const VolumeTrader &) = delete;
        ~VolumeTrader() = default;

        void process(
            const BtcPrice &price);

    private:
        BotContext &ctx;
        Config conf;

        CoinbaseOrder order;
};

}
