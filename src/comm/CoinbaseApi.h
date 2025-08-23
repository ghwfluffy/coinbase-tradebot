#pragma once

#include <gtb/CoinbaseOrder.h>
#include <gtb/CoinbaseWallet.h>

#include <nlohmann/json.hpp>

namespace gtb
{

namespace CoinbaseApi
{
    CoinbaseOrder parseOrder(
        const nlohmann::json &order);

    nlohmann::json serializeOrder(
        const std::string &clientUuid,
        const CoinbaseOrder &order);

    CoinbaseWallet parseWallet(
        const nlohmann::json &data);
}

}
