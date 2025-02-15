#include <gtb/PeriodicPrinter.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/IntegerUtils.h>
#include <gtb/Log.h>

#include <gtb/Profits.h>
#include <gtb/PendingProfits.h>

#include <gtb/BtcPrice.h>
#include <gtb/CoinbaseWallet.h>

#include <gtb/MockMode.h>
#include <gtb/MarketInfo.h>

using namespace gtb;

PeriodicPrinter::PeriodicPrinter(
    BotContext &ctx)
        : ctx(ctx)
{
    ctx.data.subscribe<Time>(*this);
}

void PeriodicPrinter::process(
    const Time &time)
{
    (void)time;

    auto now = SteadyClock::now();

    std::lock_guard<std::mutex> lock(mtx);
    if (now < nextPrint)
        return;

    // Wait until we're initialized
    if (!ctx.data.get<CoinbaseInit>())
        return;

    uint32_t cents = ctx.data.get<BtcPrice>().getCents();

    uint32_t usd = ctx.data.get<CoinbaseWallet>().getUsdCents();
    uint64_t btc = ctx.data.get<CoinbaseWallet>().getBtcSatoshi();

    uint32_t btcCents = static_cast<uint32_t>((btc * cents) / 100'000'000);
    uint32_t totalCents = btcCents + usd;

    int32_t profit = ctx.data.get<Profits>().getProfit();
    int32_t pending = ctx.data.get<PendingProfits>().getProfit();

    std::string mockTime;
    if (ctx.data.get<MockMode>())
        mockTime = MarketInfo::getTimeString(time.getTime()) + " | ";
    log::info("%sSTATUS | BTC: %u.%02u | Wallet: $%u.%02u USD + $%u.%02u BTC = $%u.%02u | SellProfit: $%s | Profit: $%s",
        mockTime.c_str(),
        cents / 100,
        cents % 100,
        usd / 100,
        usd % 100,
        btcCents / 100,
        btcCents % 100,
        totalCents / 100,
        totalCents % 100,
        IntegerUtils::centsToUsd(profit).c_str(),
        IntegerUtils::centsToUsd(profit + pending).c_str());

    nextPrint = now + std::chrono::seconds(10);
}
