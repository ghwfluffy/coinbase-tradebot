#include <gtb/CoinbaseUserInfo.h>
#include <gtb/CoinbaseWallet.h>
#include <gtb/CoinbaseInit.h>
#include <gtb/IntegerUtils.h>
#include <gtb/Log.h>

using namespace gtb;

CoinbaseUserInfo::CoinbaseUserInfo(
    BotContext &ctx)
        : ThreadedDataSource("coinbase-wallet")
        , ctx(ctx)
{
    query();
}

void CoinbaseUserInfo::process()
{
    query();
    sleep(std::chrono::seconds(10));
}

void CoinbaseUserInfo::process(
    const CoinbaseOrderBook &book)
{
    (void)book;

    try {
        query();
    } catch (const std::exception &e) {
        // Ignore
    }
}

void CoinbaseUserInfo::query()
{
    HttpResponse resp = client.get("api/v3/brokerage/accounts");
    if (!resp)
    {
        log::error("Failed to query Coinbase user account information: %s",
            resp.text().c_str());
        return;
    }

    if (!resp.data.contains("accounts"))
    {
        log::error("Coinbase user account information malformed.");
        return;
    }

    // Parse response
    std::string usdBalance = "0.00";
    std::string btcBalance = "0.00";
    for (const auto &account : resp.data["accounts"])
    {
        if (account["currency"] == "USD")
            usdBalance = account["available_balance"]["value"];
        else if (account["currency"] == "BTC")
            btcBalance = account["available_balance"]["value"];
    }

    // Update data model
    ctx.data.get<CoinbaseWallet>().update(
        IntegerUtils::usdToCents(usdBalance),
        IntegerUtils::btcToSatoshi(btcBalance));
    ctx.data.get<CoinbaseInit>().setWalletInit();
}
