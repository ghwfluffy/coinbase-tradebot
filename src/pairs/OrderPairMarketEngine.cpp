#include <gtb/OrderPairMarketEngine.h>
#include <gtb/MarketInfo.h>
#include <gtb/Uuid.h>
#include <gtb/Log.h>
#include <gtb/BigInt.h>

#include <cassert>
#include <algorithm>
#include <limits>
#include <mutex>
#include <unordered_map>

using namespace gtb;

namespace
{

struct MarketPeriodModifier
{
    std::string periodName;
    MarketInfo::Market market = MarketInfo::Market::None;

    MarketPeriodConfig period;
    utime_t timeIntoBuffer;
};

pp_t safeFractionPp(
    uint64_t numerator,
    uint64_t denominator)
{
    if (!denominator)
        return pp_t();

    BigInt num(numerator);
    num *= BigInt(PP_SCALE);
    BigInt den(denominator);
    BigInt res = num / den;
    uint64_t capped = std::min<uint64_t>(
        res.toUint64(),
        static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()));
    return pp_t(static_cast<uint32_t>(capped));
}

pp_t safeFractionUsd(
    usd_t numerator,
    usd_t denominator)
{
    if (!denominator)
        return pp_t();

    BigInt num(numerator.value());
    num *= BigInt(PP_SCALE);
    BigInt den(denominator.value());
    BigInt res = num / den;
    uint64_t capped = std::min<uint64_t>(
        res.toUint64(),
        static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()));
    return pp_t(static_cast<uint32_t>(capped));
}

// Determine what modifiers should be applied to the order pair
std::vector<MarketPeriodModifier> getPeriodModifiers(
    const BaseTraderConfig &config,
    utime_t time)
{
    std::vector<MarketPeriodModifier> modifiers;

    for (const MarketTimeTraderConfig &params : config.marketParams)
    {
        MarketInfo marketTime(params.market, time);

        MarketPeriodModifier modifier;
        modifier.market = params.market;

        // Market is open
        if (marketTime.isOpen())
        {
            utime_t tillClosed = marketTime.tillClosed();
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
            utime_t tillOpen = marketTime.tillOpen();
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
            utime_t tillOpen = marketTime.tillOpen();
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
                static_cast<unsigned long>(time.value()));
        }

        if (!modifier.periodName.empty())
            modifiers.push_back(modifier);
    }

    return modifiers;
}

// Cache modifiers per config pointer in coarse buckets to avoid recomputing every price tick in mocks.
std::vector<MarketPeriodModifier> getPeriodModifiersCached(
    const BaseTraderConfig &config,
    utime_t time)
{
    constexpr utime_t CACHE_GRANULARITY = 60_Seconds; // recompute at most once per minute of mock time

    struct CacheEntry
    {
        utime_t bucket = {};
        std::vector<MarketPeriodModifier> modifiers;
    };

    static std::mutex cacheMtx;
    static std::unordered_map<const BaseTraderConfig *, CacheEntry> cache;

    utime_t bucket = utime_t((time.value() / CACHE_GRANULARITY.value()) * CACHE_GRANULARITY.value());

    std::lock_guard<std::mutex> lock(cacheMtx);
    CacheEntry &entry = cache[&config];
    if (entry.bucket != bucket)
    {
        entry.bucket = bucket;
        entry.modifiers = getPeriodModifiers(config, time);
    }
    return entry.modifiers;
}

