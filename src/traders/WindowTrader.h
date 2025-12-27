#pragma once

#include <gtb/BotContext.h>

#include <gtb/BtcPrice.h>
#include <gtb/SteadyClock.h>
#include <gtb/IntLiterals.h>
#include <gtb/StrongTypedBigInt.h>

#include <set>
#include <mutex>
#include <vector>

namespace gtb
{

/**
 * Window-based mean-reversion trader.
 *
 * Strategy overview:
 *  - Maintains a rolling price window of size Config::windowSize,
 *    divided into candles of duration Config::candleSize.
 *  - Uses the *older half* of the window to estimate a trading range:
 *      - Upper bound derived from the maximum price in that half,
 *        reduced by Config::highWindowBuffer (no new buys near this level).
 *      - Lower bound derived from the minimum price in that half,
 *        adjusted by Config::lowWindowBuffer. Falling below this
 *        triggers a fire sale and enters a pause state for
 *        Config::pauseDuration (no new buys during this period).
 *
 * Buying:
 *  - Trades in fixed notional increments of Config::betSize (USD).
 *  - Opens new buys only when the price has moved sufficiently away
 *    from the last fill (buy or sell) (Config::buyDelta) and total
 *    spend does not exceed Config::spendLimit (0 == no limit).
 *  - Per candle, accumulates:
 *      - TotalBoughtQty  (satoshis)
 *      - TotalBoughtCost (USD)
 *
 * Selling:
 *  - At most once every Config::sellFrequency, evaluates for profit-taking.
 *  - Computes an average buy price over the last
 *    Config::avgBuyPriceCandleCount candles
 *  - If the current price exceeds this average by at least
 *    Config::takeProfitDelta, sells Config::betSize (USD) worth
 *    of position.
 *
 * Additional controls:
 *  - buyDelta: minimum price move from last action before a new buy.
 *  - sellFrequency: minimum time between sells.
 *  - spendLimit / maxExposureUsd: cap notional deployed (0 = no cap).
 *  - pauseDuration: cooldown after a fire sale; no buys during pause.
 *  - fireWindowBuffer: triggers a fire sale when price drops below the window
 *    floor by this buffer (and then pauses).
 *  - highWindowBuffer / lowWindowBuffer: tighten the upper/lower bounds of the
 *    window to avoid buying near extremes.
 *  - adaptive bands: pause new buys above a rolling high percentile and trigger
 *    a defensive exit + pause below a low percentile.
 *  - percentile bands: optional lower/upper bands using historical closes
 *    (e.g., 20th/80th percentile) instead of raw min/max.
 *  - trend guard: optional downward-trend blocker for buys.
 *  - volatility dampening: optional bet sizing scale based on price volatility.
 *  - partial exits: optional fraction of holdings to sell per take-profit.
 */
class WindowTrader
{
    public:
        struct Config
        {
            std::string name;
            // How big of a window to track
            utime_t windowSize = 48_Hours;
            // Don't buy within this amount of the upper part of the window
            pp_t highWindowBuffer = 15_Percent;
            // Don't buy within this amount on the lower part of the window
            pp_t lowWindowBuffer = 10_Percent;
            // Fire sale and trigger pause if we fall this far below the window
            pp_t fireWindowBuffer = 1_Percent;
            // Duration of pause after a fire sale
            utime_t pauseDuration = 24_Hours;
            // How small to divide each candle
            utime_t candleSize = 5_Minutes;
            // Max frequency of sales
            utime_t sellFrequency = 5_Seconds;
            // Amount to buy/sell at a time
            usd_t betSize = 20_Dollars;
            // Max to invest at once (0 = no limit)
            usd_t spendLimit = 0_Dollars;
            // When calculating the average buy price,
            // use this many previous candles
            size_t avgBuyPriceCandleCount = 2;
            // Required profit over average buy price to trigger a sale
            usd_t takeProfitDelta = 2_Dollars;
            // Required profit over average buy price to trigger a sale
            usd_t buyDelta = 20_Dollars;

            // Percentile bands (disabled by default)
            bool usePercentileBands = false;
            uint32_t lowerPercentile = 20; // 20th percentile
            uint32_t upperPercentile = 80; // 80th percentile

