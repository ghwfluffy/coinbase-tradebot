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

    // Trader: 0.5% Spread
    {
        SpreadTrader::Config conf;
        conf.name = "BreakEven";
        conf.spread = 50;
        conf.cents = 100'00;
        conf.num_pairs = 4;
        conf.buffer_percent = 25;
        conf.maxValue = 107'000'00;

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
        conf.enabled = true;

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
        conf.enabled = true;

        auto trader = std::make_unique<StaticTrader>(ctx, conf);
        bot.addProcessor(std::move(trader));
    }

    // Trader: 0.31% time spread
    {
        TimeTrader::Config conf;
        conf.name = "SmallTime";
        conf.cents = 20'00;
        conf.seconds = 5 * 60;
        conf.minSpread = 31;
        conf.paddingSpread = 1;
        conf.numPairs = 10;
        conf.maxValue = 107'000'00;
        conf.enabled = true;

        auto trader = std::make_unique<TimeTrader>(ctx, conf);
        bot.addProcessor(std::move(trader));
    }
}
