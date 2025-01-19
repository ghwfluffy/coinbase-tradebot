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
        CoinbaseCredential(CoinbaseCredential &&) = default;
        CoinbaseCredential(const CoinbaseCredential &) = default;
        CoinbaseCredential &operator=(CoinbaseCredential &&) = default;
        CoinbaseCredential &operator=(const CoinbaseCredential &) = default;
        ~CoinbaseCredential() = default;

        std::string createJwt(
            const std::string &uri);

    private:
        void init();

        std::string key;
        std::string uuid;
};

}
