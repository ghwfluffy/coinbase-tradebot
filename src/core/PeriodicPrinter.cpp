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

    std::string mockTime;
    if (ctx.data.get<MockMode>())
        mockTime = MarketInfo::getTimeString(time.getTime()) + " | ";
    int64_t profit = ctx.data.get<Profits>().getProfit().value().toInt64();
    bool profitNeg = profit < 0;
    usd_t profitAbs = usd_t(static_cast<uint64_t>(profitNeg ? -profit : profit));

    usd_t volume = usd_t(ctx.data.get<Profits>().getVolume().value().toUint64());

    printf("%sSTATUS | BTC: %s | Wallet: $%s USD + $%s BTC = $%s | Profit: %s$%s | Volume: %s\n",
        mockTime.c_str(),
        IntegerUtils::toUsdString(price).c_str(),
        IntegerUtils::toUsdString(usd).c_str(),
        IntegerUtils::toUsdString(btcValue).c_str(),
        IntegerUtils::toUsdString(usd + btcValue).c_str(),
        profitNeg ? "-" : "",
        IntegerUtils::toUsdString(profitAbs).c_str(),
        IntegerUtils::toUsdCompact(volume).c_str());

    if (ctx.data.get<MockMode>())
        nextPrint = now + std::chrono::days(1);
    else
        nextPrint = now + std::chrono::seconds(10);
}
