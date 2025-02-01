#pragma once

#include <gtb/LoggerStreambuf.h>

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

#include <nlohmann/json.hpp>

#include <mutex>
#include <functional>

namespace gtb
{

/**
 * Handle setup and tear down of websocket client
 */
class WebsocketClient
{
    public:
        WebsocketClient(
            std::string name);
        WebsocketClient(WebsocketClient &&) = delete;
        WebsocketClient(const WebsocketClient &) = delete;
        WebsocketClient &operator=(WebsocketClient &&) = delete;
        WebsocketClient &operator=(const WebsocketClient &) = delete;
        ~WebsocketClient();

        void run(
            const std::string &uri,
            const nlohmann::json &subscribeRequest,
            std::function<void(nlohmann::json message)> handler);

        void shutdown();

    private:
        typedef websocketpp::client<websocketpp::config::asio_tls_client> WebsockClient;
        typedef std::shared_ptr<websocketpp::connection<websocketpp::config::asio_tls_client>> WebsockConn;

        void init();
        void reset();
        void shutdown(
            const std::unique_lock<std::mutex> &lock);
        void updateIdleTimer();

        void log(
            bool error,
            std::string msg);

        void handleOpen(
            std::shared_ptr<WebsockClient> client,
            websocketpp::connection_hdl hdl);
        void handleClose(
            websocketpp::connection_hdl hdl);
        void handleFail(
            websocketpp::connection_hdl hdl);
        std::shared_ptr<boost::asio::ssl::context> handleTlsInit(
            websocketpp::connection_hdl hdl);
        void handleMessage(
            websocketpp::connection_hdl hdl,
            WebsockClient::message_ptr msg);

        WebsockConn conn;
        std::shared_ptr<WebsockClient> client;
        std::shared_ptr<boost::asio::io_context> io;
        std::shared_ptr<boost::asio::ssl::context> tlsCtx;
        std::shared_ptr<boost::asio::steady_timer> idleTimer;

        std::mutex mtx;
        std::string name;
        std::string subscribeMsg;
        std::function<void(nlohmann::json message)> handler;

        LoggerStreambuf errorLogger;
        LoggerStreambuf accessLogger;
};

}
