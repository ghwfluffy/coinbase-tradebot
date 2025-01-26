#pragma once

#include <gtb/RestClient.h>
#include <gtb/CoinbaseInterface.h>
#include <gtb/CoinbaseCredential.h>

namespace gtb
{

/**
 * Wrapper for RestClient that autofills coinbase-specific values
 */
class CoinbaseRestClient : public CoinbaseInterface
{
    public:
        CoinbaseRestClient(
            bool verbose = false);
        CoinbaseRestClient(CoinbaseRestClient &&) = delete;
        CoinbaseRestClient(const CoinbaseRestClient &) = delete;
        CoinbaseRestClient &operator=(CoinbaseRestClient &&) = delete;
        CoinbaseRestClient &operator=(const CoinbaseRestClient &) = delete;
        ~CoinbaseRestClient() = default;

        HttpResponse post(
            const std::string &path,
            const nlohmann::json &data);

        HttpResponse get(
            const std::string &path,
            const nlohmann::json &data = nlohmann::json());

        HttpResponse del(
            const std::string &path);

        CoinbaseOrder getOrder(
            const std::string &uuid) final;

        CoinbaseOrder getOrderByClientUuid(
            const std::string &uuid);

        bool submitOrder(
            CoinbaseOrder &order) final;

        bool cancelOrder(
            const std::string &uuid) final;

        CoinbaseWallet::Data getWallet() final;

        uint32_t getFeeTier() final;

    private:
        RestClient client;
        CoinbaseCredential credential;
};

}
