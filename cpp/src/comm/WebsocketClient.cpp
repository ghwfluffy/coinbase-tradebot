#include <gtb/WebsocketClient.h>
#include <gtb/Log.h>

using namespace gtb;

WebsocketClient::WebsocketClient(
    std::string name)
        : name(std::move(name))
{
    enabled = true;
}

WebsocketClient::~WebsocketClient()
{
    enabled = false;
    reset();
}

void WebsocketClient::init()
{
    client = std::make_shared<WebsockClient>();
    io = std::make_shared<boost::asio::io_context>();
    idleTimer = std::make_shared<boost::asio::steady_timer>(*io);

    // Use dedicated ASIO event context
    client->init_asio(io.get());

    // Log connect/disconnect and errors only
    client->clear_error_channels(websocketpp::log::elevel::all);
    client->set_error_channels(websocketpp::log::elevel::rerror | websocketpp::log::elevel::fatal);

    client->clear_access_channels(websocketpp::log::alevel::all);
    client->set_access_channels(websocketpp::log::alevel::connect | websocketpp::log::alevel::disconnect);

    // Forward to our logger
    errorLogger = LoggerStreambuf(std::bind(&WebsocketClient::log, this, true, std::placeholders::_1));
    accessLogger = LoggerStreambuf(std::bind(&WebsocketClient::log, this, false, std::placeholders::_1));
    client->get_elog().set_ostream(errorLogger);
    client->get_alog().set_ostream(accessLogger);

    // Setup callbacks
    client->set_tls_init_handler(std::bind(&WebsocketClient::handleTlsInit, this, std::placeholders::_1));
    client->set_open_handler(std::bind(&WebsocketClient::handleOpen, this, client, std::placeholders::_1));
    client->set_message_handler(std::bind(&WebsocketClient::handleMessage, this, std::placeholders::_1, std::placeholders::_2));
    client->set_fail_handler(std::bind(&WebsocketClient::handleFail, this, std::placeholders::_1));
    client->set_close_handler(std::bind(&WebsocketClient::handleClose, this, std::placeholders::_1));
}

void WebsocketClient::updateIdleTimer()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (!idleTimer)
        return;

    // If no traffic after 5 minutes we will reset the connection
    idleTimer->expires_from_now(std::chrono::minutes(5));
    idleTimer->async_wait(
        [this](const boost::system::error_code& ec)
        {
            if (!ec)
            {
                log::info("Idle timeout triggered for websocket '%s'.", name.c_str());
                disconnect();
            }
        });
}

void WebsocketClient::reset()
{
    std::unique_lock<std::mutex> lock(mtx);

    // Save old pointers to free outside of mutex
    WebsockConn oldConn(std::move(conn));
    std::shared_ptr<WebsockClient> oldClient(std::move(client));
    std::shared_ptr<boost::asio::io_context> oldIo(std::move(io));
    std::shared_ptr<boost::asio::steady_timer> oldTimer(std::move(idleTimer));

    io.reset();
    conn.reset();
    client.reset();
    idleTimer.reset();

    lock.unlock();

    // Stop everything
    if (oldClient)
        oldClient->stop_perpetual();
    if (oldIo)
        oldIo->stop();

    // Free memory
    oldConn.reset();
    oldClient.reset();
    oldTimer.reset();
    oldIo.reset();
}

void WebsocketClient::run(
    const std::string &uri,
    const nlohmann::json &subscribeRequest,
    std::function<void(nlohmann::json message)> handler)
{
    disconnect();

    std::unique_lock<std::mutex> lock(mtx);
    // Already running
    if (client)
        return;
    // Completely disconnect
    if (!enabled)
        return;

    // Setup new instance
    init();
    this->handler = std::move(handler);
    this->subscribeMsg = subscribeRequest.dump();

    // Create new connection
    try {
        websocketpp::lib::error_code ec;
        conn = client->get_connection(uri, ec);
        if (ec)
        {
            log::error("Websocket '%s' setup error: %s",
                name.c_str(),
                ec.message().c_str());
            disconnect(lock);
            return;
        }
    } catch (const std::exception &e) {
        log::error("Erorr while initializing websocket '%s': %s", name.c_str(), e.what());
        return;
    }

    // TCP/TLS connect
    try {
        client->connect(conn);
    } catch (const std::exception &e) {
        log::error("Websocket '%s' connection error: %s",
            name.c_str(),
            e.what());
        disconnect(lock);
        return;
    }

    // Run websocket (blocks and triggers callbacks)
    lock.unlock();
    try {
        client->run();
    } catch (const std::exception &e) {
        log::error("Erorr while processing websocket '%s': %s", name.c_str(), e.what());
        disconnect();
    }
}

void WebsocketClient::shutdown()
{
    std::unique_lock<std::mutex> lock(mtx);
    enabled = false;
    disconnect(lock);
}

void WebsocketClient::disconnect()
{
    std::unique_lock<std::mutex> lock(mtx);
    disconnect(lock);
}

void WebsocketClient::disconnect(
    std::unique_lock<std::mutex> &lock)
{
    if (client && conn)
    {
        auto conn2 = conn;
        auto client2 = client;
        lock.unlock();

        log::info("Shutting down '%s' websocket.", name.c_str());
        try {
            websocketpp::connection_hdl handle = conn2->get_handle();
            client2->close(handle, websocketpp::close::status::going_away, "Client disconnect");
        } catch (const std::exception &e) {
            log::error("Erorr while shutting down websocket '%s': %s", name.c_str(), e.what());
        }

        reset();

        lock.lock();
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
    std::shared_ptr<WebsockClient> client,
    websocketpp::connection_hdl hdl)
{
    if (client)
    {
        log::info("Websocket '%s' connection opened.", name.c_str());

        try {
            client->send(hdl, subscribeMsg, websocketpp::frame::opcode::text);
        } catch (const std::exception &e) {
            log::error("Erorr while subscribing websocket '%s': %s", name.c_str(), e.what());
        }

        updateIdleTimer();
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

    updateIdleTimer();
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
