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

    auto formatPnL = [](const big_usd_t &val) -> std::string {
        int64_t picos = 0;
        if (val.value().tryToInt64(picos))
        {
            bool neg = picos < 0;
            uint64_t absPicos = neg ? static_cast<uint64_t>(-picos) : static_cast<uint64_t>(picos);
            uint64_t dollars = absPicos / static_cast<uint64_t>(usd_t(1_Dollars).value());
            uint64_t cents = (absPicos - (dollars * static_cast<uint64_t>(usd_t(1_Dollars).value()))) /
                static_cast<uint64_t>(usd_t(1_Cents).value());
            char buf[64] = {};
            snprintf(buf, sizeof(buf), "%s$%llu.%02llu",
                neg ? "-" : "",
                static_cast<unsigned long long>(dollars),
                static_cast<unsigned long long>(cents));
            return std::string(buf);
        }
        return IntegerUtils::toUsdCompact(val);
    };

    log::status("BTC: $%s | Wallet: $%s (%3u%% BTC) | Volume: %s (fee %.2f%%)",
        IntegerUtils::toUsdString(price).c_str(),
        IntegerUtils::toUsdString(total).c_str(),
        btcPct,
        IntegerUtils::toUsdCompact(ctx.coinbase().getVolume()).c_str(),
        static_cast<double>(ctx.data.get<CoinbaseFeeTier>().getFeeTier().value()) / 100.0);

    // Per-trader P&L (realized only; excludes unsold BTC)
    auto traderData = ctx.data.get<Profits>().getAllTraderData();
    for (const auto &[trader, data] : traderData)
    {
        log::status("  PnL[%s]: %s vol=%s pending=%s",
            trader.c_str(),
            formatPnL(data.getProfit(price)).c_str(),
            IntegerUtils::toUsdCompact(data.getVolume()).c_str(),
            IntegerUtils::toBtcString(data.getPending()).c_str());
    }

    if (ctx.data.get<MockMode>())
        nextPrint = now + std::chrono::days(1);
    else
        nextPrint = now + std::chrono::seconds(10);
}
