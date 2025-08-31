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
constexpr const utime_t PERIODIC_FREQUENCY = 5_Minutes;

}

WalletHistoricalWriter::WalletHistoricalWriter(
    BotContext &ctx)
        : ctx(ctx)
{
    ctx.data.subscribe<BtcPrice>(*this);
    ctx.data.subscribe<CoinbaseWallet>(*this);
}

void WalletHistoricalWriter::process(
    const BtcPrice &price)
{
    (void)price;

    utime_t curTime = ctx.data.get<Time>().getTime();

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

    utime_t curTime = ctx.data.get<Time>().getTime();
    if (curTime == prevTime)
        return;

    prevTime = curTime;

    const CoinbaseWallet &wallet = ctx.data.get<CoinbaseWallet>();
    usd_t btcValue = ctx.data.get<BtcPrice>().getPrice();
    usd_t totalValue = wallet.getUsd();
    totalValue += IntegerUtils::getValue(btcValue, wallet.getBtc());

    std::ostringstream query;
    query << "INSERT INTO wallet (time, usd, btc, value) VALUES ("
        << curTime.value() << ","
        << wallet.getUsd().value() << ","
        << wallet.getBtc().value() << ","
        << totalValue.value() << ")";
    if (!ctx.historicalDb.getConn().execute(query.str()))
        log::error("Failed to record wallet data.");
}
