#include <gtb/PeriodicPrinter.h>
#include <gtb/Log.h>

#include <gtb/BtcPrice.h>
#include <gtb/CoinbaseWallet.h>
#include <gtb/CoinbaseOrderBook.h>

using namespace gtb;

PeriodicPrinter::PeriodicPrinter(
    BotContext &ctx)
        : ctx(ctx)
{
}

void PeriodicPrinter::process(
    const Time &time)
{
    (void)time;

    auto now = std::chrono::steady_clock::now();

    std::lock_guard<std::mutex> lock(mtx);
    if (now >= nextPrint)
    {
        uint32_t cents = ctx.data.get<BtcPrice>().getCents();
        if (cents)
        {
            size_t open = 0;
            auto orders = ctx.data.get<CoinbaseOrderBook>().getOrders();
            for (const auto &[uuid, order] : orders)
                open += order.state == CoinbaseOrder::State::Open;
            size_t closed = orders.size() - open;

            uint32_t usd = ctx.data.get<CoinbaseWallet>().getUsdCents();
            uint64_t btc = ctx.data.get<CoinbaseWallet>().getBtcSatoshi();

            uint32_t btcCents = static_cast<uint32_t>((btc * cents) / 100'000'000);
            uint32_t totalCents = btcCents + usd;

            log::info("STATUS | BTC price: $%u.%02u.", cents / 100, cents % 100);
            log::info("STATUS | %zu open orders. %zu closed orders.", open, closed);
            log::info("STATUS | Wallet: $%u.%02u USD + $%u.%02u BTC = $%u.%02u.",
                usd / 100,
                usd % 100,
                btcCents / 100,
                btcCents % 100,
                totalCents / 100,
                totalCents % 100);

            nextPrint = now + std::chrono::seconds(10);
        }
    }
}
