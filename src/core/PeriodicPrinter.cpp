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

    usd_t price = ctx.data.get<BtcPrice>().getPrice();

    usd_t usd = ctx.data.get<CoinbaseWallet>().getUsd();
    btc_t btc = ctx.data.get<CoinbaseWallet>().getBtc();

    usd_t btcValue = IntegerUtils::getValue(price, btc);

#if 0
    int64_t profit = ctx.data.get<Profits>().getProfit();
    int64_t pending = ctx.data.get<PendingProfits>().getProfit();
#endif

    std::string mockTime;
    if (ctx.data.get<MockMode>())
        mockTime = MarketInfo::getTimeString(time.getTime()) + " | ";
#if 0
    log::info("%sSTATUS | BTC: %s | Wallet: $%s USD + $%s BTC = $%s | SellProfit: $%s | Profit: $%s | Volume: $%s",
        mockTime.c_str(),
        IntegerUtils::toUsdString(price).c_str(),
        IntegerUtils::toUsdString(usd).c_str(),
        IntegerUtils::toUsdString(btcValue).c_str(),
        IntegerUtils::toUsdString(usd + btcValue).c_str(),
        IntegerUtils::toUsdString(pending).c_str(),
        IntegerUtils::toUsdString(profit).c_str(),
        IntegerUtils::toUsdString(ctx.data.get<Profits>().getVolume()).c_str());
#else
    printf("%sSTATUS | BTC: %s | Wallet: $%s USD + $%s BTC = $%s (Volume: $%lu.%02lu)\n",
        mockTime.c_str(),
        IntegerUtils::toUsdString(price).c_str(),
        IntegerUtils::toUsdString(usd).c_str(),
        IntegerUtils::toUsdString(btcValue).c_str(),
        IntegerUtils::toUsdString(usd + btcValue).c_str(),
        ctx.data.get<Profits>().getVolumeCents() / 100,
        ctx.data.get<Profits>().getVolumeCents() % 100);
#endif

    if (ctx.data.get<MockMode>())
        nextPrint = now + std::chrono::days(1);
    else
        nextPrint = now + std::chrono::seconds(10);
}
