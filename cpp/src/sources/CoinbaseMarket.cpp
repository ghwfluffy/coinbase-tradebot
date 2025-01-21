#include <gtb/CoinbaseMarket.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/IntegerUtils.h>
#include <gtb/Log.h>

#include <nlohmann/json.hpp>

using namespace gtb;

namespace
{

constexpr const char *TRADE_URI = "wss://advanced-trade-ws.coinbase.com";

}

CoinbaseMarket::CoinbaseMarket(
    BotContext &ctx)
        : ThreadedDataSource("coinbase-market")
        , ctx(ctx)
        , btc(ctx.data.get<BtcPrice>())
        , time(ctx.data.get<Time>())
        , client("coinbase-market")
{
}

void CoinbaseMarket::process()
{
    nlohmann::json json;
    json["type"] = "subscribe";
    json["channel"] = "market_trades";
    json["product_ids"] = std::list<std::string> {"BTC-USD"};

    client.run(TRADE_URI, json, std::bind(&CoinbaseMarket::handleMessage, this, std::placeholders::_1));
    sleep(std::chrono::seconds(1));
}

void CoinbaseMarket::shutdown()
{
    client.shutdown();
}

void CoinbaseMarket::handleMessage(
    nlohmann::json json)
{
    if (!json.contains("channel") || json["channel"].get<std::string>() != "market_trades")
        return;
    if (!json.contains("events") || !json["events"].is_array() || json["events"].empty())
        return;

    uint64_t total = 0;
    uint32_t count = 0;
    for (const nlohmann::json &event : json["events"])
    {
        if (!event.contains("trades") || !event["trades"].is_array() || event["trades"].empty())
            continue;

        for (const nlohmann::json &trade : event["trades"])
        {
            if (!trade.contains("price") || !trade["price"].is_string())
                continue;

            std::string price = trade["price"].get<std::string>();
            total += IntegerUtils::usdToCents(price);
            count++;
        }
    }

    if (count > 0)
    {
        time.setNow();
        btc.setCents(static_cast<uint32_t>(total / count));
        ctx.data.get<CoinbaseInit>().setBtcInit();
    }
}