// New pair according to the base trader config
OrderPair newPair(
    const BaseTraderConfig &config,
    utime_t currentTime)
{
    OrderPair pair;
    pair.uuid = Uuid::generate();
    pair.algo = config.name;
    pair.bet = config.bet;
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
    utime_t bufferPeriod = modifier.period.bufferPeriod();

    // This period just straight disables buying
    if (!modifier.period.hot && !bufferPeriod)
    {
        pair.bet = {};
        setDesc("disabled");
        return;
    }

    // We are passed the buffer and this period disables buying
    if (!modifier.period.hot && modifier.timeIntoBuffer >= bufferPeriod)
    {
        pair.bet = {};
        setDesc("full pause");
        return;
    }

    // We are passed the buffer and this period enables buying
    if (modifier.period.hot && modifier.timeIntoBuffer >= bufferPeriod)
    {
        // Modifier is not applied, just normal buying
        pair.modifiers.push_back(modifier.periodName);
        setDesc();
        return;
    }

    // Are we in the pause period or the ramp period
    bool pause = true;
    if (!modifier.period.hot && modifier.timeIntoBuffer <= modifier.period.rampPeriod)
        pause = false;
    else if (modifier.period.hot && modifier.timeIntoBuffer >= modifier.period.pausePeriod)
        pause = false;

    // Pause period, disable buying
    if (pause)
    {
        pair.bet = {};
        setDesc("pause");
        return;
    }

    // How far into the ramp period are we?
    utime_t intoRampPeriod;
    if (modifier.period.hot)
        intoRampPeriod = modifier.timeIntoBuffer - modifier.period.pausePeriod;
    else
        intoRampPeriod = modifier.timeIntoBuffer;

    // Calculate where we are on the ramp
    pp_t rampPercent = safeFractionPp(
        (intoRampPeriod * modifier.period.rampGrade).value(),
        modifier.period.rampPeriod.value());
    // Hot=Ramping down the handicap (ramp percent is added to initial spread)
    if (modifier.period.hot)
        rampPercent = modifier.period.rampGrade - rampPercent;

    // Just double super check we're not going to rollover these unsigned with subtraction
    if (pair.sellPrice < pair.buyPrice)
        pair.sellPrice = pair.buyPrice;

    // Calculate the spread of this order pair
    usd_t mid = IntegerUtils::avg(pair.sellPrice, pair.buyPrice);
    if (!mid)
    {
        pair.bet = {};
        log::error("Invalid prices when calculating '%s %s' ramp modifier.",
            to_string(modifier.market).c_str(),
            modifier.periodName.c_str());
        return;
    }

    usd_t diff = pair.sellPrice - pair.buyPrice;
    pp_t spread = safeFractionUsd(diff, mid);
    if (!spread)
        spread = pp_t(1);

    // Apply ramp
    pp_t additionalSpread = spread * rampPercent;
    diff = mid * (spread + additionalSpread);
    usd_t diffHalf = diff * 50_Percent;

    // Recalculate buy/sell pairs
    usd_t buyPrice = mid - diffHalf;
    usd_t sellPrice = mid + diffHalf;

    // Be super sure our math didn't roll something over
    if (buyPrice > pair.buyPrice || sellPrice < pair.sellPrice)
    {
        log::error("Overflow while calculating '%s %s' ramp modifier.\n"
                    "[buy %s - %s], [sell %s - %s] - mid %s\n"
                    "%lu spread + %lu additional\n"
                    "%s diff / %s half diff",
            to_string(modifier.market).c_str(),
            modifier.periodName.c_str(),
            IntegerUtils::toUsdString(pair.buyPrice).c_str(),
            IntegerUtils::toUsdString(buyPrice).c_str(),
            IntegerUtils::toUsdString(pair.sellPrice).c_str(),
            IntegerUtils::toUsdString(sellPrice).c_str(),
            IntegerUtils::toUsdString(mid).c_str(),
            static_cast<unsigned long>(spread.value()),
            static_cast<unsigned long>(additionalSpread.value()),
            IntegerUtils::toUsdString(diff).c_str(),
            IntegerUtils::toUsdString(diffHalf).c_str());
        pair.buyPrice = {};
        return;
    }

    // Apply new buy/sell price
    pair.buyPrice = buyPrice;
    pair.sellPrice = sellPrice;
    setDesc("ramp");
}

