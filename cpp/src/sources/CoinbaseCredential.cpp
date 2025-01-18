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
#include <ctime>

using namespace gtb;

namespace
{

constexpr const char *API_KEY = "../secrets/cdp_api_key.json";

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
        const auto iat = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        const auto exp = iat + 120;

        auto token = jwt::create()
            .set_type("JWT")
            .set_algorithm("ES256")
            .set_issued_now()
            .set_expires_in(std::chrono::seconds(120))
            .set_payload_claim("sub", jwt::claim(uuid))
            .set_payload_claim("iss", jwt::claim(std::string("cdp")))
            .set_payload_claim("uri", jwt::claim(uri))
            .set_header_claim("alg", jwt::claim(std::string("ES256")))
            .set_header_claim("typ", jwt::claim(std::string("JWT")))
            .set_header_claim("kid", jwt::claim(uuid));

        auto signer = jwt::algorithm::es256("", key, "", "");
        return token.sign(signer);
    } catch (const std::exception &e) {
        log::error("Failed to create JWT: %s", e.what());
    }

    return std::string();
}
