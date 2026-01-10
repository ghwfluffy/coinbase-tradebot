#include <gtb/IntegerUtils.h>

#include <openssl/bn.h>

#include <memory>
#include <cstdint>
#include <cstring>

using namespace gtb;

namespace
{

struct BN_Deleter { void operator()(BIGNUM* p)  const noexcept { BN_free(p);  } };
struct CTX_Deleter{ void operator()(BN_CTX* p) const noexcept { BN_CTX_free(p);} };

using bn_ptr  = std::unique_ptr<BIGNUM, BN_Deleter>;
using ctx_ptr = std::unique_ptr<BN_CTX,  CTX_Deleter>;

// Factory functor: bn_ptr make_bn() via operator()
struct BN_Make {
    bn_ptr operator()() const {
        if (auto* p = BN_new()) return bn_ptr(p);
        return bn_ptr(nullptr);
    }
};

// Helpers
bool bn_set_u64_native(bn_ptr &x, uint64_t v)
{
    unsigned char buf[sizeof(uint64_t)] = {};
    std::memcpy(buf, &v, sizeof(uint64_t));
    return BN_native2bn(buf, sizeof(uint64_t), x.get()) != nullptr;
}

bool bn_get_u64_native(const BIGNUM *x, uint64_t &out)
{
    if (BN_num_bits(x) > 64)
        return false;
    unsigned char buf[8] = {};
    if (BN_bn2nativepad(x, buf, 8) != 8)
        return false;
    std::memcpy(&out, buf, 8);
    return true;
}

// (A * B) / C using OpenSSL BIGNUM
bool mul_div_u64_bn(uint64_t A, uint64_t B, uint64_t C, uint64_t &out)
{
    out = 0;
    if (C == 0)
        return false;

    ctx_ptr ctx{BN_CTX_new()};
    if (!ctx)
        return false;

    BN_Make make;
    bn_ptr a = make(), b = make(), c = make(), prod = make(), q = make();
    if (!a || !b || !c || !prod || !q)
        return false;

    if (!bn_set_u64_native(a, A) ||
        !bn_set_u64_native(b, B) ||
        !bn_set_u64_native(c, C))
    {
        return false;
    }

    if (BN_is_zero(c.get()))
        return false;

    if (!BN_mul(prod.get(), a.get(), b.get(), ctx.get()))
        return false;
    if (!BN_div(q.get(), nullptr, prod.get(), c.get(), ctx.get()))
        return false;

    return bn_get_u64_native(q.get(), out);
}

}

// From $X.YY format (or X.YYYYYYYY) to usd_t
usd_t IntegerUtils::fromUsdString(
    const std::string &usd)
{
    if (usd.empty() || usd[0] == '-')
        return {};

    size_t pos = 0;
    if (usd[0] == '$')
        pos++;

    uint64_t cents = 0;
    uint64_t dollars = 0;
    try {
        dollars = std::stoull(usd.c_str() + pos);
    } catch (const std::exception &e) {}

    size_t period = usd.find('.', pos);
    if (period != std::string::npos)
    {
        if ((period + 1) < usd.length() && usd[period + 1] >= '0' && usd[period + 1] <= '9')
            cents += static_cast<uint32_t>((usd[period + 1] - '0') * 10);
        if ((period + 2) < usd.length() && usd[period + 2] >= '0' && usd[period + 2] <= '9')
            cents += static_cast<uint32_t>((usd[period + 1] - '0'));
    }

    return (1_Dollars * dollars) + (1_Cents * cents);
}

std::string IntegerUtils::toUsdString(
    usd_t picos)
{
    uint64_t dollars = picos / usd_t(1_Dollars);
    uint64_t cents = (picos - (dollars * 1_Dollars)) / usd_t(1_Cents);
    char usd[64] = {};
    snprintf(usd, sizeof(usd), "%llu.%02llu",
        static_cast<unsigned long long>(dollars),
        static_cast<unsigned long long>(cents));
    return std::string(usd);
}

std::string IntegerUtils::toUsdString(
    int64_t picos)
{
    bool negative = picos < 0;
    if (negative)
        picos *= -1L;
    std::string ret = toUsdString(usd_t(static_cast<uint64_t>(picos)));
    if (negative)
        ret = "-" + ret;
    return ret;
}

// From fractional bitcoin format (X.YYYYYYYY) to btc_t
btc_t IntegerUtils::fromBtcString(
    const std::string &btc)
{
    if (btc.empty() || btc[0] == '-')
        return {};

    size_t period = btc.find('.');
    btc_t satoshi;
    try {
        satoshi = std::stoull(btc.c_str()) * 1_Bitcoins;
    } catch (const std::exception &e) {}
    if (period != std::string::npos)
    {
        size_t pos = period + 1;
        int64_t order = 100'000'000 / 10;
        while (order > 0 && pos < btc.length())
        {
            if (btc[pos] >= '0' && btc[pos] <= '9')
                satoshi += btc_t(static_cast<uint64_t>((btc[pos] - '0') * order));
            order /= 10;
            pos++;
        }
    }

    return satoshi;
}

// Format satoshi's into fractional bitcoins
std::string IntegerUtils::toBtcString(
    btc_t satoshi)
{
    uint64_t bitcoins = satoshi / btc_t(1_Bitcoins);
    uint64_t remainder = btc_t(satoshi - (1_Bitcoins * bitcoins)).value();

    char btc[64] = {};
    snprintf(btc, sizeof(btc), "%llu.%08llu",
        static_cast<unsigned long long>(bitcoins),
        static_cast<unsigned long long>(remainder));
    return std::string(btc);
}

