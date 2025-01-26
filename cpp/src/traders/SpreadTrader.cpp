#include <gtb/SpreadTrader.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/OrderPairDb.h>
#include <gtb/Time.h>
#include <gtb/Uuid.h>
#include <gtb/Log.h>

using namespace gtb;

SpreadTrader::SpreadTrader(
    BotContext &ctx,
    Config confIn)
        : ctx(ctx)
        , conf(std::move(confIn))
        , stateMachine(ctx, db, conf.name)
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

    for (OrderPair &pair : orderPairs)
        pair.betCents = conf.cents;
}

void SpreadTrader::process(
    const BtcPrice &price)
{
    if (!ctx.data.get<CoinbaseInit>())
        return;

    std::lock_guard<std::mutex> lock(mtx);

    // Check if we need to do anything for the existing pairs
    handleExistingPairs();

    // Check if we want to create a new pair (which might make us drop a pending one)
    handleNewPair(price);
}

void SpreadTrader::process(
    const CoinbaseOrderBook &orderbook)
{
    (void)orderbook;

    // Update wallet on orderbook change
    // TODO: Don't do this 'n' times (once per trader)
    ctx.data.get<CoinbaseWallet>().update(ctx.coinbase().getWallet());

    // Check state of each pair
    // Force querying order information from Coinbase (don't trust orderbook cache)
    std::lock_guard<std::mutex> lock(mtx);
    handleExistingPairs(true);
}

void SpreadTrader::handleExistingPairs(
    bool force)
{
    // Handle each order pair
    stateMachine.churn(orderPairs, force);

    // Stop tracking completed states
    auto iter = orderPairs.begin();
    while (iter != orderPairs.end())
    {
        OrderPair &pair = (*iter);

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
    pair.betCents = conf.cents;
    pair.quantity = IntegerUtils::getSatoshiForPrice(pair.buyPrice, pair.betCents);

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
    stateMachine.logChange(OrderPair::State::None, pair);
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
