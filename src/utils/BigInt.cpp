#include <gtb/BigInt.h>
#include <gtb/Log.h>

#include <stdexcept>
#include <limits>

using namespace gtb;

BigInt::BigInt(uint64_t val)
    : bn(nullptr)
{
    bn = BN_new();
    BN_set_word(bn, static_cast<BN_ULONG>(val));
}

BigInt::BigInt(BigInt &&other)
    : bn(other.bn)
{
    other.bn = nullptr;
}

BigInt::BigInt(const BigInt &other)
    : bn(nullptr)
{
    if (!other.bn)
    {
        bn = nullptr;
        return;
    }

    bn = BN_dup(other.bn);
}

BigInt &BigInt::operator=(BigInt &&other)
{
    if (this != &other)
    {
        if (bn)
            BN_free(bn);

        bn = other.bn;
        other.bn = nullptr;
    }
    return *this;
}

BigInt &BigInt::operator=(const BigInt &other)
{
    if (this != &other)
    {
        if (!other.bn)
        {
            if (bn)
            {
                BN_free(bn);
                bn = nullptr;
            }
            return *this;
        }

        BIGNUM *tmp = BN_dup(other.bn);
        if (bn)
            BN_free(bn);

        bn = tmp;
    }
    return *this;
}

BigInt::~BigInt()
{
    if (bn)
        BN_free(bn);
}

BigInt::operator bool() const
{
    return bn && !BN_is_zero(bn);
}

uint64_t BigInt::to_uint64() const
{
    if (!bn)
        return 0;

    const int bits = BN_num_bits(bn);
    if (bits < 0)
    {
        log::error("BN_num_bits failed");
        return 0;
    }
    if (bits > 64)
    {
        log::error("BigNum does not fit into uint64_t");
        return 0;
    }

    unsigned char buf[8] = {0};
    const int rc = BN_bn2binpad(bn, buf, sizeof(buf));
    if (rc != sizeof(buf))
        log::error("BN_bn2binpad failed");

    uint64_t v = 0;
    for (unsigned char b : buf)
        v = (v << 8) | static_cast<uint64_t>(b);
    return v;
}

BigInt &BigInt::operator+=(const BigInt &rhs)
{
    if (BN_add(bn, bn, rhs.bn) != 1)
        log::error("BN_add failed");
    return *this;
}

BigInt &BigInt::operator-=(const BigInt &rhs)
{
    if (BN_sub(bn, bn, rhs.bn) != 1)
        log::error("BN_sub failed");
    return *this;
}

BigInt &BigInt::operator*=(const BigInt &rhs)
{
    BN_CTX *ctx = BN_CTX_new();
    if (BN_mul(bn, bn, rhs.bn, ctx) != 1)
        log::error("BN_mul failed");

    BN_CTX_free(ctx);
    return *this;
}

BigInt &BigInt::operator/=(const BigInt &rhs)
{
    if (BN_is_zero(rhs.bn))
        throw std::domain_error{"division by zero"};

    BN_CTX *ctx = BN_CTX_new();

    // BN_div: q = this / rhs, r ignored
    if (BN_div(bn, nullptr, bn, rhs.bn, ctx) != 1)
        log::error("BN_div failed");

    BN_CTX_free(ctx);
    return *this;
}

namespace gtb
{

BigInt operator+(BigInt lhs, const BigInt &rhs)
{
    lhs += rhs;
    return lhs;
}

BigInt operator-(BigInt lhs, const BigInt &rhs)
{
    lhs -= rhs;
    return lhs;
}

BigInt operator*(BigInt lhs, const BigInt &rhs)
{
    lhs *= rhs;
    return lhs;
}

BigInt operator/(BigInt lhs, const BigInt &rhs)
{
    lhs /= rhs;
    return lhs;
}

bool operator==(const BigInt &lhs, const BigInt &rhs)
{
    // BN_cmp returns 0 if equal
    return BN_cmp(lhs.bn, rhs.bn) == 0;
}

bool operator!=(const BigInt &lhs, const BigInt &rhs)
{
    return !(lhs == rhs);
}

bool operator<(const BigInt &lhs, const BigInt &rhs)
{
    return BN_cmp(lhs.bn, rhs.bn) < 0;
}

bool operator>(const BigInt &lhs, const BigInt &rhs)
{
    return BN_cmp(lhs.bn, rhs.bn) > 0;
}

bool operator<=(const BigInt &lhs, const BigInt &rhs)
{
    return BN_cmp(lhs.bn, rhs.bn) <= 0;
}

bool operator>=(const BigInt &lhs, const BigInt &rhs)
{
    return BN_cmp(lhs.bn, rhs.bn) >= 0;
}

}
