#include <gtb/WalletHistoricalWriter.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/IntegerUtils.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

#include <sstream>

using namespace gtb;

namespace
{

// 5 minutes
constexpr const uint64_t PERIODIC_FREQUENCY = 5 * 60 * 1'000'000;

}

WalletHistoricalWriter::WalletHistoricalWriter(
    BotContext &ctx)
        : ctx(ctx)
{
    prevTime = 0;
    ctx.data.subscribe<BtcPrice>(*this);
    ctx.data.subscribe<CoinbaseWallet>(*this);
}

void WalletHistoricalWriter::process(
    const BtcPrice &price)
{
    (void)price;

    uint64_t curTime = ctx.data.get<Time>().getTime();

    std::lock_guard<std::mutex> lock(mtx);
    if (curTime < prevTime + PERIODIC_FREQUENCY)
        return;

    write(lock);
}

void WalletHistoricalWriter::process(
    const CoinbaseWallet &wallet)
{
    (void)wallet;

    std::lock_guard<std::mutex> lock(mtx);
    write(lock);
}

void WalletHistoricalWriter::write(
    const std::lock_guard<std::mutex> &lock)
{
    (void)lock;

    if (!ctx.data.get<CoinbaseInit>())
        return;

    uint64_t curTime = ctx.data.get<Time>().getTime();
    if (curTime == prevTime)
        return;

    prevTime = curTime;

    const CoinbaseWallet &wallet = ctx.data.get<CoinbaseWallet>();
    uint32_t btcValue = ctx.data.get<BtcPrice>().getCents();
    uint32_t totalValue = wallet.getUsdCents();
    totalValue += IntegerUtils::getValueCents(btcValue, wallet.getBtcSatoshi());

    std::ostringstream query;
    query << "INSERT INTO wallet (time, usd, btc, value) VALUES ("
        << curTime << ","
        << wallet.getUsdCents() << ","
        << wallet.getBtcSatoshi() << ","
        << totalValue << ")";
    if (!ctx.historicalDb.getConn().execute(query.str()))
        log::error("Failed to record wallet data.");
}
