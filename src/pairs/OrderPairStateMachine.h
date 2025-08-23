#pragma once

#include <gtb/Database.h>
#include <gtb/BotContext.h>

#include <gtb/BtcPrice.h>
#include <gtb/OrderPair.h>
#include <gtb/SteadyClock.h>
#include <gtb/BaseTraderConfig.h>

#include <list>

namespace gtb
{

/**
 * Moves an order pair through it's states of waiting, buying, holding, then selling
 */
class OrderPairStateMachine
{
    public:
        OrderPairStateMachine(
            BotContext &ctx,
            Database &db,
            BaseTraderConfig conf);
        OrderPairStateMachine(OrderPairStateMachine &&) = default;
        OrderPairStateMachine(const OrderPairStateMachine &) = default;
        OrderPairStateMachine &operator=(OrderPairStateMachine &&) = delete;
        OrderPairStateMachine &operator=(const OrderPairStateMachine &) = delete;
        ~OrderPairStateMachine() = default;

        void churn(
            std::list<OrderPair> &orderPairs,
            bool force = false);

        void churn(
            OrderPair &pair,
            bool force = false);

        void logChange(
            OrderPair::State startState,
            const OrderPair &pair);

    private:
        void checkBuyState(
            OrderPair &pair,
            bool force = false);

        void checkSellState(
            OrderPair &pair,
            bool force = false);

        void handlePending(
            OrderPair &pair,
            const BtcPrice &price);

        void handleBuyActive(
            OrderPair &pair,
            const BtcPrice &price);

        void handleHolding(
            OrderPair &pair,
            const BtcPrice &price);

        void handleSellActive(
            OrderPair &pair,
            const BtcPrice &price);

        bool cancelPending(
            const BtcPrice &price);

        BotContext &ctx;
        Database &db;
        BaseTraderConfig conf;
        SteadyClock::TimePoint nextTrade;
};

}
