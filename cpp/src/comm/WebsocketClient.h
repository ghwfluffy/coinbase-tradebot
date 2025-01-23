#pragma once

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

#include <nlohmann/json.hpp>

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

        void handleOpen(
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

        websocketpp::connection_hdl conn;
        boost::asio::io_context io;
        WebsockClient client;

        std::string name;
        std::string subscribeMsg;
        std::function<void(nlohmann::json message)> handler;
};

}
