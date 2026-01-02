#pragma once

#include <gtb/BotContext.h>
#include <gtb/BtcPrice.h>
#include <gtb/CoinbaseOrder.h>
#include <gtb/IntLiterals.h>
#include <gtb/OrderPair.h>
#include <gtb/OrderPairStateMachine.h>
#include <gtb/BaseTraderConfig.h>

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
            usd_t minProfitDelta = 2_Dollars;
            usd_t repriceBand = 2_Dollars;
            utime_t orderTtl = 1_Minutes;
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
        void resetState();
        void ensurePair(
            usd_t price);
        void manageSellReprice(
            const BtcPrice &price);
        void manageBuyCancel(
            const BtcPrice &price);
        void manageHoldingDecay(
            const BtcPrice &price);
        bool canCancel(
            const std::string &uuid) const;

        BotContext &ctx;
        Config conf;
        OrderPair pair;
        OrderPairStateMachine stateMachine;
        SteadyClock::TimePoint orderCreated;
};

}
