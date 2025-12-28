#pragma once

#include <gtb/BotContext.h>
#include <gtb/BtcPrice.h>
#include <gtb/CoinbaseOrder.h>
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
        enum class Phase
        {
            Idle,
            BuyPending,
            SellPending
        };

        bool startBuy(
            usd_t price);
        bool startSell(
            usd_t price);
        void handleFilled(
            const CoinbaseOrder &updated,
            usd_t price);
        void handleOpen(
            const CoinbaseOrder &updated,
            usd_t price);
        void resetState();

        BotContext &ctx;
        Config conf;

        Phase phase;
        btc_t pendingQty;
        CoinbaseOrder order;
};

}
