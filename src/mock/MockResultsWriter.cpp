#include <gtb/MockResultsWriter.h>
#include <gtb/BtcPrice.h>
#include <gtb/Profits.h>
#include <gtb/Log.h>

#include <nlohmann/json.hpp>

#include <sstream>
#include <iomanip>

#include <time.h>
#include <stdio.h>

using namespace gtb;

namespace
{

std::tm getTm(
    utime_t uTime)
{
    auto now = std::chrono::system_clock::time_point(std::chrono::microseconds(uTime.value()));
    auto now_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm local_tm;
    localtime_r(&now_time_t, &local_tm);
    return local_tm;
}

std::string getTimestamp(
    utime_t uTime)
{
    std::tm local_tm = getTm(uTime);
    std::ostringstream time_stream;
    time_stream << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    return time_stream.str();
}

std::string getDayOfWeek(
    utime_t uTime)
{
    std::tm local_tm = getTm(uTime);
    std::ostringstream time_stream;
    time_stream << std::put_time(&local_tm, "%a");
    return time_stream.str();
}

}

MockResultsWriter::MockResultsWriter(
    BotContext &ctx,
    std::string fn)
        : ctx(ctx)
        , filename(std::move(fn))
{
    ctx.data.subscribe<Time>(*this);

    prevPrint = SteadyClock::now();

    // Backup the previous run if it exists
    std::string backup = "mv \"" + filename + "\" \"" + filename + ".bak\"";
    __attribute__((unused)) int iRet = system(backup.c_str());
}

void MockResultsWriter::process(
    const Time &time)
{
    std::lock_guard<std::mutex> lock(mtx);

    // Time to print?
    SteadyClock::TimePoint now = SteadyClock::now();
    if (nextPrint > now)
        return;

    // Collect some info
    utime_t elapsed = now.time - prevPrint.time;
    uint64_t elapsedSeconds = elapsed / 1_Seconds;
    usd_t curPrice = ctx.data.get<BtcPrice>().getPrice();

    // Build output
    nlohmann::json json;
    json["time"] = getTimestamp(time.getTime());
    json["dayOfWeek"] = getDayOfWeek(time.getTime());
    json["elapsed"] = elapsedSeconds;
    json["totalVolume"] = IntegerUtils::toDollars(ctx.data.get<Profits>().getVolume());
    json["recentVolume"] = IntegerUtils::toDollars(ctx.coinbase().getVolume());

    // Output each trader
    std::vector<nlohmann::json> traders;
    for (const auto &[trader, data] : ctx.data.get<Profits>().getAllTraderData())
    {
        nlohmann::json jsonTrader;
        jsonTrader["name"] = trader;
        jsonTrader["buys"] = data.buys;
        jsonTrader["sells"] = data.sells;
        jsonTrader["pendingSells"] = data.buys - data.sells;
        jsonTrader["volume"] = IntegerUtils::toDollars(data.getVolume());
        jsonTrader["profit"] = IntegerUtils::toDollars(data.getProfit(curPrice));
        jsonTrader["holding"] = IntegerUtils::toDollars(IntegerUtils::getValue(curPrice, data.getPending()));
        jsonTrader["totalFees"] = IntegerUtils::toDollars(data.buyFees + data.sellFees);
        traders.push_back(std::move(jsonTrader));
    }

    json["traders"] = std::move(traders);

    // Write to file
    FILE *fp = fopen(filename.c_str(), "a");
    if (!fp)
        log::error("Failed to open mock results file '%s'.", filename.c_str());
    else
    {
        fprintf(fp, "%s\n", json.dump().c_str());
        fclose(fp);
    }

    // Queue next run
    prevPrint = now;
    nextPrint = now + std::chrono::minutes(10);
}