            // Trend guard: block buys if recent trend is down more than this
            usd_t trendGuardDelta = 0_Dollars; // 0 = disabled
            size_t trendLookbackCandles = 2;

            // Volatility dampening (scale bet size when volatile). 0 = disabled.
            pp_t volatilityDampen = 0_Percent;
            pp_t minBetScale = 50_Percent;
            pp_t maxBetScale = 100_Percent;

            // Adaptive band trading: use recent percentiles to define a safe zone.
            bool enableAdaptiveBands = true;
            uint32_t highPausePercentile = 95; // pause new buys above this percentile
            uint32_t lowExitPercentile = 2;    // fire-sale + pause below this percentile
            uint32_t buyBandLowerPercentile = 25;
            uint32_t buyBandUpperPercentile = 70;
            uint32_t softExitPercentile = 10;    // reduce exposure below this percentile
            pp_t softExitSellRatio = 25_Percent; // portion to shed on soft exit
            utime_t extremePauseDuration = 4_Hours;
            // Crash handling: stay paused after a crash until recovery.
            utime_t crashCooldown = 6_Hours;
            uint32_t crashRecoveryPercentile = 50;
            pp_t crashExitDropPct = 2_Percent; // also require this drop from mean before fire-sale

            // Dynamic take-profit boost based on volatility (as % of avg price).
            pp_t takeProfitVolBoost = 0_Percent;
            // Trailing drop from high water to lock gains (0 = disabled).
            pp_t trailingDrop = 0_Percent;

            // Require buys to be spaced by this percent below last buy (0 = disabled).
            pp_t buySpacingPct = 0_Percent;

            // Exposure cap (0 = no cap)
            usd_t maxExposureUsd = 0_Dollars;
            // Capital allocation cap for this trader (0 = unlimited). Lets multiple
            // traders share a wallet without stomping each other's funds.
            usd_t capitalCap = 0_Dollars;

            // Partial exit ratio (percentage of holding to sell per take-profit)
            pp_t partialSellRatio = 100_Percent;
        };

        WindowTrader(
            BotContext &ctx,
            Config config);
        WindowTrader(WindowTrader &&) = delete;
        WindowTrader(const WindowTrader &) = delete;
        WindowTrader &operator=(WindowTrader &&) = delete;
        WindowTrader &operator=(const WindowTrader &) = delete;
        ~WindowTrader() = default;

        void process(
            const BtcPrice &price);

    private:
        void checkCandle(
            const BtcPrice &price);

        bool isPaused() const;
        bool isHighPaused() const;
        void pauseTrading();
        void pauseTrading(
            utime_t duration);

        bool checkMarketBottom(
            const BtcPrice &price);
        bool fireSale(
            const BtcPrice &price);
        void requeueSale(
            const BtcPrice &price);

        void handleBuy(
            const BtcPrice &price,
            const std::vector<usd_t> &closes);
        void handleSell(
            const BtcPrice &price);

        usd_t getWindowMax() const;
        usd_t getWindowMin() const;
        usd_t getWindowSize() const;
        usd_t getFireMin() const;
        std::vector<usd_t> getCloses() const;
        void applyExtremeGuards(
            const BtcPrice &price,
            const std::vector<usd_t> &closes);

        void setAction(
            const BtcPrice &price);

        BotContext &ctx;
        Config conf;

        struct Candle
        {
            SteadyClock::TimePoint start;

            usd_t minPrice;
            usd_t maxPrice;
            usd_t closePrice;

            Candle();
        };

        std::mutex mtx;
        std::vector<Candle> candles;

        usd_t prevActionPrice;
        SteadyClock::TimePoint prevActionTime;

        std::string fireSaleUuid;
        SteadyClock::TimePoint pauseTimer;
        SteadyClock::TimePoint highPauseTimer;
        SteadyClock::TimePoint crashUntil;
        bool inCrash = false;

        btc_t holding;
        big_usd_t totalSpent;
        big_btc_t totalPurchased;
        usd_t highWaterPrice;
        usd_t lastBuyPrice;

        // Debug log throttling
        struct DebugState
        {
            usd_t lastSellPrice;
            usd_t lastSellTarget;
            btc_t lastSellAmount;
            SteadyClock::TimePoint lastLogTime;
        } debugState;
};

}
