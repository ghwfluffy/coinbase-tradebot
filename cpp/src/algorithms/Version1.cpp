#include <gtb/Version1.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

#include <gtb/PeriodicTimeUpdater.h>
#include <gtb/PeriodicPrinter.h>

#include <gtb/CoinbaseMarket.h>
#include <gtb/CoinbaseOrderBook.h>
#include <gtb/CoinbaseRestClient.h>
#include <gtb/CoinbaseUserInfo.h>
#include <gtb/CoinbaseUserTrades.h>

#include <gtb/BtcHistoricalWriter.h>
#include <gtb/WalletHistoricalWriter.h>
#include <gtb/PendingProfitsCalc.h>
#include <gtb/ProfitsReader.h>
#include <gtb/ProfitsWriter.h>

#include <gtb/SpreadTrader.h>
#include <gtb/StaticTrader.h>
#include <gtb/TimeTrader.h>

using namespace gtb;

void Version1::init(TradeBot &bot)
{
    log::info("Initializing Ghw Trade Bot version 1.");

    BotContext &ctx = bot.getCtx();

    ctx.setCoinbase(std::make_unique<CoinbaseRestClient>());

    // Initial state
    ProfitsReader::initProfits(ctx);

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

    // Trader: 0.1% Spread
    {
        SpreadTrader::Config conf;
        conf.name = "BreakEven";
        conf.spread = 10;
        conf.cents = 20'00;
        conf.num_pairs = 4;
        conf.buffer_percent = 25;
        conf.maxValue = 107'000'00;

        auto trader = std::make_unique<SpreadTrader>(ctx, conf);
        bot.addProcessor(std::move(trader));
    }

    // Trader: 103k -> 107k ($1000)
    {
        StaticTrader::Config conf;
        conf.name = "Static4k";
        conf.cents = 500'00;
        conf.buyCents = 103'000'00;
        conf.sellCents = 107'000'00;
        conf.enabled = true;

        auto trader = std::make_unique<StaticTrader>(ctx, conf);
        bot.addProcessor(std::move(trader));
    }

    // Trader: 104k -> 105k ($400)
    {
        StaticTrader::Config conf;
        conf.name = "Static1k";
        conf.cents = 400'00;
        conf.buyCents = 104'000'00;
        conf.sellCents = 105'200'00;
        conf.enabled = true;

        auto trader = std::make_unique<StaticTrader>(ctx, conf);
        bot.addProcessor(std::move(trader));
    }

    // Trader: 0.01% time spread
    {
        TimeTrader::Config conf;
        conf.name = "SmallTime";
        conf.cents = 20'00;
        conf.seconds = 5 * 60;
        conf.minSpread = 5;
        conf.paddingSpread = 1;
        conf.numPairs = 10;
        conf.maxValue = 107'000'00;
        conf.enabled = true;

        auto trader = std::make_unique<TimeTrader>(ctx, conf);
        bot.addProcessor(std::move(trader));
    }
}
