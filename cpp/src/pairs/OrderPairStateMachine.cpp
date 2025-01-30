#include <gtb/OrderPairStateMachine.h>
#include <gtb/CoinbaseOrderBook.h>
#include <gtb/OrderPairDb.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

using namespace gtb;

namespace
{

// If an order fails to post, wait this long before retrying the spread
constexpr const unsigned int FAILURE_RETRY_SECONDS = 10;

// Keep orders active if they're within $100 of our target price
constexpr const uint32_t KEEP_ACTIVE_ORDER_WINDOW_CENTS = 100'00;

}

OrderPairStateMachine::OrderPairStateMachine(
    BotContext &ctx,
    Database &db,
    std::string name)
        : ctx(ctx)
        , db(db)
        , name(std::move(name))
{
}

void OrderPairStateMachine::churn(
    std::list<OrderPair> &orderPairs,
    bool force)
{
    for (OrderPair &pair : orderPairs)
        churn(pair, force);
}

void OrderPairStateMachine::churn(
    OrderPair &pair,
    bool force)
{
    // This pair is not ready to act yet
    if (pair.nextTry > std::chrono::steady_clock::now() && !force)
        return;

    OrderPair::State startState = pair.state;

    const BtcPrice &price = ctx.data.get<BtcPrice>();

    // Don't trust the cache and query an update from Coinbase
    if (force)
    {
        checkBuyState(pair, true);
        checkSellState(pair, true);
    }

    // Handle each state
    switch (pair.state)
    {
        case OrderPair::State::Pending:
            handlePending(pair, price);
            break;
        case OrderPair::State::BuyActive:
            handleBuyActive(pair, price);
            break;
        case OrderPair::State::Holding:
            handleHolding(pair, price);
            break;
        case OrderPair::State::SellActive:
            handleSellActive(pair, price);
            break;
        default:
        case OrderPair::State::None:
        case OrderPair::State::Complete:
        case OrderPair::State::Canceled:
        case OrderPair::State::Error:
            break;
    }

    // Update database and log change
    if (pair.state != startState)
    {
        logChange(startState, pair);
        OrderPairDb::update(db, pair);
    }
}

void OrderPairStateMachine::checkBuyState(
    OrderPair &pair,
    bool force)
{
    if (pair.state != OrderPair::State::BuyActive)
        return;

    CoinbaseOrderBook &orderbook = ctx.data.get<CoinbaseOrderBook>();

    CoinbaseOrder buyOrder = orderbook.getOrder(pair.buyOrder);
    if (!buyOrder || force)
    {
        buyOrder = ctx.coinbase().getOrder(pair.buyOrder);
        if (buyOrder)
            orderbook.update(buyOrder);
    }

    if (!buyOrder)
    {
        pair.state = OrderPair::State::Error;
        log::error("Failed to find buy order '%s' for order pair '%s'.",
            pair.buyOrder.c_str(),
            pair.uuid.c_str());
    }
    else if (buyOrder.state == CoinbaseOrder::State::Open)
    {
        // Still buy active
    }
    else if (buyOrder.state == CoinbaseOrder::State::Filled)
    {
        // Complete, now holding
        pair.state = OrderPair::State::Holding;
        // Record the price it filled at
        pair.buyPrice = buyOrder.priceCents;
        pair.quantity = buyOrder.quantity;
        pair.profit.purchased = buyOrder.beforeFees;
        pair.profit.buyFees = buyOrder.fees;
    }
    else if (buyOrder.state == CoinbaseOrder::State::Canceled)
    {
        // Buy canceled
        pair.state = OrderPair::State::Canceled;
    }
    else //if (buyOrder.state == CoinbaseOrder::state::None || buyOrder.state == CoinbaseOrder::state::Error)
    {
        pair.state = OrderPair::State::Error;
        log::error("Buy order '%s' for order pair '%s' in error state.",
            pair.buyOrder.c_str(),
            pair.uuid.c_str());
    }
}

void OrderPairStateMachine::checkSellState(
    OrderPair &pair,
    bool force)
{
    if (pair.state != OrderPair::State::SellActive)
        return;

    CoinbaseOrderBook &orderbook = ctx.data.get<CoinbaseOrderBook>();

    CoinbaseOrder sellOrder = orderbook.getOrder(pair.sellOrder);
    if (!sellOrder || force)
    {
        sellOrder = ctx.coinbase().getOrder(pair.sellOrder);
        if (sellOrder)
            orderbook.update(sellOrder);
    }

    if (!sellOrder)
    {
        pair.state = OrderPair::State::Error;
        log::error("Failed to find sell order '%s' for order pair '%s'.",
            pair.sellOrder.c_str(),
            pair.uuid.c_str());
    }
    else if (sellOrder.state == CoinbaseOrder::State::Open)
    {
        // Still active sell
    }
    else if (sellOrder.state == CoinbaseOrder::State::Filled)
    {
        // Complete
        pair.state = OrderPair::State::Complete;
        // Record the price it filled at
        pair.sellPrice = sellOrder.priceCents;
        pair.profit.sold = sellOrder.beforeFees;
        pair.profit.sellFees = sellOrder.fees;
        // Track total profits
        ctx.data.get<Profits>().addOrderPair(pair.profit);
    }
    else if (sellOrder.state == CoinbaseOrder::State::Canceled)
    {
        // Sell canceled -> Back to holding
        pair.state = OrderPair::State::Holding;
    }
    else //if (sellOrder.state == CoinbaseOrder::state::None || sellOrder.state == CoinbaseOrder::state::Error)
    {
        pair.state = OrderPair::State::Error;
        log::error("Sell order '%s' for order pair '%s' in error state.",
            pair.sellOrder.c_str(),
            pair.uuid.c_str());
    }
}

