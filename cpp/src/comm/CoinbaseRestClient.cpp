#include <gtb/CoinbaseRestClient.h>
#include <gtb/CoinbaseApi.h>
#include <gtb/IntegerUtils.h>
#include <gtb/Uuid.h>
#include <gtb/Log.h>

#include <list>

using namespace gtb;

namespace
{

constexpr const char *BASE_URL = "api.coinbase.com";
constexpr const char *API_PREFIX = "/api/v3/";

std::string getError(
    const HttpResponse &resp)
{
    std::string msg;
    if (resp.data.contains("error_response") && resp.data["error_response"].is_object())
    {
        if (resp.data["error_response"].contains("error_details"))
            msg = resp.data["error_response"]["error_details"].get<std::string>();
        else if (resp.data["error_response"].contains("message"))
            msg = resp.data["error_response"]["message"].get<std::string>();
        if (resp.data["error_response"].contains("error"))
            msg = resp.data["error_response"]["error"].get<std::string>();
    }

    if (msg.empty())
        msg = "Unknown.";

    if (resp.code > 0)
        msg = "(" + std::to_string(resp.code) + ") " + msg;

    return msg;
}

}

CoinbaseRestClient::CoinbaseRestClient(
    bool verbose)
        : client("https://" + std::string(BASE_URL), verbose)
{
}

HttpResponse CoinbaseRestClient::post(
    const std::string &path,
    const nlohmann::json &data)
{
    return client.post(API_PREFIX + path, credential.createJwt("POST " + std::string(BASE_URL) + API_PREFIX + path), data);
}

HttpResponse CoinbaseRestClient::get(
    const std::string &path,
    const nlohmann::json &data)
{
    return client.get(API_PREFIX + path, credential.createJwt("GET " + std::string(BASE_URL) + API_PREFIX + path), data);
}

HttpResponse CoinbaseRestClient::del(
    const std::string &path)
{
    return client.del(API_PREFIX + path, credential.createJwt("DELETE " + std::string(BASE_URL) + API_PREFIX + path));
}

CoinbaseOrder CoinbaseRestClient::getOrder(
    const std::string &uuid)
{
    // Build request path
    std::string path = "brokerage/orders/historical/" + uuid;

    // Ask Coinbase
    HttpResponse resp = get(path);
    // Retry at least once so we don't get out of sync due to simple comm issues
    if (!resp)
    {
        log::error("Retrying failed order query.");
        resp = get(path);
    }

    // Parse response
    CoinbaseOrder order;
    if (!resp)
        log::error("Failed to query for order '%s': %s", uuid.c_str(), getError(resp).c_str());
    else if (!resp.data.contains("order") || !resp.data["order"].is_object())
        log::error("No order matching UUID '%s'.", uuid.c_str());
    else
    {
        order = CoinbaseApi::parseOrder(resp.data["order"]);
        if (!order)
            log::error("Received incomplete order response for '%s'.", uuid.c_str());
    }

    return order;
}

CoinbaseOrder CoinbaseRestClient::getOrderByClientUuid(
    const std::string &uuid)
{
    // Build filter
    nlohmann::json filter;
    filter["client_order_id"] = uuid;

    nlohmann::json body;
    body["order_filters"] = std::list<nlohmann::json> {filter};

    // Ask Coinbase
    HttpResponse resp = post("brokerage/orders/historical/batch", body);
    // Retry at least once so we don't get out of sync due to simple comm issues
    if (!resp)
    {
        log::error("Retrying failed order query.");
        resp = post("brokerage/orders/historical/batch", body);
    }

    // Parse response
    CoinbaseOrder order;
    if (!resp)
        log::error("Failed to query for order '%s'.", uuid.c_str());
    else if (!resp.data.contains("orders") || !resp.data["orders"].is_array() || resp.data["orders"].empty())
        log::error("No order matching UUID '%s'.", uuid.c_str());
    else
        order = CoinbaseApi::parseOrder(resp.data["orders"][0]);

    return order;
}

