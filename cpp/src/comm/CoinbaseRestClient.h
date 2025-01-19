#pragma once

#include <gtb/RestClient.h>
#include <gtb/CoinbaseCredential.h>

namespace gtb
{

/**
 * Wrapper for RestClient that autofills coinbase-specific values
 */
class CoinbaseRestClient
{
    public:
        CoinbaseRestClient();
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

    private:
        RestClient client;
        CoinbaseCredential credential;
};

}
