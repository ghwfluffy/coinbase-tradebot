#include <gtb/Version1.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

#include <gtb/OrderPairDb.h>

#include <gtb/PeriodicTimeUpdater.h>
#include <gtb/PeriodicPrinter.h>

#include <gtb/CoinbaseMarket.h>
#include <gtb/CoinbaseRestClient.h>
#include <gtb/CoinbaseUserInfo.h>
#include <gtb/CoinbaseUserTrades.h>

#include <gtb/CoinbaseInit.h>
#include <gtb/CoinbaseFeeTier.h>
#include <gtb/CoinbaseOrderBook.h>

#include <gtb/BtcHistoricalWriter.h>
#include <gtb/WalletHistoricalWriter.h>
#include <gtb/PendingProfitsCalc.h>
#include <gtb/ProfitsReader.h>
#include <gtb/ProfitsWriter.h>

#include <gtb/SpreadTrader.h>
#include <gtb/StaticTrader.h>
#include <gtb/TimeTrader.h>

#include <gtb/MockMode.h>
#include <gtb/MockMarket.h>
#include <gtb/MockCoinbase.h>
#include <gtb/MockUserTrades.h>
#include <gtb/SteadyClock.h>

using namespace gtb;

namespace
{

void initProd(
    TradeBot &bot)
{
    BotContext &ctx = bot.getCtx();

    ctx.historicalDb.init("historical.sqlite", "./schema/historical.sql");

    // Initial state
    ProfitsReader::initProfits(ctx);

    // Real coinbase API
    ctx.setCoinbase(std::make_unique<CoinbaseRestClient>());

    // Source: Time updater
    bot.addSource(std::make_unique<PeriodicTimeUpdater>(ctx));

    // Source: Coinbase market
    bot.addSource(std::make_unique<CoinbaseMarket>(ctx));

    // Source: Coinbase user trades
    bot.addSource(std::make_unique<CoinbaseUserTrades>(ctx));

    // Source: Coinbase user info
    bot.addSource(std::make_unique<CoinbaseUserInfo>(ctx));

    // Processor: Record BTC prices
    bot.addProcessor(std::make_unique<BtcHistoricalWriter>(ctx));

    // Processor: Record wallet value
    bot.addProcessor(std::make_unique<WalletHistoricalWriter>(ctx));

    // Processor: Record profits
    bot.addProcessor(std::make_unique<ProfitsWriter>(ctx));

    // Processor: Print periodic status updates
    bot.addProcessor(std::make_unique<PeriodicPrinter>(ctx));

    // Processor: Pending Profits
    bot.addProcessor(std::make_unique<PendingProfitsCalc>(ctx));
}

void initMock(
    TradeBot &bot)
{
    BotContext &ctx = bot.getCtx();

    ctx.data.initData(MockMode(true));
    SteadyClock::setMockTime(ctx.data.get<Time>());
    unlink("mock_trader.sqlite");
    OrderPairDb::setDbFile("mock_trader.sqlite");
    // XXX: Use a copy of the historical database
    // so our fast reads dont interrupt the active tradebot by holding a read lock
    ctx.historicalDb.init("mock_historical.sqlite", "./schema/historical.sql");

    // Mock coinbase API
    constexpr const uint32_t FEE_TIER = 15;
    ctx.setCoinbase(std::make_unique<MockCoinbase>(ctx, FEE_TIER));

    // Initial state
    ctx.data.get<CoinbaseInit>().setFullInit();
    ctx.data.get<CoinbaseWallet>().update(5000'00, 0, 0, 0);
    ctx.data.get<CoinbaseFeeTier>().setFeeTier(ctx.coinbase().getFeeTier());

    // Source: Historical market data
    bot.addSource(std::make_unique<MockMarket>(ctx));
    //bot.addSource(std::make_unique<MockMarket>(ctx, "2025-01-29", "2025-02-05"));

    // Processor: Emulate trades according to current BTC price
    bot.addProcessor(std::make_unique<MockUserTrades>(ctx));

    // Processor: Print periodic status updates
    bot.addProcessor(std::make_unique<PeriodicPrinter>(ctx));

    // Processor: Pending Profits
    bot.addProcessor(std::make_unique<PendingProfitsCalc>(ctx));
}

MarketPeriodConfig getRampedPeriodConf(bool hot)
{
    return {
        .hot = hot,
        // 1 hour
        .pausePeriod = 2 * 60ull * 60ull * 1'000'000ull,
        .pauseAcceptLoss = 100u,
        .rampPeriod = 2 * 60ull * 60ull * 1'000'000ull,
        .rampGrade = 500,
    };
}

MarketTimeTraderConfig getMarketConf()
{
    return {
        //.market = MarketInfo::Market::StockMarket,
        .market = MarketInfo::Market::BitcoinFutures,

        // Open ----->
        .openMarket = getRampedPeriodConf(true),
        // Open -> Closed
        .closingMarket = getRampedPeriodConf(false),
        // Closed ----->
        .closedMarket = {
            // Paused
            .hot = false,
        },
        // Closed -> Open
        .openingMarket = {
            // Paused
            .hot = false,
        },
        // Open -> Weekend
        .weekendingMarket = getRampedPeriodConf(false),
        // Weekend ---->
        .weekendMarket = {
            // Paused
            .hot = false,
        },
        // Weekend -> Open
        .weekStartingMarket = {
            // Paused
            .hot = false,
        },
    };
}

MarketTimeTraderConfig getStockMarketConf()
{
    MarketTimeTraderConfig ret = getMarketConf();
    ret.market = MarketInfo::Market::StockMarket;
    return ret;
}

}

void Version1::init(
    TradeBot &bot,
    bool mock)
{
    log::info("Initializing Ghw Trade Bot version 1%s.", mock ? " - Mock Test" : "");

    // Setup sources, processors, and initial state
    if (mock)
        initMock(bot);
    else
        initProd(bot);

    BotContext &ctx = bot.getCtx();

    // Trader: Spread
    {
        SpreadTrader::Config conf;
        conf.name = "BreakEven";
        conf.spread = 100; // 1% spread
        conf.cents = 200'00; // $200 bet
        conf.num_pairs = 20;
        conf.buffer_percent = 5;
        conf.maxValue = 110'000'00;
        conf.marketParams.push_back(getMarketConf());
        conf.marketParams.push_back(getStockMarketConf());

        auto trader = std::make_unique<SpreadTrader>(ctx, conf);
        bot.addProcessor(std::move(trader));
    }

    // Trader: 101k -> 105k ($500)
    {
        StaticTrader::Config conf;
        conf.name = "Static4k";
        conf.cents = 500'00;
        conf.buyCents = 101'000'00;
        conf.sellCents = 105'000'00;
        conf.enabled = false;
        conf.marketParams.push_back(getMarketConf());

        auto trader = std::make_unique<StaticTrader>(ctx, conf);
        bot.addProcessor(std::move(trader));
    }

    // Trader: 100k -> 101k ($400)
    {
        StaticTrader::Config conf;
        conf.name = "Static1k";
        conf.cents = 400'00;
        conf.buyCents = 100'000'00;
        conf.sellCents = 101'000'00;
        conf.enabled = false;
        conf.marketParams.push_back(getMarketConf());

        auto trader = std::make_unique<StaticTrader>(ctx, conf);
        bot.addProcessor(std::move(trader));
    }

    // Trader: 0.31% time spread
    {
        TimeTrader::Config conf;
        conf.name = "SmallTime";
        conf.cents = 100'00;
        conf.seconds = 5 * 60;
        conf.minSpread = 31;
        conf.paddingSpread = 1;
        conf.numPairs = 10;
        conf.maxValue = 107'000'00;
        conf.enabled = false;
        conf.marketParams.push_back(getMarketConf());

        auto trader = std::make_unique<TimeTrader>(ctx, conf);
        bot.addProcessor(std::move(trader));
    }
}
