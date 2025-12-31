#include <gtb/WindowTrader.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/IntegerUtils.h>
#include <gtb/CoinbaseOrderBook.h>
#include <gtb/Log.h>

#include <algorithm>
#include <cmath>

using namespace gtb;

WindowTrader::Candle::Candle()
{
    start = SteadyClock::now();
}

namespace
{
    constexpr std::chrono::seconds DEBUG_RATE_LIMIT{10};
}
WindowTrader::WindowTrader(
    BotContext &ctx,
    Config config)
        : ctx(ctx)
        , conf(config)
        , prevActionPrice()
        , prevActionTime()
        , fireSaleUuid()
        , pauseTimer()
        , highPauseTimer()
        , crashUntil()
        , holding()
        , totalSpent()
        , totalPurchased()
        , highWaterPrice()
        , lastBuyPrice()
        , pendingBuyUsd()
        , debugState{usd_t(), usd_t(), btc_t(), SteadyClock::TimePoint()}
{
    ctx.data.subscribe<BtcPrice>(*this);
}

void WindowTrader::process(
    const BtcPrice &price)
{
    // Sane validate
    if (!ctx.data.get<CoinbaseInit>())
        return;
    if (price.getPrice() <= 10'000_Dollars)
        return;

    // Sync logical holding with wallet balances so we never try to trade BTC we do not
    // actually control (including coins on hold by other traders).
    CoinbaseWallet &wallet = ctx.data.get<CoinbaseWallet>();
    btc_t walletBtc = wallet.getBtc();
    btc_t walletAvailBtc = wallet.getAvailBtc();
    if (holding > walletBtc)
        holding = walletBtc;
    if (holding > walletAvailBtc)
        holding = walletAvailBtc;

    // Check/Update candle
    checkCandle(price);

    auto closes = getCloses();

    // Not enough data yet
    if (candles.size() < 2)
        return;

    // Do nothing during a pause period
    requeueSale(price);
    if (isPaused())
        return;

    // If a previous buy order is still open, wait for it to resolve before placing another.
    if (!buyUuid.empty())
    {
        CoinbaseOrder existing = ctx.data.get<CoinbaseOrderBook>().getOrder(buyUuid);
        if (existing && existing.state == CoinbaseOrder::Open)
            return;
        if (!existing || existing.state == CoinbaseOrder::Filled || existing.state == CoinbaseOrder::Canceled)
        {
            buyUuid.clear();
            pendingBuyUsd = usd_t();
        }
    }

    // If we fall below the market fire sale and pause
    if (checkMarketBottom(price))
        return;

    applyExtremeGuards(price, closes);
    if (isPaused())
        return;

    handleBuy(price, closes);
    handleSell(price);
}

void WindowTrader::checkCandle(
    const BtcPrice &price)
{
    // Time for next candle
    if (candles.empty() || (candles.back().start.time + conf.candleSize) <= SteadyClock::now().time)
        candles.push_back(Candle());

    // Update min/max for this candle
    Candle &c = candles.back();
    if (!c.minPrice || c.minPrice > price.getPrice())
        c.minPrice = price.getPrice();

    if (!c.maxPrice || c.maxPrice < price.getPrice())
        c.maxPrice = price.getPrice();
    c.closePrice = price.getPrice();

    // Too many candles
    if (candles.size() > utime_t(conf.windowSize / conf.candleSize).value())
        candles.erase(candles.begin());
}

bool WindowTrader::isPaused() const
{
    return pauseTimer > SteadyClock::now() || crashUntil > SteadyClock::now();
}

bool WindowTrader::isHighPaused() const
{
    return highPauseTimer > SteadyClock::now();
}

bool WindowTrader::fireSale(
    const BtcPrice &price)
{
    btc_t btc = ctx.data.get<CoinbaseWallet>().getAvailBtc();
    if (btc > holding)
        btc = holding;
    if (!btc)
    {
        totalSpent = big_usd_t();
        totalPurchased = big_btc_t();
        holding = btc_t();

        while (candles.size() > 1)
            candles.erase(candles.begin());

        return true;
    }
    // Already fire saling
    if (!fireSaleUuid.empty())
        return true;

    // Try to sell everything we got
    CoinbaseOrder order;
    order.buy = false;
    order.price = IntegerUtils::makerSellPrice(price.getPrice());
    order.quantity = btc;
    order.createdTime = ctx.data.get<Time>().getTime();
    order.trader = conf.name;
    if (ctx.coinbase().submitOrder(order))
    {
        log::trade("Window trader '%s' fire sale %s BTC.",
            conf.name.c_str(),
            IntegerUtils::toBtcString(btc).c_str());
        holding = btc_t();

        fireSaleUuid = order.uuid;
        return true;
    }

    return false;
}

void WindowTrader::pauseTrading()
{
    log::debug("Window trader '%s' pausing trading.", conf.name.c_str());
    pauseTimer = SteadyClock::now() + std::chrono::minutes(conf.pauseDuration / 1_Minutes);
    prevActionPrice = usd_t();
}

void WindowTrader::pauseTrading(
    utime_t duration)
{
    log::debug("Window trader '%s' pausing trading for %llu minutes.",
        conf.name.c_str(),
        static_cast<unsigned long long>(duration / 1_Minutes));
    pauseTimer = SteadyClock::now() + std::chrono::minutes(duration / 1_Minutes);
    prevActionPrice = usd_t();
}

bool WindowTrader::checkMarketBottom(
    const BtcPrice &price)
{
    // We have fallen below the window minimum
    if (price.getPrice() < getFireMin())
    {
        if (fireSale(price))
            pauseTrading();
        return true;
    }

    return false;
}

usd_t WindowTrader::getFireMin() const
{
    usd_t minPrice;
    for (size_t ui = 0; ui < candles.size() / 2; ui++)
    {
        const Candle &c = candles[ui];
        if (!minPrice || minPrice > c.minPrice)
            minPrice = c.minPrice;
    }

    usd_t buffer = getWindowSize() * conf.fireWindowBuffer;
    minPrice -= buffer;
    return minPrice;
}

usd_t WindowTrader::getWindowMin() const
{
    usd_t minPrice;
    for (size_t ui = 0; ui < candles.size() / 2; ui++)
    {
        const Candle &c = candles[ui];
        if (!minPrice || minPrice > c.minPrice)
            minPrice = c.minPrice;
    }

    usd_t buffer = getWindowSize() * conf.lowWindowBuffer;
    minPrice -= buffer;
    return minPrice;
}

usd_t WindowTrader::getWindowMax() const
{
    usd_t maxPrice;
    for (size_t ui = 0; ui < candles.size() / 2; ui++)
    {
        const Candle &c = candles[ui];
        if (!maxPrice || maxPrice < c.maxPrice)
            maxPrice = c.maxPrice;
    }

    usd_t buffer = getWindowSize() * conf.highWindowBuffer;
    maxPrice -= buffer;
    return maxPrice;
}

usd_t WindowTrader::getWindowSize() const
{
    usd_t minPrice;
    usd_t maxPrice;
    for (size_t ui = 0; ui < candles.size() / 2; ui++)
    {
        const Candle &c = candles[ui];
        if (!maxPrice || maxPrice < c.maxPrice)
            maxPrice = c.maxPrice;
        if (!minPrice || minPrice > c.minPrice)
            minPrice = c.minPrice;
    }
    return maxPrice - minPrice;
}

namespace
{
usd_t percentile(std::vector<usd_t> values, uint32_t pct)
{
    if (values.empty())
        return {};
    std::sort(values.begin(), values.end(), [](usd_t a, usd_t b) { return a.value() < b.value(); });
    if (pct >= 100)
        return values.back();
    double span = static_cast<double>(values.size() - 1);
    size_t idx = static_cast<size_t>((span * static_cast<double>(pct)) / 100.0);
    return values[idx];
}

double volatilityRatio(const std::vector<usd_t> &closes)
{
    if (closes.size() < 2)
        return 0.0;
    double sum = 0.0;
    for (usd_t v : closes)
        sum += static_cast<double>(v.value());
    double mean = sum / static_cast<double>(closes.size());
    if (mean <= std::numeric_limits<double>::epsilon())
        return 0.0;

    double var = 0.0;
    for (usd_t v : closes)
    {
        double diff = static_cast<double>(v.value()) - mean;
        var += diff * diff;
    }
    var /= static_cast<double>(closes.size() - 1);
    double stddev = std::sqrt(var);
    return stddev / mean;
}
}

std::vector<usd_t> WindowTrader::getCloses() const
{
    std::vector<usd_t> closes;
    closes.reserve(candles.size());
    for (const auto &c : candles)
    {
        if (c.closePrice)
            closes.push_back(c.closePrice);
    }
    return closes;
}

void WindowTrader::applyExtremeGuards(
    const BtcPrice &price,
    const std::vector<usd_t> &closes)
{
    if (!conf.enableAdaptiveBands || closes.size() < 5)
        return;

    usd_t highGuard = percentile(closes, conf.highPausePercentile);
    usd_t lowGuard = percentile(closes, conf.lowExitPercentile);
    // Simple mean as a sanity floor for crash detection.
    usd_t meanPrice;
    if (!closes.empty())
    {
        BigInt sum;
        for (usd_t v : closes)
            sum += BigInt(v.value());
        BigInt denom(static_cast<uint64_t>(closes.size()));
        BigInt avgBn = sum / denom;
        uint64_t avgVal = avgBn.toUint64();
        meanPrice = usd_t(avgVal);
    }
    auto now = SteadyClock::now();

    if (highGuard && price.getPrice() >= highGuard)
    {
        if (!isHighPaused())
        {
            log::debug("Window trader '%s' pausing new buys: price %s above P%u=%s.",
                conf.name.c_str(),
                IntegerUtils::toUsdString(price.getPrice()).c_str(),
                conf.highPausePercentile,
                IntegerUtils::toUsdString(highGuard).c_str());
        }
        highPauseTimer = now + std::chrono::minutes(conf.extremePauseDuration / 1_Minutes);
    }

    bool deepDrop = false;
    if (meanPrice && conf.crashExitDropPct)
    {
        usd_t dropThresh = meanPrice - (meanPrice * conf.crashExitDropPct);
        deepDrop = price.getPrice() <= dropThresh;
    }

    if (lowGuard && price.getPrice() <= lowGuard && deepDrop)
    {
        log::debug("Window trader '%s' defensive exit: price %s below P%u=%s.",
            conf.name.c_str(),
            IntegerUtils::toUsdString(price.getPrice()).c_str(),
            conf.lowExitPercentile,
            IntegerUtils::toUsdString(lowGuard).c_str());
        if (fireSale(price))
        {
            crashUntil = SteadyClock::now() + std::chrono::minutes(conf.crashCooldown / 1_Minutes);
            inCrash = true;
            pauseTrading(conf.extremePauseDuration);
        }
    }
}

void WindowTrader::handleBuy(
    const BtcPrice &price,
    const std::vector<usd_t> &closes)
{
    usd_t curPrice = price.getPrice();

    if (isHighPaused())
        return;
    if (inCrash && crashUntil > SteadyClock::now())
        return;
    if (inCrash)
    {
        auto closes = getCloses();
        if (!closes.empty())
        {
            usd_t recovery = percentile(closes, conf.crashRecoveryPercentile);
            if (recovery && price.getPrice() >= recovery)
                inCrash = false;
        }
    }

    // Trend guard (block buys in clear down-trend)
    if (conf.trendGuardDelta && closes.size() > conf.trendLookbackCandles)
    {
        usd_t recent = closes.back();
        size_t idx = closes.size() - std::min(conf.trendLookbackCandles, closes.size() - 1);
        usd_t prior = closes[idx];
        if (prior > recent && (prior - recent) >= conf.trendGuardDelta)
            return;
    }

    // Exposure cap
    if (conf.maxExposureUsd)
    {
        usd_t holdingValue = IntegerUtils::getValue(curPrice, holding);
        if (holdingValue >= conf.maxExposureUsd)
            return;
    }
    // Trader-specific capital cap
    if (conf.capitalCap)
    {
        usd_t deployed = IntegerUtils::getValue(curPrice, holding);
        if (deployed >= conf.capitalCap)
            return;
    }

    // Too soon
    if (prevActionTime.time && IntegerUtils::difference(prevActionTime.time, SteadyClock::now().time) < conf.sellFrequency)
        return;

    // Max invested
    if (conf.spendLimit && conf.spendLimit <= IntegerUtils::getValue(price.getPrice(), holding))
        return;

    // Require spacing below last buy to avoid stacking too high.
    if (conf.buySpacingPct && lastBuyPrice)
    {
        usd_t spacingThreshold = lastBuyPrice - (lastBuyPrice * conf.buySpacingPct);
        if (curPrice > spacingThreshold)
            return;
    }

    // Don't increase our buy average
    if (totalSpent && totalPurchased)
    {
        usd_t avgPrice = IntegerUtils::getPrice(totalSpent, totalPurchased);
        if (avgPrice && curPrice > avgPrice + conf.buyDelta)
            return;
        if (avgPrice && conf.buyBelowPct)
        {
            usd_t maxPrice = avgPrice - (avgPrice * conf.buyBelowPct);
            if (curPrice > maxPrice)
                return;
        }
    }

    // Not in the buy window (optionally percentile-based)
    usd_t lower = getWindowMin();
    usd_t upper = getWindowMax();
    usd_t pctLower = lower;
    usd_t pctUpper = upper;
    if (conf.usePercentileBands || conf.enableAdaptiveBands)
    {
        if (!closes.empty())
        {
            pctLower = percentile(closes, conf.lowerPercentile);
            pctUpper = percentile(closes, conf.upperPercentile);
            lower = pctLower;
            upper = pctUpper;
        }
    }
    if (conf.enableAdaptiveBands && closes.size() >= 5)
    {
        usd_t bandLower = percentile(closes, conf.buyBandLowerPercentile);
        usd_t bandUpper = percentile(closes, conf.buyBandUpperPercentile);
        if (bandLower && curPrice < bandLower)
            return; // too low; likely falling knife
        if (bandUpper && curPrice > bandUpper)
            return; // too high; avoid chasing
    }
    if (curPrice < lower || curPrice > upper)
        return;

    // Volatility-aware bet sizing
    usd_t betSize = conf.betSize;
    if (conf.volatilityDampen)
    {
        double vol = volatilityRatio(closes);
        double damp = static_cast<double>(conf.volatilityDampen.value()) / PP_SCALE;
        double scale = 1.0 - (vol * damp);
        double scaleMin = static_cast<double>(conf.minBetScale.value()) / PP_SCALE;
        double scaleMax = static_cast<double>(conf.maxBetScale.value()) / PP_SCALE;
        if (scale < scaleMin) scale = scaleMin;
        if (scale > scaleMax) scale = scaleMax;
        betSize = usd_t(static_cast<uint64_t>(static_cast<double>(conf.betSize.value()) * scale));
        if (!betSize)
            betSize = usd_t(1);
        auto now = SteadyClock::now();
        if (now >= debugState.lastLogTime + DEBUG_RATE_LIMIT)
        {
            log::debug("Window trader '%s' vol=%.4f scale=%.2f bet=%s lower=%s upper=%s pctL=%s pctU=%s",
                conf.name.c_str(),
                vol,
                scale,
                IntegerUtils::toUsdString(betSize).c_str(),
                IntegerUtils::toUsdString(lower).c_str(),
                IntegerUtils::toUsdString(upper).c_str(),
                IntegerUtils::toUsdString(pctLower).c_str(),
                IntegerUtils::toUsdString(pctUpper).c_str());
            debugState.lastLogTime = now;
        }
    }

    // Place another bet
    CoinbaseOrder order;
    order.buy = true;
    order.price = IntegerUtils::makerBuyPrice(curPrice);
    order.quantity = IntegerUtils::getSatoshiForPrice(curPrice, betSize);
    order.createdTime = ctx.data.get<Time>().getTime();
    order.trader = conf.name;

    // Enough money?
    usd_t wallet = ctx.data.get<CoinbaseWallet>().getAvailUsd();
    if (order.value() > wallet - 20_Dollars)
        return;

    btc_t prevHolding = holding;
    // Respect trader capital cap by resizing the order if needed.
    if (conf.capitalCap)
    {
        usd_t deployed = IntegerUtils::getValue(curPrice, holding) + pendingBuyUsd;
        usd_t room = (conf.capitalCap > deployed) ? (conf.capitalCap - deployed) : usd_t();
        if (room && order.value() > room)
        {
            order.quantity = IntegerUtils::getSatoshiForPrice(curPrice, room);
            if (!order.quantity)
                return;
        }
        else if (!room)
        {
            return;
        }
    }

    // Re-read available USD just before submit to avoid racing other traders.
    usd_t availNow = ctx.data.get<CoinbaseWallet>().getAvailUsd();
    if (order.value() > availNow)
        return;

    if (ctx.coinbase().submitOrder(order))
    {
        buyUuid = order.uuid;
        pendingBuyUsd = order.value();
        setAction(price);
        holding += order.quantity;

        totalSpent += order.value();
        totalPurchased += order.quantity;
        lastBuyPrice = curPrice;
        if (!prevHolding || curPrice > highWaterPrice)
            highWaterPrice = curPrice;

        log::trade("Window trader '%s' BUY %s BTC @ %s (value %s).",
            conf.name.c_str(),
            IntegerUtils::toBtcString(order.quantity).c_str(),
            IntegerUtils::toUsdString(order.price).c_str(),
            IntegerUtils::toUsdString(order.value()).c_str());
    }
}

void WindowTrader::setAction(
    const BtcPrice &price)
{
    prevActionTime = SteadyClock::now();
    prevActionPrice = price.getPrice();
}

void WindowTrader::handleSell(
    const BtcPrice &price)
{
    if (holding && (!highWaterPrice || price.getPrice() > highWaterPrice))
        highWaterPrice = price.getPrice();

    // Too soon
    if (prevActionTime.time && IntegerUtils::difference(prevActionTime.time, SteadyClock::now().time) < conf.sellFrequency)
        return;

    // Nothing to sell
    if (!holding)
        return;

    if (!totalSpent)
        return;
    if (!totalPurchased)
        return;

    usd_t avgPrice = IntegerUtils::getPrice(totalSpent, totalPurchased);
    if (!avgPrice)
        return;

    // Soft defensive exit if we drift below the safe band
    if (conf.enableAdaptiveBands && conf.softExitPercentile > 0)
    {
        auto closes = getCloses();
        if (closes.size() >= 5)
        {
            usd_t softGuard = percentile(closes, conf.softExitPercentile);
            if (softGuard && price.getPrice() <= softGuard)
            {
                btc_t amount = holding * conf.softExitSellRatio;
                if (!amount || amount > holding)
                    amount = holding;

                btc_t have = ctx.data.get<CoinbaseWallet>().getAvailBtc();
                if (!have)
                    return;
                if (amount > have)
                    amount = have;
                if (!amount)
                    return;

                CoinbaseOrder order;
                order.buy = false;
                order.price = IntegerUtils::makerSellPrice(price.getPrice());
                order.quantity = amount;
                order.createdTime = ctx.data.get<Time>().getTime();
                order.trader = conf.name;
                btc_t haveNow = ctx.data.get<CoinbaseWallet>().getAvailBtc();
                if (order.quantity > haveNow)
                    return;
                if (ctx.coinbase().submitOrder(order))
                {
                    sellUuid = order.uuid;
                    log::debug("Window trader '%s' soft exit %s BTC @ %s (softGuard=%s).",
                        conf.name.c_str(),
                        IntegerUtils::toBtcString(order.quantity).c_str(),
                        IntegerUtils::toUsdString(order.price).c_str(),
                        IntegerUtils::toUsdString(softGuard).c_str());
                    setAction(price);
                    holding -= amount;
                    totalSpent += usd_t(order.value().value() / 2);
                    totalPurchased += btc_t(order.quantity.value() / 2);
                    if (!holding)
                        highWaterPrice = usd_t();
                }
                return;
            }
        }
    }

    // Stop-loss relative to average cost: shed a chunk if price sags below avg.
    if (conf.stopLossPct)
    {
        usd_t stopThreshold = avgPrice - (avgPrice * conf.stopLossPct);
        if (price.getPrice() <= stopThreshold)
        {
            if (!sellUuid.empty())
            {
                CoinbaseOrder existing = ctx.data.get<CoinbaseOrderBook>().getOrder(sellUuid);
                if (existing && existing.state == CoinbaseOrder::Open)
                    return;
                if (!existing || existing.state == CoinbaseOrder::Filled || existing.state == CoinbaseOrder::Canceled)
                    sellUuid.clear();
            }

            btc_t have = ctx.data.get<CoinbaseWallet>().getAvailBtc();
            if (!have || !holding)
                return;
            btc_t amount = holding * conf.partialSellRatio;
            if (!amount || amount > holding)
                amount = holding;
            if (amount > have)
                amount = have;
            if (!amount)
                return;

            CoinbaseOrder order;
            order.buy = false;
            order.price = IntegerUtils::makerSellPrice(price.getPrice());
            order.quantity = amount;
            order.createdTime = ctx.data.get<Time>().getTime();
            order.trader = conf.name;
            btc_t haveNow = ctx.data.get<CoinbaseWallet>().getAvailBtc();
            if (order.quantity > haveNow)
                return;
            if (ctx.coinbase().submitOrder(order))
            {
                sellUuid = order.uuid;
                setAction(price);
                holding -= amount;
                log::debug("Window trader '%s' stop-loss sell %s BTC @ %s (stop=%s avg=%s).",
                    conf.name.c_str(),
                    IntegerUtils::toBtcString(order.quantity).c_str(),
                    IntegerUtils::toUsdString(order.price).c_str(),
                    IntegerUtils::toUsdString(stopThreshold).c_str(),
                    IntegerUtils::toUsdString(avgPrice).c_str());
                totalSpent += usd_t(order.value().value() / 2);
                totalPurchased += btc_t(order.quantity.value() / 2);
                if (!holding)
                    highWaterPrice = usd_t();
            }
            return;
        }
    }

    // Require profit after estimated fees
    pp_t feeRate = ctx.coinbase().getFeeTier();
    pp_t roundTripFee = feeRate + feeRate;
    usd_t feeBuffer = avgPrice * roundTripFee;

    usd_t target = avgPrice + feeBuffer + conf.takeProfitDelta + (avgPrice * conf.takeProfitVolBoost);
    if (conf.takeProfitPct)
    {
        usd_t pctTarget = avgPrice + (avgPrice * conf.takeProfitPct);
        if (pctTarget > target)
            target = pctTarget;
    }

    bool trailingTrigger = false;
    usd_t trailingLimit;
    if (conf.trailingDrop && highWaterPrice)
    {
        trailingLimit = highWaterPrice - (highWaterPrice * conf.trailingDrop);
        if (price.getPrice() <= trailingLimit && price.getPrice() > avgPrice)
            trailingTrigger = true;
    }

    if (!trailingTrigger && price.getPrice() < target)
        return;

    btc_t amount = IntegerUtils::getSatoshiForPrice(avgPrice, conf.betSize);
    if (conf.partialSellRatio.value() != PP_SCALE)
        amount = holding * conf.partialSellRatio;
    if (amount > holding || !amount)
        amount = holding;
    // Debug log on change or rate limit
    auto now = SteadyClock::now();
    if ((price.getPrice() != debugState.lastSellPrice ||
         target != debugState.lastSellTarget ||
         amount != debugState.lastSellAmount) &&
        now >= debugState.lastLogTime + DEBUG_RATE_LIMIT)
    {
        log::debug("Window trader '%s' sellCheck price=%s avg=%s feeBuf=%s target=%s highWater=%s trailStop=%s holding=%s sellAmt=%s",
            conf.name.c_str(),
            IntegerUtils::toUsdString(price.getPrice()).c_str(),
            IntegerUtils::toUsdString(avgPrice).c_str(),
            IntegerUtils::toUsdString(feeBuffer).c_str(),
            IntegerUtils::toUsdString(target).c_str(),
            IntegerUtils::toUsdString(highWaterPrice).c_str(),
            IntegerUtils::toUsdString(trailingLimit).c_str(),
            IntegerUtils::toBtcString(holding).c_str(),
            IntegerUtils::toBtcString(amount).c_str());
        debugState.lastSellPrice = price.getPrice();
        debugState.lastSellTarget = target;
        debugState.lastSellAmount = amount;
        debugState.lastLogTime = now;
    }

    // Only sell what this trader currently holds and what is available
    // Avoid overlapping sells while one is open.
    if (!sellUuid.empty())
    {
        CoinbaseOrder existing = ctx.data.get<CoinbaseOrderBook>().getOrder(sellUuid);
        if (existing && existing.state == CoinbaseOrder::Open)
            return;
        if (!existing || existing.state == CoinbaseOrder::Filled || existing.state == CoinbaseOrder::Canceled)
            sellUuid.clear();
    }

    btc_t have = ctx.data.get<CoinbaseWallet>().getAvailBtc();
    if (!have || !holding)
        return;

    if (amount > holding)
        amount = holding;
    if (amount > have)
        amount = have;
    if (!amount)
        return;

    CoinbaseOrder order;
    order.buy = false;
    order.price = IntegerUtils::makerSellPrice(price.getPrice());
    order.quantity = amount;
    order.createdTime = ctx.data.get<Time>().getTime();
    order.trader = conf.name;
    // Final availability check just before submit to avoid racing other traders.
    btc_t haveNow = ctx.data.get<CoinbaseWallet>().getAvailBtc();
    if (order.quantity > haveNow)
        return;

    if (ctx.coinbase().submitOrder(order))
    {
        sellUuid = order.uuid;
        setAction(price);
        holding -= amount;
        log::trade("Window trader '%s' SELL %s BTC @ %s (value %s).",
            conf.name.c_str(),
            IntegerUtils::toBtcString(order.quantity).c_str(),
            IntegerUtils::toUsdString(order.price).c_str(),
            IntegerUtils::toUsdString(order.value()).c_str());

        totalSpent += usd_t(order.value().value() / 2);
        totalPurchased += btc_t(order.quantity.value() / 2);
        if (!holding)
            highWaterPrice = usd_t();
    }
}

void WindowTrader::requeueSale(
    const BtcPrice &price)
{
    if (fireSaleUuid.empty())
        return;

    CoinbaseOrder order = ctx.data.get<CoinbaseOrderBook>().getOrder(fireSaleUuid);
    if (!order || order.state == CoinbaseOrder::Filled)
    {
        log::trade("Window trader '%s' fire sale complete.", conf.name.c_str());
        fireSaleUuid.clear();

        totalSpent = big_usd_t();
        totalPurchased = big_btc_t();

        while (candles.size() > 1)
            candles.erase(candles.begin());
        return;
    }

    // This order is still good
    if (order.state == CoinbaseOrder::Open && order.price < IntegerUtils::makerSellPrice(price.getPrice()))
        return;

    // Cancel open order
    if (order.state == CoinbaseOrder::Open && !ctx.coinbase().cancelOrder(fireSaleUuid))
        return;

    // Add back to holding
    holding += order.quantity;
    // Requeue
    fireSaleUuid.clear();
    if (isPaused())
        fireSale(price);
}
