#include <gtb/WebsocketClient.h>
#include <gtb/Log.h>

using namespace gtb;

WebsocketClient::WebsocketClient(
    std::string name)
        : name(std::move(name))
{
    init();
}

WebsocketClient::~WebsocketClient()
{
    shutdown();
}

void WebsocketClient::init()
{
    // Use dedicated ASIO event context
    client.init_asio(&io);

    // Log connect/disconnect and errors only
    client.clear_error_channels(websocketpp::log::elevel::all);
    client.set_error_channels(websocketpp::log::elevel::rerror | websocketpp::log::elevel::fatal);

    client.clear_access_channels(websocketpp::log::alevel::all);
    client.set_access_channels(websocketpp::log::alevel::connect | websocketpp::log::alevel::disconnect);

    // Forward to our logger
    errorLogger = LoggerStreambuf(std::bind(&WebsocketClient::log, this, true, std::placeholders::_1));
    accessLogger = LoggerStreambuf(std::bind(&WebsocketClient::log, this, false, std::placeholders::_1));
    client.get_elog().set_ostream(errorLogger);
    client.get_alog().set_ostream(accessLogger);

    // Setup callbacks
    client.set_tls_init_handler(std::bind(&WebsocketClient::handleTlsInit, this, std::placeholders::_1));
    client.set_open_handler(std::bind(&WebsocketClient::handleOpen, this, std::placeholders::_1));
    client.set_message_handler(std::bind(&WebsocketClient::handleMessage, this, std::placeholders::_1, std::placeholders::_2));
    client.set_fail_handler(std::bind(&WebsocketClient::handleFail, this, std::placeholders::_1));
    client.set_close_handler(std::bind(&WebsocketClient::handleClose, this, std::placeholders::_1));
}

void WebsocketClient::run(
    const std::string &uri,
    const nlohmann::json &subscribeRequest,
    std::function<void(nlohmann::json message)> handler)
{
    std::unique_lock<std::mutex> lock(mtx);

    this->handler = std::move(handler);
    subscribeMsg = subscribeRequest.dump();

    // Cleanup old connection
    shutdown(lock);

    // Create new connection
    try {
        websocketpp::lib::error_code ec;
        conn = client.get_connection(uri, ec);
        if (ec)
        {
            log::error("Websocket '%s' setup error: %s",
                name.c_str(),
                ec.message().c_str());
            shutdown(lock);
            return;
        }
    } catch (const std::exception &e) {
        log::error("Erorr while initializing websocket '%s': %s", name.c_str(), e.what());
        return;
    }

    // TCP/TLS connect
    try {
        client.connect(conn);
    } catch (const std::exception &e) {
        log::error("Websocket '%s' connection error: %s",
            name.c_str(),
            e.what());
        shutdown(lock);
        return;
    }

    // Run websocket (blocks and triggers callbacks)
    lock.unlock();
    try {
        client.run();
    } catch (const std::exception &e) {
        log::error("Erorr while processing websocket '%s': %s", name.c_str(), e.what());
        shutdown();
    }
}

void WebsocketClient::shutdown()
{
    std::unique_lock<std::mutex> lock(mtx);
    shutdown(lock);
}

void WebsocketClient::shutdown(
    const std::unique_lock<std::mutex> &lock)
{
    (void)lock;

    if (conn)
    {
        log::info("Shutting down '%s' websocket.", name.c_str());
        try {
            websocketpp::connection_hdl handle = conn->get_handle();
            client.close(handle, websocketpp::close::status::going_away, "Client shutdown");
            conn.reset();
        } catch (const std::exception &e) {
            log::error("Erorr while shutting down websocket '%s': %s", name.c_str(), e.what());
            conn.reset();
        }
    }
}

std::shared_ptr<boost::asio::ssl::context> WebsocketClient::handleTlsInit(
    websocketpp::connection_hdl hdl)
{
    (void)hdl;

    if (!tlsCtx)
    {
        tlsCtx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tls_client);
        try {
            // Use the system's default trusted CA certificates
            tlsCtx->set_default_verify_paths();
        } catch (const std::exception &e) {
            log::error("Failed to setup '%s' websocket TLS: %s", name.c_str(), e.what());
            tlsCtx.reset();
        }
    }

    return tlsCtx;
}

void WebsocketClient::handleOpen(
    websocketpp::connection_hdl hdl)
{
    log::info("Websocket '%s' connection opened.", name.c_str());

    try {
        client.send(hdl, subscribeMsg, websocketpp::frame::opcode::text);
    } catch (const std::exception &e) {
        log::error("Erorr while subscribing websocket '%s': %s", name.c_str(), e.what());
    }
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

void WebsocketClient::log(
    bool error,
    std::string msg)
{
    // Remove timestamp
    size_t pos = msg.find(']');
    if (pos != std::string::npos)
        msg.erase(0, pos + 1);
    if (msg[0] == ' ')
        msg.erase(0, 1);

    if (error)
        log::error("WebSocket++: %s", msg.c_str());
    else
        log::info("WebSocket++: %s", msg.c_str());
}
