#include <gtb/PeriodicPrinter.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/IntegerUtils.h>
#include <gtb/Log.h>

#include <gtb/Profits.h>
#include <gtb/PendingProfits.h>

#include <gtb/BtcPrice.h>
#include <gtb/CoinbaseWallet.h>
#include <gtb/CoinbaseFeeTier.h>

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
    usd_t total = usd + btcValue;
    unsigned int btcPct = 0.0;
    if (total)
        btcPct = IntegerUtils::fraction(btcValue, total).value();

    log::status("BTC: $%s | Wallet: $%s (%3u%% BTC) | Volume: %s (fee %.2f%%)",
        IntegerUtils::toUsdString(price).c_str(),
        IntegerUtils::toUsdString(total).c_str(),
        btcPct,
        IntegerUtils::toUsdCompact(ctx.coinbase().getVolume()).c_str(),
        static_cast<double>(ctx.data.get<CoinbaseFeeTier>().getFeeTier().value()) / 100.0);

    if (ctx.data.get<MockMode>())
        nextPrint = now + std::chrono::days(1);
    else
        nextPrint = now + std::chrono::seconds(10);
}
