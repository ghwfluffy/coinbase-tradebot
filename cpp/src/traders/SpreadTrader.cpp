#include <gtb/SpreadTrader.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/OrderPairDb.h>
#include <gtb/Time.h>
#include <gtb/Uuid.h>
#include <gtb/Log.h>

using namespace gtb;

namespace
{

// If an order fails to post, wait this long before retrying the spread
constexpr const unsigned int FAILURE_RETRY_SECONDS = 10;

// Keep orders active if they're within $100 of our target price
constexpr const uint32_t KEEP_ACTIVE_ORDER_WINDOW_CENTS = 100'00;

}

SpreadTrader::SpreadTrader(
    BotContext &ctx,
    Config conf)
        : ctx(ctx)
        , conf(std::move(conf))
{
    loadDatabase();
}

void SpreadTrader::loadDatabase()
{
    db.init("spread_trader.sqlite", "./schema/spread_trader.sql");
    orderPairs = OrderPairDb::select(db, conf.name);
    log::info("Read %zu pairs for spread '%s' from database.",
        orderPairs.size(),
        conf.name.c_str());
}

void SpreadTrader::process(
    const BtcPrice &price)
{
    if (!ctx.data.get<CoinbaseInit>())
        return;

    std::lock_guard<std::mutex> lock(mtx);

    // Check if we need to do anything for the existing pairs
    handleExistingPairs(price);

    // Check if we want to create a new pair (which might make us drop a pending one)
    handleNewPair(price);
}

