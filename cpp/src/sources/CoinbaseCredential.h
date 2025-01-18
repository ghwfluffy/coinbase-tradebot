#pragma once

#include <string>

namespace gtb
{

/**
 * Reads in the Coinbase API credential
 */
class CoinbaseCredential
{
    public:
        CoinbaseCredential();
        CoinbaseCredential(CoinbaseCredential &&) = delete;
        CoinbaseCredential(const CoinbaseCredential &) = delete;
        CoinbaseCredential &operator=(CoinbaseCredential &&) = delete;
        CoinbaseCredential &operator=(const CoinbaseCredential &) = delete;
        ~CoinbaseCredential() = default;

        std::string createJwt(
            const std::string &uri);

    private:
        void init();

        std::string key;
        std::string uuid;
};

}