usd_t IntegerUtils::getValue(
    usd_t price,
    btc_t satoshi)
{
    // price/1btc = x/satoshi
    // (price/1btc) * satoshi = x
    // (price*satoshi)/1btc = x
    uint64_t uiPrice = price.value();
    uint64_t uiSatoshi = satoshi.value();
    uint64_t uiBTC = btc_t(1_Bitcoins).value();
    uint64_t value = 0;
    mul_div_u64_bn(uiPrice, uiSatoshi, uiBTC, value);
    return usd_t(value);
}

usd_t IntegerUtils::getPrice(
    usd_t value,
    btc_t satoshi)
{
    // x/1btc = value/satoshi
    // x = (value*1btc)/satoshi
    uint64_t uiValue = value.value();
    uint64_t uiSatoshi = satoshi.value();
    uint64_t uiBTC = btc_t(1_Bitcoins).value();
    uint64_t price = 0;
    mul_div_u64_bn(uiValue, uiBTC, uiSatoshi, price);
    return usd_t(price);
}

usd_t IntegerUtils::getPrice(
    const BigInt &value,
    const BigInt &satoshi)
{
    if (!value || !satoshi)
        return usd_t();

    BigInt one_btc{btc_t(1_Bitcoins).value()};
    BigInt price_bn = (value * one_btc) / satoshi;

    uint64_t price = price_bn.toUint64();
    return usd_t(price);
}

usd_t IntegerUtils::getPrice(
    const big_usd_t &value,
    const big_btc_t &satoshi)
{
    return getPrice(value.value(), satoshi.value());
}

btc_t IntegerUtils::getSatoshiForPrice(
    usd_t btcPrice,
    usd_t transactionSize)
{
    if (!btcPrice || !transactionSize)
        return {};

    // btcPrice/1 = transactionSize/x
    // btcPrice * x = transactionSize
    // x Bitcoins = transactionSize / btcPrice
    uint64_t uiT = transactionSize.value();
    uint64_t uiSatoshi = btc_t(1_Bitcoins).value();
    uint64_t uiPrice = btcPrice.value();
    uint64_t quantity = 0;
    mul_div_u64_bn(uiT, uiSatoshi, uiPrice, quantity);
    return btc_t(quantity);
}

std::string IntegerUtils::toUsdCompact(
    usd_t amount)
{
    uint64_t dollars = amount / usd_t(1_Dollars);
    if (dollars >= 1'000'000)
    {
        double millions = static_cast<double>(dollars) / 1'000'000.0;
        char buf[32] = {};
        snprintf(buf, sizeof(buf), "$%.1fM", millions);
        return std::string(buf);
    }
    if (dollars >= 1'000)
    {
        double thousands = static_cast<double>(dollars) / 1'000.0;
        char buf[32] = {};
        snprintf(buf, sizeof(buf), "$%.1fK", thousands);
        return std::string(buf);
    }

    return "$" + toUsdString(amount);
}

std::string IntegerUtils::toUsdCompact(
    big_usd_t amount)
{
    // First try the precise (picos) path; if it overflows, fall back to a dollar-only path.
    int64_t picos = 0;
    bool preciseOk = amount.value().tryToInt64(picos);
    bool neg = false;
    uint64_t dollars = 0;
    uint64_t cents = 0;
    if (preciseOk)
    {
        neg = picos < 0;
        uint64_t absPicos = neg ? static_cast<uint64_t>(-picos) : static_cast<uint64_t>(picos);
        dollars = absPicos / static_cast<uint64_t>(usd_t(1_Dollars).value());
        cents = (absPicos - (dollars * static_cast<uint64_t>(usd_t(1_Dollars).value()))) /
            static_cast<uint64_t>(usd_t(1_Cents).value());
    }
    else
    {
        BigInt denom(static_cast<uint64_t>(usd_t(1_Dollars).value()));
        BigInt dollarsBn = amount.value() / denom;
        int64_t dollarsSigned = 0;
        if (!dollarsBn.tryToInt64(dollarsSigned))
            return "$INF";
        neg = dollarsSigned < 0;
        dollars = neg ? static_cast<uint64_t>(-dollarsSigned) : static_cast<uint64_t>(dollarsSigned);
        cents = 0;
    }

    if (dollars >= 1'000'000)
    {
        double millions = static_cast<double>(dollars) / 1'000'000.0;
        char buf[32] = {};
        snprintf(buf, sizeof(buf), "%s$%.1fM", neg ? "-" : "", millions);
        return std::string(buf);
    }
    if (dollars >= 1'000)
    {
        double thousands = static_cast<double>(dollars) / 1'000.0;
        char buf[32] = {};
        snprintf(buf, sizeof(buf), "%s$%.1fK", neg ? "-" : "", thousands);
        return std::string(buf);
    }

    char usd[64] = {};
    snprintf(usd, sizeof(usd), "%s$%llu.%02llu",
        neg ? "-" : "",
        static_cast<unsigned long long>(dollars),
        static_cast<unsigned long long>(cents));
    return std::string(usd);
}

int64_t IntegerUtils::toDollars(
    big_usd_t amount)
{
    bool negative = amount.isNegative();
    if (negative)
        amount.setNegative(false);
    BigInt dollars = amount.value() / BigInt(usd_t(1_Dollars).value());
    int64_t iDollars = dollars.toInt64() * (negative ? -1 : 1);
    return iDollars;
}

int64_t IntegerUtils::toDollars(
    usd_t amount)
{
    return toDollars(big_usd_t(amount.value()));
}

usd_t IntegerUtils::makerBuyPrice(
    usd_t mid,
    usd_t offset)
{
    return mid - offset;
}

usd_t IntegerUtils::makerSellPrice(
    usd_t mid,
    usd_t offset)
{
    return mid + offset;
}