void applyNewOrderModifiers(
    const BaseTraderConfig &config,
    utime_t currentTime,
    OrderPair &pair)
{
    // Get modifiers based on market value
    std::vector<MarketPeriodModifier> modifiers = getPeriodModifiersCached(config, currentTime);

    // Apply
    for (const MarketPeriodModifier &modifier : modifiers)
        applyNewOrderModifier(modifier, pair);

    // We need to make sure the sell price isn't passed the max config
    if (pair && pair.sellPrice > config.maxValue)
        pair = OrderPair();

    // Update quantity based on buy parameters
    if (pair)
    {
        pair.quantity = IntegerUtils::getSatoshiForPrice(pair.buyPrice, pair.bet);
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
    utime_t intoRampPeriod;
    if (modifier.period.hot)
    {
        // Are we out of the pause period yet?
        if (modifier.period.pausePeriod < modifier.timeIntoBuffer)
            intoRampPeriod = modifier.timeIntoBuffer - modifier.period.pausePeriod;
        else
            intoRampPeriod = utime_t();
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
    pp_t rampPercent;
    if (modifier.period.rampPeriod)
        rampPercent = safeFractionPp(
            (intoRampPeriod * modifier.period.pauseAcceptLoss).value(),
            modifier.period.rampPeriod.value());
    else
        rampPercent = modifier.period.pauseAcceptLoss;
    // Ramping down losses (ramping up to be active)
    if (modifier.period.hot)
        rampPercent = modifier.period.pauseAcceptLoss - rampPercent;

    // Just double super check we're not going to rollover these unsigned with subtraction
    if (pair.origSellPrice < pair.buyPrice)
        pair.origSellPrice = pair.buyPrice;

    // Calculate the new sell price
    usd_t diff = pair.origSellPrice - pair.buyPrice;
    usd_t less = diff * rampPercent;
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
    usd_t currentBtcPrice,
    utime_t currentTime,
    pp_t spread)
{
    if (!config.enabled)
        return OrderPair();

    // Setup pair
    OrderPair pair = newPair(config, currentTime);

    usd_t spread_cents = currentBtcPrice * spread;
    usd_t half_spread = spread_cents * 50_Percent;
    pair.buyPrice = currentBtcPrice - half_spread;
    pair.sellPrice = currentBtcPrice + half_spread;

    // Apply modifiers
    applyNewOrderModifiers(config, currentTime, pair);

    //Debug spam from here was overwhelming; keep disabled unless re-enabled for deep dives.
#if 0
    if (log::isDebugLoggingEnabled() && !pair.getModifiers().empty())
    {
        log::debug("%s : %s : %s(%u) = [%s - %s]",
            MarketInfo::getTimeString(currentTime).c_str(),
            pair.getModifiers().c_str(),
            IntegerUtils::toUsdString(currentBtcPrice).c_str(),
            static_cast<unsigned int>(spread.value()),
            IntegerUtils::toUsdString(pair.buyPrice).c_str(),
            IntegerUtils::toUsdString(pair.sellPrice).c_str());
    }
#endif

    return pair;
}

OrderPair OrderPairMarketEngine::newStatic(
    const BaseTraderConfig &config,
    utime_t currentTime,
    usd_t buyPrice,
    usd_t sellPrice)
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
    utime_t currentTime)
{
    // This only applies to BTC we're holding and may want to discount
    if (pair.state != OrderPair::State::Holding)
        return;
    // Not applying market parameters
    if (config.marketParams.empty())
        return;

    // Cache the original sell price,
    // Or restore from it
    if (!pair.origSellPrice)
        pair.origSellPrice = pair.sellPrice;
    else
        pair.sellPrice = pair.origSellPrice;

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
    std::vector<MarketPeriodModifier> modifiers = getPeriodModifiersCached(config, currentTime);

    // Apply
    for (const MarketPeriodModifier &modifier : modifiers)
        applySellModifier(modifier, pair);
}
