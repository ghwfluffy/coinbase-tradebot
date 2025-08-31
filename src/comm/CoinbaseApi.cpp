#include <gtb/CoinbaseApi.h>
#include <gtb/IntegerUtils.h>

#include <sstream>
#include <iomanip>

using namespace gtb;

CoinbaseOrder CoinbaseApi::parseOrder(
    const nlohmann::json &data)
{
    CoinbaseOrder order;

    // We will use the UUID coinbase generates (as opposed to client_order_id)
    if (data.contains("order_id") && data["order_id"].is_string())
        order.uuid = data["order_id"].get<std::string>();

    // Parse State
    if (data.contains("status") && data["status"].is_string())
    {
        const std::string status = data["status"].get<std::string>();
        if (status == "OPEN")
            order.state = CoinbaseOrder::State::Open;
        else if (status == "FILLED")
            order.state = CoinbaseOrder::State::Filled;
        else if (status == "CANCELLED")
            order.state = CoinbaseOrder::State::Canceled;
        else
            order.state = CoinbaseOrder::State::Error;
    }

    // Parse Buy/Sell
    if (data.contains("order_side") && data["order_side"].is_string())
        order.buy = (data["order_side"].get<std::string>() == "BUY");

    // Parse Price/Quantity
    // GET order
    if (data.contains("order_configuration") && data["order_configuration"].contains("limit_limit_gtc"))
    {
        order.price = IntegerUtils::fromUsdString(
            data["order_configuration"]["limit_limit_gtc"]["limit_price"].get<std::string>()
        );

        // Buys
        if (data["order_configuration"]["limit_limit_gtc"].contains("quote_size"))
        {
            usd_t amount = IntegerUtils::fromUsdString(
                data["order_configuration"]["limit_limit_gtc"]["quote_size"].get<std::string>()
            );
            order.setQuantity(order.price, amount);
        }
        // Sells
        else if (data["order_configuration"]["limit_limit_gtc"].contains("base_size"))
        {
            order.quantity = IntegerUtils::fromBtcString(
                data["order_configuration"]["limit_limit_gtc"]["base_size"].get<std::string>()
            );
        }
    }
    // Websocket order
    else
    {
        if (data.contains("avg_price"))
            order.price = IntegerUtils::fromUsdString(data["avg_price"].get<std::string>());
        else if (data.contains("limit_price"))
            order.price = IntegerUtils::fromUsdString(data["limit_price"].get<std::string>());

        usd_t amount;
        if (data.contains("filled_value"))
            amount += IntegerUtils::fromUsdString(data["filled_value"].get<std::string>());
        if (data.contains("leaves_quantity"))
            amount += IntegerUtils::fromUsdString(data["leaves_quantity"].get<std::string>());
        if (data.contains("total_fees"))
            amount += IntegerUtils::fromUsdString(data["total_fees"].get<std::string>());

        order.setQuantity(order.price, amount);
    }

    // Parse Created Time (convert to microseconds since epoch)
    if (data.contains("created_at") && data["created_at"].is_string())
    {
        std::string createdAtStr = data["created_at"].get<std::string>();

        // Parse the string into a time_point
        std::tm tm = {};
        std::istringstream ss(createdAtStr);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

        if (ss.fail())
            order.createdTime = {};
        else
        {
            // Convert to time_point (UTC assumed)
            auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

            // Parse milliseconds if present (e.g., ".123Z")
            size_t dotPos = createdAtStr.find('.');
            if (dotPos != std::string::npos)
            {
                // Up to milliseconds
                std::string fractional = createdAtStr.substr(dotPos + 1, 3);
                // Handle "Z" or end of string
                fractional = fractional.substr(0, fractional.find('Z'));
                int millis = std::stoi(fractional);
                tp += std::chrono::milliseconds(millis);
            }

            // Convert time_point to microseconds since epoch
            order.createdTime = utime_t(static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::microseconds>(
                    tp.time_since_epoch()
                ).count()));
        }
    }

    if (data.contains("total_fees"))
        order.fees = IntegerUtils::fromUsdString(data["total_fees"].get<std::string>());

    if (data.contains("filled_value"))
    {
        order.beforeFees = IntegerUtils::fromUsdString(data["filled_value"].get<std::string>());
        if (order.buy)
            order.beforeFees += order.fees;
    }

    return order;
}

nlohmann::json CoinbaseApi::serializeOrder(
    const std::string &clientUuid,
    const CoinbaseOrder &order)
{
    nlohmann::json gtc;
    // Force maker, never taker
    gtc["post_only"] = true;
    // Most we want to buy/Least we want to sell it for
    gtc["limit_price"] = IntegerUtils::toUsdString(order.price);

    // Include only one of base_size or quote_size
    // Buy we will go based off how much we want to spend
    if (order.buy)
        gtc["quote_size"] = IntegerUtils::toUsdString(order.value());
    // Sell we will sell exactly how many Satoshi we bought
    else
        gtc["base_size"] = IntegerUtils::toBtcString(order.quantity);

    nlohmann::json conf;
    conf["limit_limit_gtc"] = std::move(gtc);

    nlohmann::json body;
    body["client_order_id"] = clientUuid;
    body["product_id"] = "BTC-USD";
    body["side"] = order.buy ? "BUY" : "SELL";
    body["order_configuration"] = std::move(conf);

    return body;
}
