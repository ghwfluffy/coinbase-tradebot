#include <gtb/IntegerUtils.h>

using namespace gtb;

uint32_t IntegerUtils::usdToCents(
    const std::string &usd)
{
    if (usd.empty() || usd[0] == '-')
        return 0;

    size_t period = usd.find('.');
    uint32_t cents = static_cast<uint32_t>(std::stoull(usd.c_str()) * 100);
    if (period != std::string::npos)
    {
        if ((period + 1) < usd.length() && usd[period + 1] >= '0' && usd[period + 1] <= '9')
            cents += static_cast<uint32_t>((usd[period + 1] - '0') * 10);
        if ((period + 2) < usd.length() && usd[period + 2] >= '0' && usd[period + 2] <= '9')
            cents += static_cast<uint32_t>((usd[period + 1] - '0'));
    }

    return cents;
}

uint64_t IntegerUtils::btcToSatoshi(
    const std::string &btc)
{
    if (btc.empty() || btc[0] == '-')
        return 0;

    size_t period = btc.find('.');
    uint64_t satoshi = static_cast<uint64_t>(std::stoull(btc.c_str()) * 100'000'000ULL);
    if (period != std::string::npos)
    {
        size_t pos = period + 1;
        int64_t order = 100'000'000 / 10;
        while (order > 0 && pos < btc.length())
        {
            if (btc[pos] >= '0' && btc[pos] <= '9')
                satoshi += static_cast<uint64_t>((btc[pos] - '0') * order);
            order /= 10;
            pos++;
        }
    }

    return satoshi;
}
