#pragma once

#include <gtb/BotContext.h>

#include <gtb/BtcPrice.h>
#include <gtb/SteadyClock.h>
#include <gtb/IntLiterals.h>
#include <gtb/BigInt.h>

#include <set>
#include <mutex>

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
        void pauseTrading();

        bool checkMarketBottom(
            const BtcPrice &price);
        bool fireSale(
            const BtcPrice &price);
        void requeueSale(
            const BtcPrice &price);

        void handleBuy(
            const BtcPrice &price);
        void handleSell(
            const BtcPrice &price);

        usd_t getWindowMax() const;
        usd_t getWindowMin() const;
        usd_t getWindowSize() const;
        usd_t getFireMin() const;

        void setAction(
            const BtcPrice &price);

        BotContext &ctx;
        Config conf;

        struct Candle
        {
            SteadyClock::TimePoint start;

            usd_t totalSpent;
            btc_t totalPurchased;

            usd_t minPrice;
            usd_t maxPrice;

            Candle();
        };

        std::mutex mtx;
        std::vector<Candle> candles;

        usd_t prevActionPrice;
        SteadyClock::TimePoint prevActionTime;

        std::string fireSaleUuid;
        SteadyClock::TimePoint pauseTimer;

        btc_t holding;
        BigInt totalSpent;
        BigInt totalPurchased;
};

}
