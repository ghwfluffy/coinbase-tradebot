#include <gtb/Version1.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

#include <gtb/PeriodicTimeUpdater.h>
#include <gtb/PeriodicPrinter.h>

#include <gtb/CoinbaseMarket.h>
#include <gtb/CoinbaseUserTrades.h>
#include <gtb/CoinbaseUserInfo.h>

#include <gtb/BtcHistoricalWriter.h>

using namespace gtb;

void Version1::init(TradeBot &bot)
{
    log::info("Initializing Ghw Trade Bot version 1.");

    BotContext &ctx = bot.getCtx();

    // Database schema
    ctx.algoDb.init("version1.sqlite", "./schema/version1.sql");

    // Initial state
    ctx.data.get<Time>().setTime(1337);
    ctx.data.get<BtcPrice>();

    // Source: Time updater
    {
        auto source = std::make_unique<PeriodicTimeUpdater>(ctx);
        bot.addSource(std::move(source));
    }

    // Source: Coinbase market
    {
        auto source = std::make_unique<CoinbaseMarket>(ctx);
        bot.addSource(std::move(source));
    }

    // Source: Coinbase user trades
    {
        auto source = std::make_unique<CoinbaseUserTrades>(ctx);
        bot.addSource(std::move(source));
    }

    // Source: Coinbase user info
    {
        auto info = std::make_unique<CoinbaseUserInfo>(ctx);
        // Reload user info on orderbook changes
        ctx.data.subscribe<CoinbaseOrderBook>(*info);
        bot.addSource(std::move(info));
    }

    // Processor: Record BTC prices
    {
        auto proc = std::make_unique<BtcHistoricalWriter>(ctx);
        ctx.data.subscribe<BtcPrice>(*proc);
        bot.addProcessor(std::move(proc));
    }

    // Processor: Printer
    {
        auto printer = std::make_unique<PeriodicPrinter>(ctx);
        ctx.data.subscribe<Time>(*printer);
        bot.addProcessor(std::move(printer));
    }
}
