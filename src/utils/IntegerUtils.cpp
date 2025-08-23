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

std::string IntegerUtils::centsToUsd(
    uint32_t cents)
{
    char usd[64] = {};
    snprintf(usd, sizeof(usd), "%u.%02u", cents / 100U, cents % 100U);
    return std::string(usd);
}

std::string IntegerUtils::centsToUsd(
    int32_t cents)
{
    std::string str;
    if (cents >= 0)
        str = centsToUsd(static_cast<uint32_t>(cents));
    else
    {
        str = centsToUsd(static_cast<uint32_t>(cents * -1));
        str.insert(0, "-");
    }

    return str;
}

std::string IntegerUtils::satoshiToBtc(
    uint64_t satoshi)
{
    char btc[64] = {};
    snprintf(btc, sizeof(btc), "%llu.%08llu", satoshi / 100'000'000ULL, satoshi % 100'000'000ULL);
    return std::string(btc);
}

uint32_t IntegerUtils::getValueCents(
    uint32_t priceCents,
    uint64_t satoshi)
{
    return static_cast<uint32_t>((satoshi * priceCents) / 100'000'000ULL);
}

uint64_t IntegerUtils::getSatoshiForPrice(
    uint32_t priceCents,
    uint32_t amountCents)
{
    if (priceCents == 0 || amountCents == 0)
        return 0;

    uint64_t satoshiCents = amountCents * 100'000'000ULL;
    // Round up
    return (satoshiCents + priceCents - 1) / priceCents;
}

uint64_t IntegerUtils::usdToPico(
    const std::string &usd)
{
    if (usd.empty() || usd[0] == '-')
        return 0;

    size_t period = usd.find('.');
    uint32_t dollars = static_cast<uint32_t>(std::stoull(usd.c_str()));
    uint64_t picos = dollars * 10'000'000'000'000ULL;
    if (period != std::string::npos)
    {
        std::string strPicos = usd.substr(period + 1);
        strPicos.resize(13, '0');
        picos += std::stoull(strPicos.c_str());
    }

    return picos;
}
