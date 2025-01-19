#include <gtb/CoinbaseRestClient.h>

using namespace gtb;

namespace
{

constexpr const char *BASE_URL = "api.coinbase.com";

}

CoinbaseRestClient::CoinbaseRestClient()
    : client("https://" + std::string(BASE_URL))
{
}

HttpResponse CoinbaseRestClient::post(
    const std::string &path,
    const nlohmann::json &data)
{
    return client.post(path, credential.createJwt("POST " + std::string(BASE_URL) + "/" + path), data);
}

HttpResponse CoinbaseRestClient::get(
    const std::string &path,
    const nlohmann::json &data)
{
    return client.get(path, credential.createJwt("GET " + std::string(BASE_URL) + "/" + path), data);
}
