#include <gtb/PeriodicPrinter.h>
#include <gtb/Log.h>

#include <gtb/BtcPrice.h>
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

            log::info("BTC price: $%u.%02u. %zu open orders. %zu closed orders.",
                cents / 100, cents % 100,
                open,
                closed);

            nextPrint = now + std::chrono::seconds(10);
        }
    }
}
