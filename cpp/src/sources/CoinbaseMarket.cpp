#include <gtb/CoinbaseMarket.h>
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
{
    client.init_asio();
}

void CoinbaseMarket::process()
{
    // Disable all logs
    client.clear_access_channels(websocketpp::log::alevel::all);
    client.clear_error_channels(websocketpp::log::elevel::all);

    client.set_tls_init_handler(std::bind(&CoinbaseMarket::handleTlsInit, this, std::placeholders::_1));
    client.set_open_handler(std::bind(&CoinbaseMarket::handleOpen, this, std::placeholders::_1));
    client.set_message_handler(std::bind(&CoinbaseMarket::handleMessage, this, std::placeholders::_1, std::placeholders::_2));
    client.set_fail_handler(std::bind(&CoinbaseMarket::handleFail, this, std::placeholders::_1));
    client.set_close_handler(std::bind(&CoinbaseMarket::handleClose, this, std::placeholders::_1));

    websocketpp::lib::error_code ec;
    auto con = client.get_connection(TRADE_URI, ec);
    if (ec)
    {
        log::error("Coinbase market connection error: %s", ec.message().c_str());
        sleep(std::chrono::seconds(1));
        return;
    }

    client.connect(con);
    client.run();
}

void CoinbaseMarket::shutdown()
{
    if (conn.lock())
    {
        log::info("Shutting down Coinbase market websocket.");
        client.close(conn, websocketpp::close::status::going_away, "Client shutdown");
    }
}

std::shared_ptr<boost::asio::ssl::context> CoinbaseMarket::handleTlsInit(websocketpp::connection_hdl hdl)
{
    auto ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tls_client);
    try {
        ctx->set_default_verify_paths(); // Use the system's default trusted CA certificates
    } catch (std::exception& e) {
        std::cerr << "Error in TLS initialization: " << e.what() << std::endl;
    }
    return ctx;
}

void CoinbaseMarket::handleOpen(websocketpp::connection_hdl hdl)
{
    log::info("Coinbase market connection opened.");
    this->conn = std::move(hdl);

    nlohmann::json json;
    json["type"] = "subscribe";
    json["channel"] = "market_trades";
    json["product_ids"] = std::list<std::string> {"BTC-USD"};

    std::string msg = json.dump();
    client.send(conn, msg, websocketpp::frame::opcode::text);
}

void CoinbaseMarket::handleClose(websocketpp::connection_hdl hdl)
{
    log::info("Coinbase market connection closed.");
}

void CoinbaseMarket::handleFail(websocketpp::connection_hdl hdl)
{
    log::error("Coinbase market connection error.");
}

void CoinbaseMarket::handleMessage(
    websocketpp::connection_hdl hdl,
    WebsockClient::message_ptr msg)
{
    try {
        nlohmann::json json = nlohmann::json::parse(msg->get_payload());
        do {
            if (!json.contains("channel") || json["channel"].get<std::string>() != "market_trades")
                break;
            if (!json.contains("events") || !json["events"].is_array() || json["events"].empty())
                break;

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
                    size_t period = price.find('.');
                    uint32_t cents = atoi(price.c_str()) * 100;
                    if (period != std::string::npos)
                        cents += atoi(price.c_str() + period + 1);
                    total += cents;
                    count++;
                }
            }

            if (count > 0)
            {
                time.setNow();
                btc.setCents(total / count);
            }
        } while (false);
    } catch (const std::exception &e) {
        log::error("Failed to parse Coinbase market data message: %s", e.what());
    }
}
