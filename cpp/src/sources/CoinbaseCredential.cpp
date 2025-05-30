#include <gtb/CoinbaseCredential.h>
#include <gtb/Log.h>

#include <nlohmann/json.hpp>
#include <jwt-cpp/jwt.h>

#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <ctime>
#include <set>

using namespace gtb;

namespace
{

constexpr const char *API_KEY = "../secrets/cdp_api_key.json";

std::string genNonce(size_t length = 64)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::default_random_engine rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, sizeof(charset) - 2);

    std::string nonce;
    nonce.reserve(length);
    for (size_t i = 0; i < length; ++i)
        nonce += charset[dist(rng)];
    return nonce;
}

}

CoinbaseCredential::CoinbaseCredential()
{
    init();
}

void CoinbaseCredential::init()
{
    std::ifstream jsonFile(API_KEY);
    if (!jsonFile.is_open())
    {
        log::error("Failed to open API key '%s'.", API_KEY);
        return;
    }

    std::stringstream buffer;
    buffer << jsonFile.rdbuf();

    try {
        nlohmann::json json = nlohmann::json::parse(buffer);
        this->uuid = std::move(json["name"].get<std::string>());
        this->key = std::move(json["privateKey"].get<std::string>());
    } catch (const std::exception &e) {
        log::error("Failed to read in API key: %s", e.what());
    }
}

std::string CoinbaseCredential::createJwt(
    const std::string &uri)
{
    try {
        const auto now = std::chrono::system_clock::now();

        auto token = jwt::create()
            .set_type("JWT")
            .set_algorithm("ES256")
            .set_issued_now()
            .set_expires_in(std::chrono::seconds(120))
            .set_payload_claim("nbf", jwt::claim(now))
            .set_payload_claim("aud", jwt::claim(std::set<std::string> {
                "public_websocket_api",
                "retail_rest_api_proxy",
            }))
            .set_payload_claim("sub", jwt::claim(uuid))
            .set_payload_claim("iss", jwt::claim(std::string("cdp")))
            .set_payload_claim("uri", jwt::claim(uri))
            .set_header_claim("alg", jwt::claim(std::string("ES256")))
            .set_header_claim("typ", jwt::claim(std::string("JWT")))
            .set_header_claim("kid", jwt::claim(uuid))
            .set_header_claim("nonce", jwt::claim(genNonce()));

        auto signer = jwt::algorithm::es256("", key, "", "");
        return token.sign(signer);
    } catch (const std::exception &e) {
        log::error("Failed to create JWT: %s", e.what());
    }

    return std::string();
}
