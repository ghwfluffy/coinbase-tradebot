#include <gtb/CoinbaseUserInfo.h>
#include <gtb/CoinbaseFeeTier.h>
#include <gtb/CoinbaseWallet.h>
#include <gtb/CoinbaseInit.h>

using namespace gtb;

CoinbaseUserInfo::CoinbaseUserInfo(
    BotContext &ctx)
        : ThreadedDataSource("coinbase-wallet")
        , ctx(ctx)
{
    // Reload user info on orderbook changes
    ctx.data.subscribe<CoinbaseOrderBook>(*this);
}

void CoinbaseUserInfo::process()
{
    query();
    sleep(std::chrono::seconds(30));
}

void CoinbaseUserInfo::process(
    const CoinbaseOrderBook &book)
{
    (void)book;
    query();
}

void CoinbaseUserInfo::query()
{
    CoinbaseWallet::Data data = ctx.coinbase().getWallet();
    if (data)
    {
        ctx.data.get<CoinbaseWallet>().update(data);
        ctx.data.get<CoinbaseInit>().setWalletInit();
    }

    uint32_t fees = ctx.coinbase().getFeeTier();
    if (fees > 0)
    {
        ctx.data.get<CoinbaseFeeTier>().setFeeTier(fees);
        ctx.data.get<CoinbaseInit>().setFeeTierInit();
    }
}
