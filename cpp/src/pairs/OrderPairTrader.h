#pragma once

#include <gtb/Database.h>
#include <gtb/BotContext.h>
#include <gtb/BaseTraderConfig.h>

#include <gtb/BtcPrice.h>
#include <gtb/CoinbaseOrderBook.h>

#include <gtb/OrderPair.h>
#include <gtb/OrderPairStateMachine.h>

#include <list>
#include <mutex>

namespace gtb
{

/**
 * Abstract base class for a trader that creates OrderPairs and moves
 * them through the order pair state machine
 */
class OrderPairTrader
{
    public:
        OrderPairTrader(
            BotContext &ctx,
            const BaseTraderConfig &conf);
        OrderPairTrader(OrderPairTrader &&) = delete;
        OrderPairTrader(const OrderPairTrader &) = delete;
        OrderPairTrader &operator=(OrderPairTrader &&) = delete;
        OrderPairTrader &operator=(const OrderPairTrader &) = delete;
        ~OrderPairTrader() = default;

        void process(
            const BtcPrice &price);

        void process(
            const CoinbaseOrderBook &orderbook);

    protected:
        virtual void handleNewPair(
            const BtcPrice &price) = 0;

        BotContext &ctx;
        BaseTraderConfig conf;

        Database db;
        std::list<OrderPair> orderPairs;
        OrderPairStateMachine stateMachine;

    private:
        void loadDatabase();

        void handleExistingPairs(
            bool force = false);

        std::mutex mtx;
};

}
