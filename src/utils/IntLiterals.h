#pragma once

#include <gtb/StrongTypedInt.h>

namespace gtb
{
    // Satoshi
    struct btc_tag {};
    using btc_t = StrongTypedInt<uint64_t, btc_tag>;

    // Decipicodollars
    struct usd_tag {};
    using usd_t = StrongTypedInt<uint64_t, usd_tag>;

    // Microseconds
    struct utime_tag {};
    using utime_t = StrongTypedInt<uint64_t, utime_tag>;

    // Percentage points (1/100 of a percent)
    struct pp_tag  {};
    using pp_t = StrongTypedInt<uint32_t, pp_tag>;

    // cents → decipicodollars
    constexpr usd_t operator"" _Cents(unsigned long long v) {
        return usd_t(v * 100'000'000'000ULL);
    }

    // dollars → decipicodollars
    constexpr usd_t operator"" _Dollars(unsigned long long v) {
        return usd_t(v * 10'000'000'000'000ULL);
    }

    // 1/10^13 of a dollar
    constexpr usd_t operator"" _Decipicodollars(unsigned long long v) {
        return usd_t(v);
    }

    // seconds → microseconds
    constexpr utime_t operator"" _Seconds(unsigned long long v) {
        return utime_t(v * 1'000'000ULL);
    }

    // minutes → microseconds
    constexpr utime_t operator"" _Minutes(unsigned long long v) {
        return utime_t(v * 60'000'000ULL);
    }

    // hours → microseconds
    constexpr utime_t operator"" _Hours(unsigned long long v) {
        return utime_t(v * 60ULL * 60'000'000ULL);
    }

    constexpr utime_t operator"" _Microseconds(unsigned long long v) {
        return utime_t(v);
    }

    // bitcoins → satoshi
    constexpr btc_t operator"" _Bitcoins(unsigned long long v) {
        return btc_t(v * 100'000'000ULL);
    }

    constexpr btc_t operator"" _Satoshi(unsigned long long v) {
        return btc_t(v);
    }

    // percent → percentage points
    constexpr pp_t operator"" _Percent(unsigned long long v) {
        return pp_t(static_cast<uint32_t>(v * 100ULL));
    }

    // 1/100 of a percent
    constexpr pp_t operator"" _PercentagePoints(unsigned long long v) {
        return pp_t(static_cast<uint32_t>(v));
    }

    // Allow multiplying by a pp_t
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
    constexpr uint32_t PP_SCALE = 10'000U; // 100%
    template<class Rep, class Tag>
    [[nodiscard]] constexpr StrongTypedInt<Rep, Tag> scale_by_pp(StrongTypedInt<Rep, Tag> x, pp_t r)
    {
        if constexpr (std::is_same_v<Rep, uint64_t> || std::is_same_v<Rep, int64_t>)
        {
#if defined(__SIZEOF_INT128__)
            using wide = unsigned __int128;
            wide prod = static_cast<wide>(x.value()) * static_cast<wide>(r.value());
            return StrongTypedInt<Rep, Tag>(static_cast<Rep>(prod / PP_SCALE));
#else
            // Fallback: do the division first when possible to reduce overflow risk.
            // (Truncation intended; adjust if you need rounding.)
            return StrongTypedInt<Rep, Tag>(static_cast<Rep>((x.value() / PP_SCALE) * r.value() +
                                                             (x.value() % PP_SCALE) * r.value() / PP_SCALE));
#endif
        }

        // 32-bit reps, etc.
        return StrongTypedInt<Rep, Tag>(static_cast<Rep>((static_cast<uint64_t>(x.value()) *
                                                          static_cast<uint64_t>(r.value())) / PP_SCALE));
    }

    // T * pp_t
    template<class Rep, class Tag>
    [[nodiscard]] constexpr StrongTypedInt<Rep, Tag> operator*(StrongTypedInt<Rep, Tag> x, pp_t r)
    {
        return scale_by_pp(x, r);
    }

    // pp_t * T
    template<class Rep, class Tag>
    [[nodiscard]] constexpr StrongTypedInt<Rep, Tag> operator*(pp_t r, StrongTypedInt<Rep, Tag> x)
    {
        return scale_by_pp(x, r);
    }

    // pp_t * pp_t
    [[nodiscard]] constexpr pp_t operator*(pp_t r, pp_t x)
    {
        return scale_by_pp(r, x);
    }

    // Allow dividing by pp_t
    template <class Rep, class Tag>
    [[nodiscard]] constexpr StrongTypedInt<Rep, Tag> operator/(StrongTypedInt<Rep, Tag> x, pp_t r)
    {
        if constexpr (std::is_same_v<Rep, std::uint64_t> || std::is_same_v<Rep, std::int64_t>)
        {
#if defined(__SIZEOF_INT128__)
            using wide = unsigned __int128;
            wide num   = static_cast<wide>(x.value()) * PP_SCALE;
            return StrongTypedInt<Rep, Tag>(static_cast<Rep>(num / r.value()));
#else
            // Fallback without __int128: may overflow for huge x.
            return StrongTypedInt<Rep, Tag>(static_cast<Rep>(
                (static_cast<std::uint64_t>(x.value()) * PP_SCALE) / r.value()));
#endif
        }

        return StrongTypedInt<Rep, Tag>(static_cast<Rep>(
            (static_cast<std::uint64_t>(x.value()) * PP_SCALE) / r.value()));
    }
    #pragma GCC diagnostic pop
}
