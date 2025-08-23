#include <gtb/CoinbaseUserTrades.h>
#include <gtb/CoinbaseCredential.h>
#include <gtb/CoinbaseOrderBook.h>
#include <gtb/CoinbaseWallet.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/CoinbaseApi.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

#include <nlohmann/json.hpp>

using namespace gtb;

namespace
{

constexpr const char *USER_URI = "wss://advanced-trade-ws-user.coinbase.com";

}

CoinbaseUserTrades::CoinbaseUserTrades(
    BotContext &ctx)
        : ThreadedDataSource("coinbase-user-trades")
        , ctx(ctx)
        , client("coinbase-user-trades")
{
}

void CoinbaseUserTrades::process()
{
    nlohmann::json json;

    std::string jwt = CoinbaseCredential().createJwt(USER_URI);

    json["type"] = "subscribe";
    json["channel"] = "user";
    json["product_ids"] = std::list<std::string> {"BTC-USD"};
    json["jwt"] = std::move(jwt);

    client.run(USER_URI, json, std::bind(&CoinbaseUserTrades::handleMessage, this, std::placeholders::_1));
    sleep(std::chrono::seconds(1));
}

void CoinbaseUserTrades::shutdown()
{
    client.shutdown();
}

void CoinbaseUserTrades::handleMessage(
    nlohmann::json json)
{
    if (!json.contains("channel") || json["channel"].get<std::string>() != "user")
        return;
    if (!json.contains("events") || !json["events"].is_array() || json["events"].empty())
        return;

    ctx.data.get<Time>().setNow();

    // Update wallet on orderbook change
    ctx.data.get<CoinbaseWallet>().update(ctx.coinbase().getWallet());

    for (const nlohmann::json &event : json["events"])
    {
        if (!event.contains("type") || !event["type"].is_string())
            continue;

        if (!event.contains("orders") || !event["orders"].is_array())
            continue;

        std::string type = event["type"].get<std::string>();
        if (type == "snapshot")
        {
            if (event["orders"].empty())
                reset();
            else
                update(event["orders"], true);

            ctx.data.get<CoinbaseInit>().setOrderBookInit();
        }
        else if (type == "update")
        {
            if (event["orders"].empty())
                continue;

            update(event["orders"]);
        }
    }
}

void CoinbaseUserTrades::reset()
{
    ctx.data.get<Time>().setNow();

    CoinbaseOrderBook &book = ctx.data.get<CoinbaseOrderBook>();
    book.update(std::list<CoinbaseOrder>(), true);
}

void CoinbaseUserTrades::update(
    const nlohmann::json &orders,
    bool reset)
{
    std::list<CoinbaseOrder> updates;
    for (const nlohmann::json &data : orders)
    {
        CoinbaseOrder order = CoinbaseApi::parseOrder(data);
        if (order)
            updates.push_back(std::move(order));
    }

    ctx.data.get<Time>().setNow();

    CoinbaseOrderBook &book = ctx.data.get<CoinbaseOrderBook>();
    book.update(std::move(updates), reset);
}
