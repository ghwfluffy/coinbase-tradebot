#include <gtb/CoinbaseUserTrades.h>
#include <gtb/CoinbaseCredential.h>
#include <gtb/CoinbaseOrderBook.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/Time.h>
#include <gtb/Log.h>

#include <nlohmann/json.hpp>

using namespace gtb;

namespace
{

constexpr const char *USER_URI = "wss://advanced-trade-ws-user.coinbase.com";

CoinbaseOrder parseOrder(
    const nlohmann::json &orders)
{
    CoinbaseOrder order;

    if (orders.contains("order_id") && orders["order_id"].is_string()) {
        order.uuid = orders["order_id"].get<std::string>();
    }

    // Parse State
    if (orders.contains("status") && orders["status"].is_string()) {
        const std::string status = orders["status"].get<std::string>();
        if (status == "OPEN") {
            order.state = CoinbaseOrder::State::Open;
        } else if (status == "FILLED") {
            order.state = CoinbaseOrder::State::Filled;
        } else if (status == "CANCELLED") {
            order.state = CoinbaseOrder::State::Canceled;
        } else {
            order.state = CoinbaseOrder::State::Error;
        }
    }

    // Parse Buy/Sell
    if (orders.contains("order_side") && orders["order_side"].is_string()) {
        order.buy = (orders["order_side"].get<std::string>() == "BUY");
    }

    // Parse Price (convert to cents)
    if (orders.contains("limit_price") && orders["limit_price"].is_string()) {
        double price = std::stod(orders["limit_price"].get<std::string>());
        order.priceCents = static_cast<uint32_t>(price * 100);
    }

    // Parse Quantity (convert to 100-millions of a Bitcoin)
    if (orders.contains("leaves_quantity") && orders["leaves_quantity"].is_string()) {
        double quantity = std::stod(orders["leaves_quantity"].get<std::string>());
        order.quantity = static_cast<uint64_t>(quantity * 100'000'000ULL);
    }

    // Parse Created Time
    if (orders.contains("created_at") && orders["created_at"].is_string()) {
        order.createdTime = orders["created_at"].get<std::string>();
    }

    // We will remove non-OPEN orders from the orderbook cache after 2 minutes
    order.cleanupTime = std::chrono::steady_clock::now() + std::chrono::minutes(2);

    return order;
}

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
    CoinbaseOrderBook &book = ctx.data.get<CoinbaseOrderBook>();
    book.update(std::list<CoinbaseOrder>(), true);

    ctx.data.get<Time>().setNow();
}

void CoinbaseUserTrades::update(
    const nlohmann::json &orders,
    bool reset)
{
    std::list<CoinbaseOrder> updates;
    for (const nlohmann::json &data : orders)
    {
        CoinbaseOrder order = parseOrder(data);
        if (order)
            updates.push_back(std::move(order));
    }

    CoinbaseOrderBook &book = ctx.data.get<CoinbaseOrderBook>();
    book.update(std::move(updates), reset);

    ctx.data.get<Time>().setNow();
}
