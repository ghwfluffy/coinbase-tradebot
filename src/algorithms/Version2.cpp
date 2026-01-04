#include <gtb/Version2.h>
#include <gtb/Log.h>

#include <gtb/BtcHistoricalWriter.h>
#include <gtb/CoinbaseMarket.h>
#include <gtb/CoinbaseRestClient.h>
#include <gtb/CoinbaseUserInfo.h>
#include <gtb/CoinbaseUserTrades.h>
#include <gtb/PeriodicPrinter.h>
#include <gtb/PeriodicTimeUpdater.h>
#include <gtb/WalletHistoricalWriter.h>

#include <gtb/MockSetup.h>
#include <gtb/TraderConfFactory.h>

using namespace gtb;

namespace
{

void addTraders(
    TradeBot &bot)
{
    auto add = [&bot]<typename Trader>(auto conf) mutable -> void {
        bot.addProcessor(std::make_unique<Trader>(bot.getCtx(), conf()));
    };

    // Add traders
    add.operator()<VolumeTrader>(TraderConfFactory::Volume::stockChurn);
    add.operator()<VolumeTrader>(TraderConfFactory::Volume::pulse);
    add.operator()<VolumeTrader>(TraderConfFactory::Volume::drip);
    add.operator()<VolumeTrader>(TraderConfFactory::Volume::allHoursFeeder);
    add.operator()<VolumeTrader>(TraderConfFactory::Volume::btcFeeder);
    add.operator()<SpreadTrader>(TraderConfFactory::Spread::breakEven);
    add.operator()<SpreadTrader>(TraderConfFactory::Spread::small);
    add.operator()<SpreadTrader>(TraderConfFactory::Spread::medium);
    add.operator()<SpreadTrader>(TraderConfFactory::Spread::large);
    add.operator()<SpreadTrader>(TraderConfFactory::Spread::allHoursProbe);
    add.operator()<TimeTrader>(TraderConfFactory::Time::small);
    add.operator()<TimeTrader>(TraderConfFactory::Time::medium);
    add.operator()<TimeTrader>(TraderConfFactory::Time::large);
}

void initProd(
    TradeBot &bot)
{
    BotContext &ctx = bot.getCtx();

    ctx.historicalDb.init("data/v2_historical.sqlite", "./schema/v2_historical.sql");

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

    // Processor: Print periodic status updates
    bot.addProcessor(std::make_unique<PeriodicPrinter>(ctx));
}

MockSetup::Config mockConf()
{
    return {
        .startDate = "2025-02-01",
        .endDate = "2025-11-25",
        .initHighVolume = false,
        .startWallet = 50'000_Dollars,
    };
}

}

void Version2::init(
    TradeBot &bot,
    bool mock)
{
    // Setup sources, processors, and initial state
    if (mock)
        MockSetup::init(bot, mockConf());
    else
        initProd(bot);

    // Traders
    addTraders(bot);
}
