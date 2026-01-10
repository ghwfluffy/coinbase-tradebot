#pragma once

#include <gtb/StrongTypedInt.h>
#include <gtb/BigInt.h>

#include <cstdint>
#include <utility>

namespace gtb
{

/**
 * Strongly-typed BigInt wrapper keyed by Tag (compatible with StrongTypedInt of the same Tag).
 */
template<class Tag>
struct StrongTypedBigInt
{
    using tag = Tag;

    StrongTypedBigInt() = default;
    explicit StrongTypedBigInt(BigInt val) : v(std::move(val)) {}
    explicit StrongTypedBigInt(uint64_t val) : v(val) {}

    template<class Rep>
    explicit StrongTypedBigInt(StrongTypedInt<Rep, Tag> x)
        : v(static_cast<uint64_t>(x.value()))
    {
    }

    BigInt &value() { return v; }
    const BigInt &value() const { return v; }

    bool isNegative() const { return v.isNegative(); }
    void setNegative(bool negative) { v.setNegative(negative); }

    explicit operator bool() const { return static_cast<bool>(v); }

    template<class Rep>
    StrongTypedBigInt &operator=(StrongTypedInt<Rep, Tag> o)
    {
        v = BigInt(static_cast<uint64_t>(o.value()));
        return *this;
    }

    // Comparisons (only same Tag)
    friend bool operator==(const StrongTypedBigInt &a, const StrongTypedBigInt &b) { return a.v == b.v; }
    friend bool operator!=(const StrongTypedBigInt &a, const StrongTypedBigInt &b) { return !(a == b); }
    friend bool operator<(const StrongTypedBigInt &a, const StrongTypedBigInt &b) { return a.v < b.v; }
    friend bool operator>(const StrongTypedBigInt &a, const StrongTypedBigInt &b) { return b < a; }
    friend bool operator<=(const StrongTypedBigInt &a, const StrongTypedBigInt &b) { return !(b < a); }
    friend bool operator>=(const StrongTypedBigInt &a, const StrongTypedBigInt &b) { return !(a < b); }
    template<class Rep>
    friend bool operator==(const StrongTypedBigInt &a, const StrongTypedInt<Rep, Tag> &b) { return a.v == BigInt(b.value()); }

    // Arithmetic with matching BigInt types
    StrongTypedBigInt &operator+=(const StrongTypedBigInt &o)
    {
        v += o.v;
        return *this;
    }
    StrongTypedBigInt &operator-=(const StrongTypedBigInt &o)
    {
        v -= o.v;
        return *this;
    }

    // Arithmetic with matching small strong ints
    template<class Rep>
    StrongTypedBigInt &operator+=(StrongTypedInt<Rep, Tag> o)
    {
        v += BigInt(static_cast<uint64_t>(o.value()));
        return *this;
    }
    template<class Rep>
    StrongTypedBigInt &operator-=(StrongTypedInt<Rep, Tag> o)
    {
        v -= BigInt(static_cast<uint64_t>(o.value()));
        return *this;
    }

    friend StrongTypedBigInt operator+(StrongTypedBigInt a, const StrongTypedBigInt &b)
    {
        a += b;
        return a;
    }
    friend StrongTypedBigInt operator-(StrongTypedBigInt a, const StrongTypedBigInt &b)
    {
        a -= b;
        return a;
    }

    template<class Rep>
    friend StrongTypedBigInt operator+(StrongTypedBigInt a, StrongTypedInt<Rep, Tag> b)
    {
        a += b;
        return a;
    }
    template<class Rep>
    friend StrongTypedBigInt operator+(StrongTypedInt<Rep, Tag> b, StrongTypedBigInt a)
    {
        a += b;
        return a;
    }
    template<class Rep>
    friend StrongTypedBigInt operator-(StrongTypedBigInt a, StrongTypedInt<Rep, Tag> b)
    {
        a -= b;
        return a;
    }

private:
    BigInt v{};
};

}
