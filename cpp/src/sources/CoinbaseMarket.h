#pragma once

#include <gtb/ThreadedDataSource.h>
#include <gtb/BotContext.h>
#include <gtb/BtcPrice.h>
#include <gtb/Time.h>

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

namespace gtb
{

/**
 * Subscribe to Coinbase advanced API websocket and receive updates about
 * Bitcoin trades being executed.
 */
class CoinbaseMarket : public ThreadedDataSource
{
    public:
        CoinbaseMarket(
            BotContext &ctx);
        CoinbaseMarket(CoinbaseMarket &&) = delete;
        CoinbaseMarket(const CoinbaseMarket &) = delete;
        CoinbaseMarket &operator=(CoinbaseMarket &&) = delete;
        CoinbaseMarket &operator=(const CoinbaseMarket &) = delete;
        ~CoinbaseMarket() final = default;

    protected:
        void process() final;
        void shutdown() final;

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

        BotContext &ctx;
        BtcPrice &btc;
        Time &time;

        websocketpp::connection_hdl conn;
        WebsockClient client;
};

}