bool CoinbaseRestClient::submitOrder(
    CoinbaseOrder &order)
{
    std::string clientUuid = Uuid::generate();

    nlohmann::json body = CoinbaseApi::serializeOrder(clientUuid, order);

    // Submit to Coinbase
    HttpResponse resp = post("brokerage/orders", body);

    // Parse response
    if (!resp)
    {
        log::error("Failed to submit order (client UUID '%s'): %s", clientUuid.c_str(), getError(resp).c_str());
        return false;
    }

    if (!resp.data.contains("success") || !resp.data["success"].is_boolean())
    {
        log::error("Unexpected response format while creating order '%s'.", clientUuid.c_str());
        return false;
    }

    bool success = resp.data["success"].get<bool>();
    if (!success)
    {
        log::error("Failed to create order '%s': %s", clientUuid.c_str(), getError(resp).c_str());
        return false;
    }

    if (!resp.data.contains("success_response") || !resp.data["success_response"].is_object() ||
        !resp.data["success_response"].contains("order_id"))
    {
        log::error("Unexpected response format while creating order '%s'.", clientUuid.c_str());
        return false;
    }

    order.uuid = resp.data["success_response"]["order_id"].get<std::string>();

    log::info("Submitted new %s order ($%s) (UUID: '%s', Client UUID: '%s').",
        order.buy ? "BUY" : "SELL",
        IntegerUtils::centsToUsd(order.valueCents()).c_str(),
        order.uuid.c_str(),
        clientUuid.c_str());

    return true;
}

bool CoinbaseRestClient::cancelOrder(
    const std::string &uuid)
{
    // Build request path
    std::string path = "brokerage/orders/" + uuid;

    // Send DELETE request to Coinbase
    HttpResponse resp = del(path);

    // Retry at least once to handle transient issues
    if (!resp && resp.code != 404)
    {
        log::error("Retrying failed order cancellation for '%s'.", uuid.c_str());
        resp = del(path);
    }

    if (resp.code == 404)
    {
        log::error("Tried to cancel order '%s' that wasn't active.", uuid.c_str());
        return false;
    }

    // Check response
    if (!resp)
    {
        log::error("Failed to cancel order '%s': %s", uuid.c_str(), getError(resp).c_str());
        return false;
    }

    if (!resp.data.contains("success") || !resp.data["success"].is_boolean())
    {
        log::error("Unexpected response format while canceling order '%s'.", uuid.c_str());
        return false;
    }

    bool success = resp.data["success"].get<bool>();
    if (success)
        log::info("Canceled order '%s'.", uuid.c_str());
    else
    {
        log::error("Failed to cancel order '%s': %s",
            uuid.c_str(),
            getError(resp).c_str());
    }

    return success;
}

CoinbaseWallet::Data CoinbaseRestClient::getWallet()
{
    HttpResponse resp = get("brokerage/accounts");
    // Retry at least once to handle transient issues
    if (!resp)
    {
        log::error("Retrying failed wallet query.");
        resp = get("brokerage/accounts");
    }

    if (!resp)
    {
        log::error("Failed to query Coinbase user account information: %s",
            getError(resp).c_str());
        return CoinbaseWallet::Data();
    }

    if (!resp.data.contains("accounts"))
    {
        log::error("Coinbase user account information malformed.");
        return CoinbaseWallet::Data();
    }

    // Parse response
    std::string usdBalance = "0.00";
    std::string btcBalance = "0.00";
    std::string usdReserved = "0.00";
    std::string btcReserved = "0.00";
    for (const auto &account : resp.data["accounts"])
    {
        if (account["currency"] == "USD")
        {
            usdBalance = account["available_balance"]["value"];
            usdReserved = account["hold"]["value"];
        }
        else if (account["currency"] == "BTC")
        {
            btcBalance = account["available_balance"]["value"];
            btcReserved = account["hold"]["value"];
        }
    }

    CoinbaseWallet::Data ret;
    ret.usd = IntegerUtils::usdToCents(usdBalance) + IntegerUtils::usdToCents(usdReserved);
    ret.btc = IntegerUtils::btcToSatoshi(btcBalance) + IntegerUtils::btcToSatoshi(btcReserved);
    return ret;
}

uint32_t CoinbaseRestClient::getFeeTier()
{
    HttpResponse resp = get("brokerage/transaction_summary");
    // Retry at least once to handle transient issues
    if (!resp)
    {
        log::error("Retrying transaction summary query.");
        resp = get("brokerage/transaction_summary");
    }

    if (!resp)
    {
        log::error("Failed to query Coinbase transactions summary: %s",
            getError(resp).c_str());
        return 0;
    }

    if (!resp.data.contains("fee_tier"))
    {
        log::error("Coinbase transaction summary malformed.");
        return 0;
    }

    // Parse response
    std::string fee = resp.data["fee_tier"]["maker_fee_rate"].get<std::string>();
    size_t pos = fee.find(".");
    if (pos == std::string::npos)
        return 0;

    fee.erase(0, pos + 1);
    fee.resize(4, '0');
    return static_cast<uint32_t>(atoi(fee.c_str()));
}
