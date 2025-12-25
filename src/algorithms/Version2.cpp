#include <gtb/Version2.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

#include <gtb/OrderPairDb.h>

#include <gtb/PeriodicPrinter.h>

#include <gtb/CoinbaseInit.h>
#include <gtb/CoinbaseFeeTier.h>

#include <gtb/PendingProfitsCalc.h>

#include <gtb/SpreadTrader.h>
#include <gtb/ConstantTrader.h>
#include <gtb/ConstantSpreadTrader.h>

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
    (void)bot;

    log::error("v2 production not hooked up");
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
    [[maybe_unused]] int x = system("cp historical.sqlite mock_historical.sqlite");
    ctx.historicalDb.init("mock_historical.sqlite", "./schema/historical.sql");

    // Mock coinbase API
    constexpr const pp_t FEE_TIER = 0_Percent;
    ctx.setCoinbase(std::make_unique<MockCoinbase>(ctx, FEE_TIER));

    // Initial state
    ctx.data.get<CoinbaseInit>().setFullInit();
    ctx.data.get<CoinbaseWallet>().update(20'000_Dollars, 0_Bitcoins, 0_Dollars, 0_Bitcoins);
    ctx.data.get<CoinbaseFeeTier>().setFeeTier(ctx.coinbase().getFeeTier());

    // Source: Historical market data
    std::vector<std::string> range = {
        //"2025-05-01", "",//"2025-06-30",
        "2025-01-25", "2025-02-01",
        "", ""
    };
    bot.addSource(std::make_unique<MockMarket>(ctx, range[0], range[1]));

    // Processor: Emulate trades according to current BTC price
    bot.addProcessor(std::make_unique<MockUserTrades>(ctx));

    // Processor: Print periodic status updates
    bot.addProcessor(std::make_unique<PeriodicPrinter>(ctx));

    // Processor: Pending Profits
    // TODO: Make the tracked order pair class keep track of this
    //bot.addProcessor(std::make_unique<PendingProfitsCalc>(ctx));
}

#if 0
MarketPeriodConfig getRampedPeriodConf(bool hot)
{
    return {
        .hot = hot,
        .pausePeriod = 2_Hours,
        .pauseAcceptLoss = (hot ? 10_PercentagePoints : 100_Percent),
        .rampPeriod = 2_Hours,
        .rampGrade = 200_Percent,
    };
}

MarketPeriodConfig getPausedPeriodConf()
{
    return {
        .hot = false,
        .pausePeriod = 0_Seconds,
        .pauseAcceptLoss = 0_Percent,
        .rampPeriod = 0_Seconds,
        .rampGrade = 0_Percent,
    };
}

MarketTimeTraderConfig getMarketConf()
{
    return {
        .market = MarketInfo::Market::BitcoinFutures,

        // Open ----->
        .openMarket = getRampedPeriodConf(true),
        // Open -> Closed
        .closingMarket = getPausedPeriodConf(),
        // Closed ----->
        .closedMarket = getPausedPeriodConf(),
        // Closed -> Open
        .openingMarket = getRampedPeriodConf(false),
        // Open -> Weekend
        .weekendingMarket = getPausedPeriodConf(),
        // Weekend ---->
        .weekendMarket = getPausedPeriodConf(),
        // Weekend -> Open
        .weekStartingMarket = getRampedPeriodConf(true),
    };
}
#endif

}

void Version2::init(
    TradeBot &bot,
    bool mock)
{
    log::info("Initializing Ghw Trade Bot version 2%s.", mock ? " - Mock Test" : "");

    // Setup sources, processors, and initial state
    if (mock)
        initMock(bot);
    else
        initProd(bot);

    BotContext &ctx = bot.getCtx();
#if 0
    bot.addProcessor(std::make_unique<ConstantTrader>(ctx, 500_Dollars, 2_Dollars));
    //bot.addProcessor(std::make_unique<ConstantTrader>(ctx, 50_Dollars, 20_Dollars));
    //bot.addProcessor(std::make_unique<ConstantTrader>(ctx, 100_Dollars, 200_Dollars));
#endif
#if 0
    // Trader: Spread
    {
        SpreadTrader::Config conf;
        conf.name = "Spread";
        conf.spread = 20_PercentagePoints;
        conf.bet = 200_Dollars;
        //conf.numPairs = 1;
        conf.numPairs = 30'000 / 200;
        conf.buffer = 20_PercentagePoints;
        conf.maxValue = 110'000_Dollars;
        //conf.marketParams.push_back(getMarketConf());

        bot.addProcessor(std::make_unique<SpreadTrader>(ctx, conf));
    }
#endif
    // Trader: Const Spread
    {
        ConstantSpreadTrader::Config conf;
        conf.name = "ConstSpread";
        conf.windowSize = 2_Dollars;
        conf.bet = 20_Dollars;
        //conf.buffer = 20_PercentagePoints; // TODO: N/A
        conf.maxValue = 110'000_Dollars;
        conf.pendingPairExpiration = 1_Hours;
        //conf.marketParams.push_back(getMarketConf());

        bot.addProcessor(std::make_unique<ConstantSpreadTrader>(ctx, conf));
    }
}
