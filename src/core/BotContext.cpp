#include <gtb/BotContext.h>
#include <gtb/Log.h>

using namespace gtb;

BotContext::BotContext()
    : data(actionPool)
{
}

CoinbaseInterface &BotContext::coinbase()
{
    if (!coinbaseIfx)
        log::error("Coinbase interface not initialized.");
    return *coinbaseIfx;
}

void BotContext::setCoinbase(std::unique_ptr<CoinbaseInterface> coinbase)
{
    this->coinbaseIfx = std::move(coinbase);
}
