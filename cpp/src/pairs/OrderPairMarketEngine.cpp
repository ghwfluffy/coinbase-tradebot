#include <gtb/OrderPairMarketEngine.h>
#include <gtb/MarketInfo.h>
#include <gtb/Uuid.h>
#include <gtb/Log.h>

using namespace gtb;

namespace
{

struct MarketPeriodModifier
{
    std::string periodName;
    MarketInfo::Market market = MarketInfo::Market::None;

    MarketPeriodConfig period;
    uint64_t timeIntoBuffer = 0;
};

// Determine what modifiers should be applied to the order pair
std::list<MarketPeriodModifier> getPeriodModifiers(
    const BaseTraderConfig &config,
    uint64_t time)
{
    std::list<MarketPeriodModifier> modifiers;

    for (const MarketTimeTraderConfig &params : config.marketParams)
    {
        MarketInfo marketTime(params.market, time);

        MarketPeriodModifier modifier;

        // Market is open
        if (marketTime.isOpen())
        {
            uint64_t tillClosed = marketTime.tillClosed();
            // Approaching the end of the week
            if (marketTime.isWeekendNext() &&
                tillClosed <= params.weekendingMarket.bufferPeriod())
            {
                modifier.periodName = "Weekending";
                modifier.period = params.weekendingMarket;
                modifier.timeIntoBuffer = params.weekendingMarket.bufferPeriod() - tillClosed;
            }
            // Approaching a normal close
            else if (tillClosed <= params.closingMarket.bufferPeriod())
            {
                modifier.periodName = "Closing";
                modifier.period = params.closingMarket;
                modifier.timeIntoBuffer = params.closingMarket.bufferPeriod() - tillClosed;
            }
            // Start/Middle of an open market
            else
            {
                modifier.periodName = "Open";
                modifier.period = params.openMarket;
                modifier.timeIntoBuffer = marketTime.sinceOpen();
            }
        }
        // Market is closed for the weekend
        else if (marketTime.isWeekend())
        {
            uint64_t tillOpen = marketTime.tillOpen();
            // Weekend is coming to a close and market starting up again
            if (tillOpen <= params.weekStartingMarket.bufferPeriod())
            {
                modifier.periodName = "Week starting";
                modifier.period = params.weekStartingMarket;
                modifier.timeIntoBuffer = params.weekStartingMarket.bufferPeriod() - tillOpen;
            }
            // Start or middle of the weekend
            else
            {
                modifier.periodName = "Weekend";
                modifier.period = params.weekendMarket;
                modifier.timeIntoBuffer = marketTime.sinceClosed();
            }
        }
        // Market is closed on a week day
        else if (marketTime.isClosed())
        {
            uint64_t tillOpen = marketTime.tillOpen();
            // Market is approaching open time
            if (tillOpen <= params.openingMarket.bufferPeriod())
            {
                modifier.periodName = "Opening";
                modifier.period = params.openingMarket;
                modifier.timeIntoBuffer = params.openingMarket.bufferPeriod() - tillOpen;
            }
            // Start or middle of closed period
            else
            {
                modifier.periodName = "Closed";
                modifier.period = params.closedMarket;
                modifier.timeIntoBuffer = marketTime.sinceClosed();
            }
        }
        else
        {
            log::error("Market time calculation invalid for market %d time %lu.",
                static_cast<int>(params.market),
                static_cast<unsigned long>(time));
        }

        if (!modifier.periodName.empty())
            modifiers.push_back(modifier);
    }

    return modifiers;
}

// New pair according to the base trader config
OrderPair newPair(
    const BaseTraderConfig &config,
    uint64_t currentTime)
{
    OrderPair pair;
    pair.uuid = Uuid::generate();
    pair.algo = config.name;
    pair.betCents = config.cents;
    pair.created = currentTime;
    pair.state = OrderPair::State::Pending;
    return pair;
}

void applyNewOrderModifier(
    const MarketPeriodModifier &modifier,
    OrderPair &pair)
{
    auto setDesc = [&](const char *desc = nullptr) -> void
    {
        std::string mod = "buy:" + to_string(modifier.market) + " " + modifier.periodName;
        if (desc)
        {
            mod += ' ';
            mod += desc;
        }

        pair.modifiers.push_back(std::move(mod));
    };

    // Check how long the pause/ramp lasts
    uint64_t bufferPeriod = modifier.period.bufferPeriod();

    // This period just straight disables buying
    if (!modifier.period.hot && !bufferPeriod)
    {
        pair.betCents = 0;
        setDesc("disabled");
        return;
    }

    // We are passed the buffer and this period disables buying
    if (!modifier.period.hot && modifier.timeIntoBuffer > bufferPeriod)
    {
        pair.betCents = 0;
        setDesc("full pause");
        return;
    }

    // We are passed the buffer and this period enables buying
    if (modifier.period.hot && modifier.timeIntoBuffer > bufferPeriod)
    {
        // Modifier is not applied, just normal buying
        pair.modifiers.push_back(modifier.periodName);
        setDesc();
        return;
    }

    // Are we in the pause period or the ramp period
    bool pause = true;
    if (!modifier.period.hot && modifier.timeIntoBuffer < modifier.period.rampPeriod)
        pause = false;
    else if (modifier.period.hot && modifier.timeIntoBuffer > modifier.period.pausePeriod)
        pause = false;

    // Pause period, disable buying
    if (pause)
    {
        pair.betCents = 0;
        setDesc("pause");
        return;
    }

    // How far into the ramp period are we?
    uint64_t intoRampPeriod = 0;
    if (modifier.period.hot)
        intoRampPeriod = modifier.timeIntoBuffer - modifier.period.pausePeriod;
    else
        intoRampPeriod = modifier.timeIntoBuffer;

    // Calculate where we are on the ramp
    uint64_t rampPercent = ((intoRampPeriod * modifier.period.rampGrade) / modifier.period.rampPeriod);
    // Ramping down
    if (!modifier.period.hot)
        rampPercent = modifier.period.rampGrade - rampPercent;

    // Just double super check we're not going to rollover these unsigned with subtraction
    if (pair.sellPrice < pair.buyPrice)
        pair.sellPrice = pair.buyPrice;

    // Calculate the spread of this order pair
    uint32_t mid = (pair.sellPrice + pair.buyPrice + 1) / 2;
    if (!mid)
    {
        pair.betCents = 0;
        log::error("Invalid prices when calculating '%s %s' ramp modifier.",
            to_string(modifier.market).c_str(),
            modifier.periodName.c_str());
        return;
    }

    uint32_t diff = pair.sellPrice - pair.buyPrice;
    uint32_t spread = (diff * 10'000) / mid;
    if (!spread)
        spread = 1;

    // Apply ramp
    spread += static_cast<uint32_t>((spread * rampPercent) / 10'000);
    diff = (mid * spread) / 10'000;

    // Recalculate buy/sell pairs
    uint32_t buyPrice = mid - diff;
    uint32_t sellPrice = mid + diff;

    // Be super sure our math didn't roll something over
    if (buyPrice > pair.buyPrice || sellPrice < pair.sellPrice)
    {
        log::error("Overflow while calculating '%s %s' ramp modifier.",
            to_string(modifier.market).c_str(),
            modifier.periodName.c_str());
        return;
    }

    // Apply new buy/sell price
    pair.buyPrice = buyPrice;
    pair.sellPrice = sellPrice;
    setDesc("ramp");
}

void applyNewOrderModifiers(
    const BaseTraderConfig &config,
    uint64_t currentTime,
    OrderPair &pair)
{
    // Get modifiers based on market value
    std::list<MarketPeriodModifier> modifiers = getPeriodModifiers(config, currentTime);

    // Apply
    for (const MarketPeriodModifier &modifier : modifiers)
        applyNewOrderModifier(modifier, pair);

    // We need to make sure the sell price isn't passed the max config
    if (pair && pair.sellPrice > config.maxValue)
        pair = OrderPair();

    // Update quantity based on buy parameters
    if (pair)
    {
        pair.quantity = IntegerUtils::getSatoshiForPrice(pair.buyPrice, pair.betCents);
        pair.origSellPrice = pair.sellPrice;
    }
}

void applySellModifier(
    const MarketPeriodModifier &modifier,
    OrderPair &pair)
{
    auto setDesc = [&](const char *desc = nullptr) -> void
    {
        std::string mod = "sell:" + to_string(modifier.market) + " " + modifier.periodName;
        if (desc)
        {
            mod += ' ';
            mod += desc;
        }

        pair.modifiers.push_back(std::move(mod));
    };

    // We don't discount sales for this modifier
    if (!modifier.period.pauseAcceptLoss)
    {
        setDesc();
        return;
    }

    // This is a hot period and we are through the ramp/pause period
    if (modifier.period.hot && modifier.timeIntoBuffer >= modifier.period.bufferPeriod())
    {
        setDesc();
        return;
    }

    // How far into the ramp period are we?
    uint64_t intoRampPeriod = 0;
    if (modifier.period.hot)
    {
        // Are we out of the pause period yet?
        if (modifier.period.pausePeriod < modifier.timeIntoBuffer)
            intoRampPeriod = modifier.timeIntoBuffer - modifier.period.pausePeriod;
    }
    else
    {
        // Are we to the pause period already?
        if (modifier.timeIntoBuffer > modifier.period.rampPeriod)
            intoRampPeriod = modifier.period.rampPeriod;
        else
            intoRampPeriod = modifier.timeIntoBuffer;
    }

    // Calculate where we are on the ramp
    uint64_t rampPercent = ((intoRampPeriod * modifier.period.pauseAcceptLoss) / modifier.period.rampPeriod);
    // Ramping down losses (ramping up to be active)
    if (modifier.period.hot)
        rampPercent = modifier.period.pauseAcceptLoss - rampPercent;

    // Just double super check we're not going to rollover these unsigned with subtraction
    if (pair.origSellPrice < pair.buyPrice)
        pair.origSellPrice = pair.buyPrice;

    // Calculate the new sell price
    uint32_t diff = pair.origSellPrice - pair.buyPrice;
    uint32_t less = static_cast<uint32_t>((diff * rampPercent) / 10'000);
    if (pair.sellPrice <= less)
    {
        log::error("Invalid discount amount exceeds sale price calculated for '%s %s' sell ramp modifier.",
            to_string(modifier.market).c_str(),
            modifier.periodName.c_str());
        return;
    }

    pair.sellPrice -= less;
    setDesc("ramp");
}

}

OrderPair OrderPairMarketEngine::newSpread(
    const BaseTraderConfig &config,
    uint32_t currentBtcPrice,
    uint64_t currentTime,
    uint32_t spread)
{
    if (!config.enabled)
        return OrderPair();

    // Setup pair
    OrderPair pair = newPair(config, currentTime);

    uint32_t spread_cents = (((currentBtcPrice * spread) + 9'999) / 10'000);
    uint32_t half_spread = (spread_cents + 1) / 2;
    pair.buyPrice = currentBtcPrice - half_spread;
    pair.sellPrice = currentBtcPrice + half_spread;

    // Apply modifiers
    applyNewOrderModifiers(config, currentTime, pair);

    return pair;
}

OrderPair OrderPairMarketEngine::newStatic(
    const BaseTraderConfig &config,
    uint64_t currentTime,
    uint32_t buyPrice,
    uint32_t sellPrice)
{
    if (!config.enabled)
        return OrderPair();

    // Setup pair
    OrderPair pair = newPair(config, currentTime);
    pair.buyPrice = buyPrice;
    pair.sellPrice = sellPrice;

    // Apply modifiers
    applyNewOrderModifiers(config, currentTime, pair);

    return pair;
}

void OrderPairMarketEngine::checkSale(
    OrderPair &pair,
    const BaseTraderConfig &config,
    uint64_t currentTime)
{
    // This only applies to BTC we're holding and may want to discount
    if (pair.state != OrderPair::State::Holding)
        return;

    // Cache the original sell price,
    // Or restore from it
    if (!pair.origSellPrice)
        pair.origSellPrice = pair.sellPrice;

    // Clear old sale modifiers
    pair.sellPrice = pair.origSellPrice;
    auto iter = pair.modifiers.begin();
    while (iter != pair.modifiers.end())
    {
        if (iter->rfind("sell:", 0) == 0)
            iter = pair.modifiers.erase(iter);
        else
            ++iter;
    }

    // Get modifiers based on market value
    std::list<MarketPeriodModifier> modifiers = getPeriodModifiers(config, currentTime);

    // Apply
    for (const MarketPeriodModifier &modifier : modifiers)
        applySellModifier(modifier, pair);
}
