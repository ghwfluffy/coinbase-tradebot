#include <gtb/WebsocketClient.h>
#include <gtb/Log.h>

using namespace gtb;

WebsocketClient::WebsocketClient(
    std::string name)
        : name(std::move(name))
{
    client.init_asio(&io);
}

WebsocketClient::~WebsocketClient()
{
    shutdown();
}

void WebsocketClient::run(
    const std::string &uri,
    const nlohmann::json &subscribeRequest,
    std::function<void(nlohmann::json message)> handler)
{
    this->handler = std::move(handler);
    subscribeMsg = subscribeRequest.dump();

    // Disable all logs
    client.clear_access_channels(websocketpp::log::alevel::all);
    client.clear_error_channels(websocketpp::log::elevel::all);

    client.set_tls_init_handler(std::bind(&WebsocketClient::handleTlsInit, this, std::placeholders::_1));
    client.set_open_handler(std::bind(&WebsocketClient::handleOpen, this, std::placeholders::_1));
    client.set_message_handler(std::bind(&WebsocketClient::handleMessage, this, std::placeholders::_1, std::placeholders::_2));
    client.set_fail_handler(std::bind(&WebsocketClient::handleFail, this, std::placeholders::_1));
    client.set_close_handler(std::bind(&WebsocketClient::handleClose, this, std::placeholders::_1));

    websocketpp::lib::error_code ec;
    auto con = client.get_connection(uri, ec);
    if (ec)
    {
        log::error("Websocket '%s' connection error: %s",
            name.c_str(),
            ec.message().c_str());
        return;
    }

    client.connect(con);
    client.run();
}

void WebsocketClient::shutdown()
{
    if (conn.lock())
    {
        log::info("Shutting down '%s' websocket.", name.c_str());
        client.close(conn, websocketpp::close::status::going_away, "Client shutdown");
    }
}

std::shared_ptr<boost::asio::ssl::context> WebsocketClient::handleTlsInit(
    websocketpp::connection_hdl hdl)
{
    (void)hdl;

    auto ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tls_client);
    try {
        // Use the system's default trusted CA certificates
        ctx->set_default_verify_paths();
    } catch (const std::exception &e) {
        log::error("Failed to setup '%s' websocket TLS: %s", name.c_str(), e.what());
    }

    return ctx;
}

void WebsocketClient::handleOpen(
    websocketpp::connection_hdl hdl)
{
    log::info("Websocket '%s' connection opened.", name.c_str());
    this->conn = std::move(hdl);

    client.send(conn, subscribeMsg, websocketpp::frame::opcode::text);
}

void WebsocketClient::handleClose(
    websocketpp::connection_hdl hdl)
{
    (void)hdl;

    log::info("Websocket '%s' connection closed.", name.c_str());
}

void WebsocketClient::handleFail(
    websocketpp::connection_hdl hdl)
{
    (void)hdl;

    log::error("Websocket '%s' connection error.", name.c_str());
}

void WebsocketClient::handleMessage(
    websocketpp::connection_hdl hdl,
    WebsockClient::message_ptr msg)
{
    (void)hdl;

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(msg->get_payload());
    } catch (const std::exception &e) {
        log::error("Failed to parse '%s' weboscket data message: %s",
            name.c_str(),
            e.what());
        return;
    }

    try {
        handler(std::move(json));
    } catch (const std::exception &e) {
        log::error("Failed to handle '%s' weboscket data: %s",
            name.c_str(),
            e.what());
    }
}
