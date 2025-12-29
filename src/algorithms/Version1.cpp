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
#include <gtb/MarketConfFactory.h>

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

    ctx.historicalDb.init("data/historical.sqlite", "./schema/historical.sql");

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
    unlink("data/mock_trader.sqlite");
    OrderPairDb::setDbFile("data/mock_trader.sqlite");
    // XXX: Use a copy of the historical database
    // so our fast reads dont interrupt the active tradebot by holding a read lock
    [[maybe_unused]] int x = system("cp data/historical.sqlite data/mock_historical.sqlite");
    ctx.historicalDb.init("data/mock_historical.sqlite", "./schema/historical.sql");

    // Mock coinbase API (volume-tracked; fee tier derived from rolling volume)
    ctx.setCoinbase(std::make_unique<MockCoinbase>(ctx));

    // Initial state
    ctx.data.get<CoinbaseInit>().setFullInit();
    ctx.data.get<CoinbaseWallet>().update(50'000_Dollars, 0_Bitcoins, 0_Dollars, 0_Bitcoins);
    ctx.data.get<CoinbaseFeeTier>().setFeeTier(ctx.coinbase().getFeeTier());

    // Source: Historical market data
    bot.addSource(std::make_unique<MockMarket>(ctx));
    //bot.addSource(std::make_unique<MockMarket>(ctx, "2025-02-02", "2025-02-15"));
    //bot.addSource(std::make_unique<MockMarket>(ctx, "2025-01-28", "2025-02-15"));
    //bot.addSource(std::make_unique<MockMarket>(ctx, "2025-01-25", "2025-02-15"));

    // Processor: Emulate trades according to current BTC price
    bot.addProcessor(std::make_unique<MockUserTrades>(ctx));

    // Processor: Print periodic status updates
    bot.addProcessor(std::make_unique<PeriodicPrinter>(ctx));

    // Processor: Pending Profits
    bot.addProcessor(std::make_unique<PendingProfitsCalc>(ctx));
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
        conf.spread = 30_PercentagePoints;
        conf.bet = 500_Dollars;
        conf.numPairs = 4;
        conf.buffer = 25_Percent;
        //conf.maxValue = 115'000_Dollars;
        conf.marketParams.push_back(MarketConfFactory::preferNormalHours());

        bot.addProcessor(std::make_unique<SpreadTrader>(ctx, conf));
    }

    // Trader: Spread
    {
        SpreadTrader::Config conf;
        conf.name = "SmallSpread";
        conf.spread = 35_PercentagePoints;
        conf.bet = 500_Dollars;
        conf.numPairs = 10;
        conf.buffer = 10_Percent;
        //conf.maxValue = 115'000_Dollars;
        conf.marketParams.push_back(MarketConfFactory::preferNormalHours());

        bot.addProcessor(std::make_unique<SpreadTrader>(ctx, conf));
    }

    // Trader: Spread
    {
        SpreadTrader::Config conf;
        conf.name = "MediumSpread";
        conf.spread = 50_PercentagePoints;
        conf.bet = 50_Dollars;
        conf.numPairs = 5;
        conf.buffer = 10_Percent;
        //conf.maxValue = 115'000_Dollars;
        conf.marketParams.push_back(MarketConfFactory::preferNormalHours());

        bot.addProcessor(std::make_unique<SpreadTrader>(ctx, conf));
    }

    // Trader: 101k -> 105k ($500)
    {
        StaticTrader::Config conf;
        conf.name = "Static4k";
        conf.bet = 500_Dollars;
        conf.buy = 100'000_Dollars;
        conf.sell = 104'000_Dollars;
        conf.enabled = true;

        bot.addProcessor(std::make_unique<StaticTrader>(ctx, conf));
    }

    // Trader: 100k -> 101k ($400)
    {
        StaticTrader::Config conf;
        conf.name = "Static1k";
        conf.bet = 400_Dollars;
        conf.buy = 101'000_Dollars;
        conf.sell = 102'000_Dollars;
        conf.enabled = true;

        bot.addProcessor(std::make_unique<StaticTrader>(ctx, conf));
    }

    // Trader: 0.5% time spread
    {
        TimeTrader::Config conf;
        conf.name = "SmallTime";
        conf.bet = 500_Dollars;
        conf.sampleSize = 30_Minutes;
        conf.minSpread = 50_PercentagePoints;
        conf.paddingSpread = 1_PercentagePoints;
        conf.numPairs = 10;
        //conf.maxValue = 115'000_Dollars;
        conf.enabled = true;
        conf.marketParams.push_back(MarketConfFactory::preferNormalHours());

        bot.addProcessor(std::make_unique<TimeTrader>(ctx, conf));
    }

    // Trader: 0.5% time spread
    {
        TimeTrader::Config conf;
        conf.name = "MedTime";
        conf.bet = 40_Dollars;
        conf.sampleSize = 50_Minutes;
        conf.minSpread = 1_Percent;
        conf.paddingSpread = 5_PercentagePoints;
        conf.numPairs = 10;
        //conf.maxValue = 115'000_Dollars;
        conf.enabled = true;
        conf.marketParams.push_back(MarketConfFactory::preferNormalHours());

        bot.addProcessor(std::make_unique<TimeTrader>(ctx, conf));
    }

    // Trader: Market immune time spreader
    {
        TimeTrader::Config conf;
        conf.name = "ImmuneTime";
        conf.bet = 50_Dollars;
        conf.sampleSize = 30_Minutes;
        conf.minSpread = 50_PercentagePoints;
        conf.paddingSpread = 100_PercentagePoints;
        conf.numPairs = 100;
        //conf.maxValue = 115'000_Dollars;
        conf.enabled = true;
        //conf.marketParams.push_back(MarketConfFactory::preferNormalHours());

        bot.addProcessor(std::make_unique<TimeTrader>(ctx, conf));
    }

    // Trader: Spread
    {
        SpreadTrader::Config conf;
        conf.name = "ImmuneSpread";
        conf.spread = 50_PercentagePoints;
        conf.bet = 50_Dollars;
        conf.numPairs = 10;
        conf.buffer = 20_Percent;
        //conf.maxValue = 115'000_Dollars;
        //conf.marketParams.push_back(MarketConfFactory::preferNormalHours());

        bot.addProcessor(std::make_unique<SpreadTrader>(ctx, conf));
    }
}