void SpreadTrader::handleExistingPairs(
    const BtcPrice &price)
{
    // Decide if it's time to act on an existing order pair
    auto iter = orderPairs.begin();
    while (iter != orderPairs.end())
    {
        OrderPair &pair = (*iter);
        OrderPair::State startState = pair.state;

        // This pair is not ready to act yet
        if (pair.nextTry > std::chrono::steady_clock::now())
        {
            ++iter;
            continue;
        }

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

        // Update database
        if (pair.state != startState)
        {
            logChange(startState, pair);
            OrderPairDb::update(db, pair);
        }

        // Stop tracking completed states
        if (pair.state >= OrderPair::State::Complete)
        {
            log::info("Removing completed order pair '%s' from spread trader '%s'.",
                pair.uuid.c_str(),
                conf.name.c_str());
            iter = orderPairs.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

void SpreadTrader::handleNewPair(
    const BtcPrice &price)
{
    // Find the closest order pair
    bool first = true;
    uint32_t closestDistance = 0;
    for (const OrderPair &pair : orderPairs)
    {
        int32_t buyDistance = static_cast<int32_t>(pair.buyPrice - price.getCents());
        if (buyDistance < 0)
            buyDistance *= -1;

        int32_t sellDistance = static_cast<int32_t>(pair.sellPrice - price.getCents());
        if (sellDistance < 0)
            sellDistance *= -1;

        int32_t midDistance = static_cast<int32_t>(
            ((pair.sellPrice + pair.buyPrice) / 2) - price.getCents());
        if (midDistance < 0)
            midDistance *= -1;

        uint32_t distance = static_cast<uint32_t>(buyDistance);
        if (sellDistance < static_cast<int32_t>(distance))
            distance = static_cast<uint32_t>(sellDistance);
        if (midDistance < static_cast<int32_t>(distance))
            distance = static_cast<uint32_t>(midDistance);

        if (first || distance < closestDistance)
            closestDistance = distance;
        first = false;
    }

    // Check if that's far enough away to want to create a new spread
    uint32_t spread_cents = (((price.getCents() * conf.spread) + 9'999) / 10'000);
    bool wantNew = first || (closestDistance > ((spread_cents * conf.buffer_percent) / 100));

    // Not far enough to consider a new pair
    if (!wantNew)
        return;

    // Setup the pair
    OrderPair pair;
    pair.algo = conf.name;
    pair.uuid = Uuid::generate();
    pair.state = OrderPair::State::Pending;
    pair.created = ctx.data.get<Time>().getTime();
    uint32_t half_spread = (spread_cents + 1) / 2;
    pair.buyPrice = price.getCents() - half_spread;
    pair.sellPrice = price.getCents() + half_spread;
    pair.quantity = IntegerUtils::getSatoshiForPrice(pair.buyPrice, conf.cents);

    // We need to make sure the sell price isn't too far outside today's estimated value of BTC
    // TODO: Make this based on config file
    // TODO: Give some leeway for making a single trade slightly over estimated value
    constexpr const uint32_t BTC_VALUE = 105'000'00;
    if (pair.sellPrice > BTC_VALUE)
        return;

    // We need to decide if we can cancel something first
    // Try to cancel the furthest pending pair
    while (conf.num_pairs >= orderPairs.size() && cancelPending(price))
    { }

    // We have too many orders already and we haven't been patient enough to exceed max
    if (conf.num_pairs <= orderPairs.size() && !patientOverride())
        return;

    // Add the pair
    if (!OrderPairDb::insert(db, pair))
    {
        log::error("Failed to insert new order pair for spread '%s' in database.",
            conf.name.c_str());
        return;
    }

    log::info("Created new pair for spread '%s'.", conf.name.c_str());
    logChange(OrderPair::State::None, pair);
    orderPairs.push_back(std::move(pair));
}

bool SpreadTrader::patientOverride() const
{
    uint64_t mostRecentTime = 0;
    for (const OrderPair &pair : orderPairs)
    {
        if (pair.created > mostRecentTime)
            mostRecentTime = pair.created;
    }

    // Can put in 1 new trade every half hour over our limit
    constexpr const uint64_t PATIENCE_MICROSECONDS = 30ULL * 60ULL * 1'000'000ULL;
    return mostRecentTime + PATIENCE_MICROSECONDS <= ctx.data.get<Time>().getTime();
}

bool SpreadTrader::cancelPending(
    const BtcPrice &price)
{
    // Find the further away pair that's still pending
    std::string furthestUuid;
    uint32_t furthestDistance = 0;
    for (const OrderPair &pair : orderPairs)
    {
        // Only pending
        if (pair.state > OrderPair::State::Pending)
            continue;

        int32_t buyDistance = static_cast<int32_t>(pair.buyPrice - price.getCents());
        if (buyDistance < 0)
            buyDistance *= -1;

        int32_t sellDistance = static_cast<int32_t>(pair.sellPrice - price.getCents());
        if (sellDistance < 0)
            sellDistance *= -1;

        uint32_t distance = 0;
        if (buyDistance < sellDistance)
            distance = static_cast<uint32_t>(buyDistance);
        else
            distance = static_cast<uint32_t>(sellDistance);

        if (furthestUuid.empty() || distance > furthestDistance)
        {
            furthestUuid = pair.uuid;
            furthestDistance = distance;
        }
    }

    // Nothing to cancel
    if (furthestUuid.empty())
        return false;

    // Find in list to remove
    auto iter = orderPairs.begin();
    while (iter != orderPairs.end())
    {
        OrderPair &pair = (*iter);
        if (pair.uuid != furthestUuid)
        {
            ++iter;
            continue;
        }

        // Set to canceled
        OrderPair clone(pair);
        clone.state = OrderPair::State::Canceled;
        if (!OrderPairDb::update(db, clone))
        {
            log::error("Failed to remove order pair '%s' for '%s' from database.",
                pair.uuid.c_str(),
                conf.name.c_str());
            return false;
        }

        log::info("Removed stale pair '%s' for '%s'.",
            pair.uuid.c_str(),
            conf.name.c_str());

        // Remove from tracking
        orderPairs.erase(iter);

        return true;
    }

    return false;
}

void SpreadTrader::process(
    const CoinbaseOrderBook &orderbook)
{
    (void)orderbook;

    // Update wallet on orderbook change
    ctx.data.get<CoinbaseWallet>().update(ctx.coinbase().getWallet());

    std::lock_guard<std::mutex> lock(mtx);

    // Update order pairs with any changes
    auto iter = orderPairs.begin();
    while (iter != orderPairs.end())
    {
        OrderPair &pair = (*iter);
        OrderPair::State startState = pair.state;

        // Check active buys are in the expected state
        checkBuyState(pair);

        // Check active sells are in the expected state
        checkSellState(pair);

        // Update database
        if (pair.state != startState)
        {
            logChange(startState, pair);
            OrderPairDb::update(db, pair);
        }

        // Stop tracking completed states
        if (pair.state >= OrderPair::State::Complete)
        {
            log::info("Removing completed order pair '%s' from spread trader '%s'.",
                pair.uuid.c_str(),
                conf.name.c_str());
            iter = orderPairs.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

void SpreadTrader::checkBuyState(
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

void SpreadTrader::checkSellState(
    OrderPair &pair)
{
    if (pair.state != OrderPair::State::SellActive)
        return;

    CoinbaseOrderBook &orderbook = ctx.data.get<CoinbaseOrderBook>();

    CoinbaseOrder sellOrder = orderbook.getOrder(pair.sellOrder);
    if (!sellOrder)
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

void SpreadTrader::handlePending(
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

    // Try to place new order
    CoinbaseOrder order;
    order.buy = true;
    order.setQuantity(price.getCents() - 200, conf.cents);
    order.createdTime = ctx.data.get<Time>().getTime();

    if (ctx.coinbase().submitOrder(order))
    {
        pair.state = OrderPair::State::BuyActive;
        pair.buyOrder = order.uuid;

        log::info("Created new '%s' buy order '%s' for pair '%s'.",
            conf.name.c_str(),
            pair.buyOrder.c_str(),
            pair.uuid.c_str());

        // Update wallet after order created
        ctx.data.get<CoinbaseWallet>().update(ctx.coinbase().getWallet());

        // XXX: Going to make sure we only run 1 trade per 30 seconds while testing
        nextTrade = std::chrono::steady_clock::now() + std::chrono::seconds(30);
    }
    else
    {
        // TODO: Different retry times for INVALID_LIMIT_PRICE_POST_ONLY and other errors
        pair.nextTry = std::chrono::steady_clock::now() + std::chrono::seconds(FAILURE_RETRY_SECONDS);
        log::error("Failed to create buy order for spread '%s' pair '%s' ($%s).",
            conf.name.c_str(),
            pair.uuid.c_str(),
            IntegerUtils::centsToUsd(order.valueCents()).c_str());
    }
}

void SpreadTrader::handleBuyActive(
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
        checkBuyState(pair, true);
}

void SpreadTrader::handleHolding(
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
        log::info("Can't buy: too soon.");
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
            conf.name.c_str(),
            pair.sellOrder.c_str(),
            pair.uuid.c_str());

        // Update wallet after order created
        ctx.data.get<CoinbaseWallet>().update(ctx.coinbase().getWallet());

        // XXX: Going to make sure we only run 1 trade per 30 seconds while testing
        nextTrade = std::chrono::steady_clock::now() + std::chrono::seconds(30);
    }
    else
    {
        pair.nextTry = std::chrono::steady_clock::now() + std::chrono::seconds(FAILURE_RETRY_SECONDS);
        log::error("Failed to create sell order for spread '%s' pair '%s' ($%s).",
            conf.name.c_str(),
            pair.uuid.c_str(),
            IntegerUtils::centsToUsd(order.valueCents()).c_str());
    }
}

void SpreadTrader::handleSellActive(
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
        checkSellState(pair);
}

void SpreadTrader::logChange(
    OrderPair::State startState,
    const OrderPair &pair)
{
    log::info("Spread '%s' pair '%s' updated '%s' => '%s' ($%s -> $%s) @ ($%s -> $%s).",
        conf.name.c_str(),
        pair.uuid.c_str(),
        to_string(startState).c_str(),
        to_string(pair.state).c_str(),
        // $ investment
        IntegerUtils::centsToUsd(conf.cents).c_str(),
        // Final sale value
        IntegerUtils::centsToUsd(pair.sellValue()).c_str(),
        // Buy BTC price
        IntegerUtils::centsToUsd(pair.buyPrice).c_str(),
        // Sell BTC price
        IntegerUtils::centsToUsd(pair.sellPrice).c_str());
}