void OrderPairStateMachine::handlePending(
    OrderPair &pair,
    const BtcPrice &price)
{
    // Wait for price to go down
    if (pair.buyPrice < price.getCents())
        return;

    // Too soon?
    // XXX: Going to make sure we only run 1 trade per 30 seconds while testing
    if (nextTrade > std::chrono::steady_clock::now())
    {
        log::info("Can't buy: too soon.");
        return;
    }

    // Not enough money to buy
    if (pair.betCents > ctx.data.get<CoinbaseWallet>().getUsdCents())
        return;

    // Try to place new order
    CoinbaseOrder order;
    order.buy = true;
    order.setQuantity(price.getCents() - 200, pair.betCents);
    order.createdTime = ctx.data.get<Time>().getTime();

    if (ctx.coinbase().submitOrder(order))
    {
        pair.state = OrderPair::State::BuyActive;
        pair.buyOrder = order.uuid;

        log::info("Created new '%s' buy order '%s' for pair '%s'.",
            name.c_str(),
            pair.buyOrder.c_str(),
            pair.uuid.c_str());

        // Update wallet after order created
        ctx.data.get<CoinbaseWallet>().update(ctx.coinbase().getWallet());

        // XXX: Going to make sure we only run 1 trade per 30 seconds while testing
        nextTrade = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    }
    else
    {
        // TODO: Different retry times for INVALID_LIMIT_PRICE_POST_ONLY and other errors
        pair.nextTry = std::chrono::steady_clock::now() + std::chrono::seconds(FAILURE_RETRY_SECONDS);
        log::error("Failed to create buy order for spread '%s' pair '%s' ($%s).",
            name.c_str(),
            pair.uuid.c_str(),
            IntegerUtils::centsToUsd(order.valueCents()).c_str());
    }
}

void OrderPairStateMachine::handleBuyActive(
    OrderPair &pair,
    const BtcPrice &price)
{
    // It should fill any time now...
    if (pair.buyPrice > price.getCents())
        return;

    // Still close enough to the desired buy price
    if (pair.buyPrice + KEEP_ACTIVE_ORDER_WINDOW_CENTS > price.getCents())
        return;

    // If price has risen too much, cancel buy order so we get the liquidity back
    if (ctx.coinbase().cancelOrder(pair.buyOrder))
        pair.state = OrderPair::State::Pending;
    // Failed to cancel, let's see if something about the order state changed
    else
    {
        pair.nextTry = std::chrono::steady_clock::now() + std::chrono::seconds(FAILURE_RETRY_SECONDS);
        checkBuyState(pair, true);
    }
}

void OrderPairStateMachine::handleHolding(
    OrderPair &pair,
    const BtcPrice &price)
{
    // Wait for price to to up
    if (pair.sellPrice > price.getCents())
        return;

    // Too soon?
    // XXX: Going to make sure we only run 1 trade per 30 seconds while testing
    if (nextTrade > std::chrono::steady_clock::now())
    {
        log::info("Can't sell: too soon.");
        return;
    }

    // Try to place new order
    CoinbaseOrder order;
    order.buy = false;
    order.priceCents = price.getCents() + 200;
    order.quantity = pair.quantity;
    order.createdTime = ctx.data.get<Time>().getTime();

    if (ctx.coinbase().submitOrder(order))
    {
        pair.state = OrderPair::State::SellActive;
        pair.sellOrder = order.uuid;

        log::info("Created new '%s' sell order '%s' for pair '%s'.",
            name.c_str(),
            pair.sellOrder.c_str(),
            pair.uuid.c_str());

        // Update wallet after order created
        ctx.data.get<CoinbaseWallet>().update(ctx.coinbase().getWallet());

        // XXX: Going to make sure we only run 1 trade per 30 seconds while testing
        nextTrade = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    }
    else
    {
        pair.nextTry = std::chrono::steady_clock::now() + std::chrono::seconds(FAILURE_RETRY_SECONDS);
        log::error("Failed to create sell order for spread '%s' pair '%s' ($%s).",
            name.c_str(),
            pair.uuid.c_str(),
            IntegerUtils::centsToUsd(order.valueCents()).c_str());
    }
}

void OrderPairStateMachine::handleSellActive(
    OrderPair &pair,
    const BtcPrice &price)
{
    // It should fill any time now...
    if (pair.sellPrice < price.getCents())
        return;

    // Still close enough to the desired sell price
    if (pair.sellPrice < price.getCents() + KEEP_ACTIVE_ORDER_WINDOW_CENTS)
        return;

    // If price has fallen too much, cancel sell order so we get the liquidity back
    if (ctx.coinbase().cancelOrder(pair.sellOrder))
        pair.state = OrderPair::State::Holding;
    // Failed to cancel, let's see if something about the order state changed
    else
    {
        pair.nextTry = std::chrono::steady_clock::now() + std::chrono::seconds(FAILURE_RETRY_SECONDS);
        checkSellState(pair);
    }
}

void OrderPairStateMachine::logChange(
    OrderPair::State startState,
    const OrderPair &pair)
{
    log::info("Spread '%s' pair '%s' updated '%s' => '%s' ($%s -> $%s) @ ($%s -> $%s).",
        name.c_str(),
        pair.uuid.c_str(),
        to_string(startState).c_str(),
        to_string(pair.state).c_str(),
        // $ investment
        IntegerUtils::centsToUsd(pair.betCents).c_str(),
        // Final sale value
        IntegerUtils::centsToUsd(pair.sellValue()).c_str(),
        // Buy BTC price
        IntegerUtils::centsToUsd(pair.buyPrice).c_str(),
        // Sell BTC price
        IntegerUtils::centsToUsd(pair.sellPrice).c_str());
}
