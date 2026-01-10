#include <gtb/MockSetup.h>

#include <gtb/MockMode.h>
#include <gtb/MockMarket.h>
#include <gtb/MockCoinbase.h>
#include <gtb/MockUserTrades.h>
#include <gtb/MockResultsWriter.h>

#include <gtb/CoinbaseInit.h>
#include <gtb/CoinbaseFeeTier.h>

#include <gtb/OrderPairDb.h>

#include <gtb/PeriodicPrinter.h>

using namespace gtb;

void MockSetup::init(
    TradeBot &bot,
    Config conf)
{
    BotContext &ctx = bot.getCtx();

    ctx.data.initData(MockMode(true));
    SteadyClock::setMockTime(ctx.data.get<Time>());

    unlink("data/mock_trader.sqlite");
    OrderPairDb::setDbFile("data/mock_trader.sqlite");

    // XXX: Use a copy of the historical database
    // so our fast reads dont interrupt the active tradebot by holding a read lock
    std::string cmd = "cp \"" + conf.dataFile + "\" data/mock_historical.sqlite";
    [[maybe_unused]] int x = system(cmd.c_str());
    ctx.historicalDb.init("data/mock_historical.sqlite", "./schema/v2_historical.sql");

    // Mock coinbase API
    ctx.setCoinbase(std::make_unique<MockCoinbase>(
        ctx,
        conf.initHighVolume ? 50_MillionDollars : 0_MillionDollars));

    // Initial state
    ctx.data.get<CoinbaseInit>().setFullInit();
    ctx.data.get<CoinbaseWallet>().update(conf.startWallet, 0_Bitcoins, 0_Dollars, 0_Bitcoins);

    // Source: Historical market data
    bot.addSource(std::make_unique<MockMarket>(ctx, conf.startDate, conf.endDate));

    // Processor: Emulate trades according to current BTC price
    bot.addProcessor(std::make_unique<MockUserTrades>(ctx));

    // Processor: Print periodic status updates
    bot.addProcessor(std::make_unique<PeriodicPrinter>(ctx));

    // Processor: Write periodic results in easily parseable format
    bot.addProcessor(std::make_unique<MockResultsWriter>(ctx, "data/mockresults.jsonl"));
}
