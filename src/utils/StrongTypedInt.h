#pragma once

#include <cstdint>
#include <type_traits>
#include <compare>

namespace gtb
{

/**
 * Typedef an integer type into a new unique type
 */
template<class Rep, class Tag>
struct StrongTypedInt
{
    using rep = Rep;

    constexpr StrongTypedInt() = default;
    constexpr explicit StrongTypedInt(Rep x) : v{x} {}

    // Accessor (explicit on purpose)
    constexpr Rep value() const
    {
        return v;
    }

    // Comparisons (only same StrongTypedInt type)
    constexpr auto operator<=>(const StrongTypedInt &) const = default;

    constexpr explicit operator bool() const
    {
        return v != 0;
    }

    // Same-type arithmetic
    friend constexpr StrongTypedInt operator+(StrongTypedInt a, StrongTypedInt b)
    {
        return StrongTypedInt{Rep(a.v + b.v)};
    }
    friend constexpr StrongTypedInt operator-(StrongTypedInt a, StrongTypedInt b)
    {
        return StrongTypedInt{Rep(a.v - b.v)};
    }
    constexpr StrongTypedInt &operator+=(StrongTypedInt o)
    {
        v = Rep(v + o.v);
        return *this;
    }
    constexpr StrongTypedInt &operator-=(StrongTypedInt o)
    {
        v = Rep(v - o.v);
        return *this;
    }

    // Scale by integral (unit * scalar and scalar * unit)
    template <class I> requires std::is_integral_v<I>
    friend constexpr StrongTypedInt operator*(StrongTypedInt a, I k)
    {
        return StrongTypedInt{Rep(a.v * static_cast<Rep>(k))};
    }
    template <class I> requires std::is_integral_v<I>
    friend constexpr StrongTypedInt operator*(I k, StrongTypedInt a)
    {
        return StrongTypedInt{Rep(static_cast<Rep>(k) * a.v)};
    }
    template <class I> requires std::is_integral_v<I>
    friend constexpr StrongTypedInt operator/(StrongTypedInt a, I k)
    {
        return StrongTypedInt{Rep(a.v / static_cast<Rep>(k))};
    }

    // Ratio (unit / unit -> scalar of underlying type)
    friend constexpr Rep operator/(StrongTypedInt a, StrongTypedInt b)
    {
        return Rep(a.v / b.v);
    }

    private:
    Rep v{};
};

}
