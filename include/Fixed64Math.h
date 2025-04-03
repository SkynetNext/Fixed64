#pragma once

#include <stdlib.h>

#include <array>
#include <concepts>
#include <type_traits>

#include "Fixed64.h"
#include "FixedTrigLut.h"
#include "Primitives.h"

namespace math::fp {

constexpr int kMinTrigPrecision =
    32;  // Minimum precision required for trigonometric lookup table implementation

/**
 * @brief Fixed-point number mathematical operations library
 *
 * Provides the following features:
 * - Basic trigonometric functions (only supports Q31.32 format)
 * - General mathematical operations (supports arbitrary precision)
 * - Interpolation functions (linear interpolation, angle interpolation, spherical interpolation)
 * - Numerical conversion utilities
 */
class Fixed64Math {
 public:
    template <int P>
    constexpr static auto Fractions(Fixed64<P> x) noexcept -> Fixed64<P> {
        constexpr int64_t mask = (1LL << P) - 1;
        return Fixed64<P>(x.value() & mask, detail::nothing{});
    }

    /**
     * @brief Calculate power of 2 (2^x)
     * @param x Exponent value
     * @return 2^x
     * @note Supports fixed-point numbers of any precision, automatically adjusts precision and
     * provides overflow protection
     */
    template <int P>
    [[nodiscard]] static auto Pow2(Fixed64<P> x) noexcept -> Fixed64<P> {
        // Overflow protection
        constexpr int MaxExponent = 63 - P;
        constexpr int MinExponent = -63 + P;

        if (x > Fixed64<P>(MaxExponent)) {
            return Fixed64<P>::Max();
        }
        if (x < Fixed64<P>(MinExponent)) {
            return Fixed64<P>::Zero();
        }

        // Handle zero exponent case
        if (x == Fixed64<P>::Zero()) {
            return Fixed64<P>::One();
        }

        // Handle negative exponents
        bool neg = x < Fixed64<P>::Zero();
        if (neg) {
            x = -x;
        }

        // Separate integer and fractional parts
        auto intPart = Floor(x);
        auto fracPart = x - intPart;
        int intValue = static_cast<int>(intPart);

        // Calculate 2^intValue using bit shifting
        Fixed64<P> intResult;
        if (intValue >= 0) {
            // For positive exponents, left shift
            intResult = Fixed64<P>(1LL << intValue);
        } else {
            // For negative exponents, right shift
            intResult = Fixed64<P>(Fixed64<P>::One().value() >> (-intValue), detail::nothing{});
        }

        // If there's no fractional part, return directly
        if (fracPart == Fixed64<P>::Zero()) {
            if (neg) {
                // For negative exponents, return 1/result
                return Fixed64<P>::One() / intResult;
            }
            return intResult;
        }

        // Calculate 2^fracPart using Intel's approach:
        // 2^x = e^(x*ln(2))
        // Use a minimax polynomial approximation for e^y where y = x*ln(2)
        const auto ln2 = Fixed64<P>::Ln2();
        auto y = fracPart * ln2;

        // Polynomial coefficients for e^x approximation (minimax)
        // These coefficients provide better accuracy than Taylor series
        constexpr Fixed64<P> c1(1.0);
        constexpr Fixed64<P> c2(1.0);
        constexpr Fixed64<P> c3(0.5);
        constexpr Fixed64<P> c4(0.166666666666667);
        constexpr Fixed64<P> c5(0.041666666666667);
        constexpr Fixed64<P> c6(0.008333333333333);
        constexpr Fixed64<P> c7(0.001388888888889);
        constexpr Fixed64<P> c8(0.000198412698413);

        // Calculate polynomial approximation using Horner's method
        auto fracResult = c8;
        fracResult = fracResult * y + c7;
        fracResult = fracResult * y + c6;
        fracResult = fracResult * y + c5;
        fracResult = fracResult * y + c4;
        fracResult = fracResult * y + c3;
        fracResult = fracResult * y + c2;
        fracResult = fracResult * y + c1;

        // Combine integer and fractional parts
        auto result = intResult * fracResult;

        // Handle negative exponents
        if (neg) {
            // For negative exponents, return 1/result
            return Fixed64<P>::One() / result;
        }

        return result;
    }

    /**
     * @brief Calculate power of e (e^x)
     * @param x Exponent
     * @return e^x
     *
     * Uses formula: e^x = 2^(x * log2(e))
     * where log2(e) ≈ 1.4426950408889634
     */
    template <int P>
    [[nodiscard]] static auto Exp(Fixed64<P> x) noexcept -> Fixed64<P> {
        // Approximate value of log2(e) for Exp function
        static const auto kLog2E = Fixed64<P>::Log2E();
        return Fixed64Math::Pow2(x * kLog2E);
    }

    /**
     * @brief Check if two fixed-point numbers are approximately equal
     * @param a First number
     * @param b Second number
     * @param tolerance Tolerance value
     * @return true if the difference between the two numbers is less than or equal to the tolerance
     */
    template <int P>
    [[nodiscard]] static constexpr auto IsNearlyEqual(
        Fixed64<P> a,
        Fixed64<P> b,
        Fixed64<P> tolerance = Fixed64<P>::ENotation3()) noexcept -> bool {
        return Abs(a - b) <= tolerance;
    }

    // === Basic trigonometric functions (supports Q31.32 format and above) ===

    /**
     * @brief Calculate sine value
     * @param x Angle (in radians)
     * @return Sine value [-1,1]
     */
    template <int P>
        requires(P >= kMinTrigPrecision)
    [[nodiscard]] static auto Sin(Fixed64<P> x) noexcept -> Fixed64<P> {
        if constexpr (P == kMinTrigPrecision) {
            return Fixed64<P>(FixedTrigLut::Sin(x.value()), detail::nothing{});
        } else {
            return Fixed64<P>(FixedTrigLut::Sin(x.value() >> (P - kMinTrigPrecision))
                                  << (P - kMinTrigPrecision),
                              detail::nothing{});
        }
    }

    /**
     * @brief Calculate cosine value
     * @param x Angle (in radians)
     * @return Cosine value [-1,1]
     */
    template <int P>
        requires(P >= kMinTrigPrecision)
    [[nodiscard]] static auto Cos(Fixed64<P> x) noexcept -> Fixed64<P> {
        if constexpr (P == kMinTrigPrecision) {
            return Fixed64<P>(FixedTrigLut::Cos(x.value()), detail::nothing{});
        } else {
            return Fixed64<P>(FixedTrigLut::Cos(x.value() >> (P - kMinTrigPrecision))
                                  << (P - kMinTrigPrecision),
                              detail::nothing{});
        }
    }

    /**
     * @brief Calculate tangent value
     * @param x Angle (in radians)
     * @return Tangent value
     */
    template <int P>
        requires(P >= kMinTrigPrecision)
    [[nodiscard]] static auto Tan(Fixed64<P> x) noexcept -> Fixed64<P> {
        if constexpr (P == kMinTrigPrecision) {
            return Fixed64<P>(FixedTrigLut::Tan(x.value()), detail::nothing{});
        } else {
            return Fixed64<P>(FixedTrigLut::Tan(x.value() >> (P - kMinTrigPrecision))
                                  << (P - kMinTrigPrecision),
                              detail::nothing{});
        }
    }

    /**
     * @brief Calculate arc cosine value
     * @param x Input value [-1,1]
     * @return Angle (in radians) [0,π]
     * @note Returns 0 if input is outside the [-1,1] range
     */
    template <int P>
        requires(P >= kMinTrigPrecision)
    [[nodiscard]] static auto Acos(Fixed64<P> x) noexcept -> Fixed64<P> {
        if (x > Fixed64<P>::One() || x < -Fixed64<P>::One()) {
            return Fixed64<P>::Zero();
        }

        if constexpr (P == kMinTrigPrecision) {
            return Fixed64<P>(FixedTrigLut::Acos(x.value()), detail::nothing{});
        } else {
            return Fixed64<P>(FixedTrigLut::Acos(x.value() >> (P - kMinTrigPrecision))
                                  << (P - kMinTrigPrecision),
                              detail::nothing{});
        }
    }

    /**
     * @brief Calculate arc sine value
     * @param x Input value [-1,1]
     * @return Angle (in radians) [-π/2,π/2]
     * @note Returns 0 if input is outside the [-1,1] range
     */
    template <int P>
        requires(P >= kMinTrigPrecision)
    [[nodiscard]] static auto Asin(Fixed64<P> x) noexcept -> Fixed64<P> {
        if (x > Fixed64<P>::One()) {
            return Fixed64<P>::HalfPi();
        }
        if (x < -Fixed64<P>::One()) {
            return -Fixed64<P>::HalfPi();
        }

        if constexpr (P == kMinTrigPrecision) {
            return Fixed64<P>(FixedTrigLut::Asin(x.value()), detail::nothing{});
        } else {
            return Fixed64<P>(FixedTrigLut::Asin(x.value() >> (P - kMinTrigPrecision))
                                  << (P - kMinTrigPrecision),
                              detail::nothing{});
        }
    }

    /**
     * @brief Calculate arc tangent value
     * @param x Input value
     * @return Angle (in radians) [-π/2,π/2]
     */
    template <int P>
    [[nodiscard]] static auto Atan(Fixed64<P> x) noexcept -> Fixed64<P> {
        return Atan2(x, Fixed64<P>::One());
    }

    /**
     * @brief Calculate two-argument arc tangent value
     * @param y y-coordinate
     * @param x x-coordinate
     * @return Angle (in radians) [-π,π], specifically:
     *         - When x>0, return value is in [-π/2, π/2]
     *         - When x<0, return value is in (-π, -π/2] and [π/2, π)
     *         - When x=0, y>0 returns π/2, y<0 returns -π/2, y=0 returns 0
     *
     * @note Precision characteristics:
     * - Highest precision: When |y| is much greater than |x|, relative error is approximately
     * 1e-8%~1e-9%
     * - Medium precision: When |y| is much less than |x|, relative error is in the range of
     * 1e-3%~1e-6%
     * - Lowest precision: When |y|≈|x|, relative error is approximately 1.7e-3%
     *
     * Uses optimized polynomial approximation algorithm with coefficients optimized using least
     * squares method
     */
    template <int P>
    [[nodiscard]] static auto Atan2(Fixed64<P> y, Fixed64<P> x) noexcept -> Fixed64<P> {
        // Arc tangent polynomial approximation coefficients (optimized using least squares)
        // Polynomial form: z * (P1*z¹⁰ + P2*z⁸ - P3*z⁶ + P4*z⁴ - P5*z² + P6)
        // Optimization interval: z ∈ [0, 1]
        // Reference paper keywords: "Efficient Approximations for the Arctangent Function"
        static constexpr Fixed64<P> P1(-0.013470836654772);
        static constexpr Fixed64<P> P2(0.057477314064364);
        static constexpr Fixed64<P> P3(0.121236299202148);
        static constexpr Fixed64<P> P4(0.195635939456039);
        static constexpr Fixed64<P> P5(0.332994954915642);
        static constexpr Fixed64<P> P6(1);                      // +1.0
        static constexpr Fixed64<P> P7 = Fixed64<P>::HalfPi();  // ~π/2
        static constexpr Fixed64<P> P8 = Fixed64<P>::Pi();      // ~π

        auto absX = Abs(x);
        auto absY = Abs(y);
        auto t3 = absX;
        auto t1 = absY;
        auto t0 = Max(t3, t1);
        t1 = Min(t3, t1);
        t3 = Fixed64<P>::One() / t0;
        t3 = t1 * t3;
        auto t4 = t3 * t3;
        t0 = P1;
        t0 = t0 * t4 + P2;
        t0 = t0 * t4 - P3;
        t0 = t0 * t4 + P4;
        t0 = t0 * t4 - P5;
        t0 = t0 * t4 + P6;
        t3 = t0 * t3;
        t3 = absY > absX ? P7 - t3 : t3;
        t3 = x < Fixed64<P>::Zero() ? P8 - t3 : t3;
        t3 = y < Fixed64<P>::Zero() ? -t3 : t3;

        return t3;
    }

    /**
     * @brief Calculate square root using improved Newton's method
     * @param x Input value
     * @return Square root value
     */
    template <int P>
    [[nodiscard]] constexpr static auto Sqrt(Fixed64<P> x) noexcept -> Fixed64<P> {
        return Fixed64<P>(Primitives::Fixed64SqrtFast(x.value(), P), detail::nothing{});
    }

    /**
     * @brief Floor function
     * @param x Input value
     * @return Largest integer not greater than the input
     */
    template <int P>
    [[nodiscard]] constexpr static auto Floor(Fixed64<P> x) noexcept -> Fixed64<P> {
        return Fixed64<P>(x.value() >> P << P, detail::nothing{});
    }

    /**
     * @brief Truncate fractional part (round toward zero)
     * @param x Input value
     * @return Integer value with fractional part truncated
     *
     * @note Difference from Floor:
     * - floor always rounds down: floor(-3.7) = -4
     * - trunc always rounds toward zero: trunc(-3.7) = -3
     *
     * Implementation principle:
     * - Directly clear the fractional bits (through bitwise AND operation)
     * - No need to consider sign, as the two's complement representation of negative numbers
     * naturally rounds toward zero
     */
    template <int P>
    [[nodiscard]] constexpr static auto Trunc(Fixed64<P> x) noexcept -> Fixed64<P> {
        int64_t value = x.value();
        int64_t fractionalMask = (1LL << P) - 1;

        if (value >= 0) {
            return Fixed64<P>(value & ~fractionalMask, detail::nothing{});
        } else {
            int64_t fractionalPart = value & fractionalMask;
            if (fractionalPart != 0) {
                return Fixed64<P>((value & ~fractionalMask) + (1LL << P), detail::nothing{});
            } else {
                return x;
            }
        }
    }

    /**
     * @brief Ceiling function
     * @param x Input value
     * @return Smallest integer not less than the input
     */
    template <int P>
    [[nodiscard]] constexpr static auto Ceil(Fixed64<P> x) noexcept -> Fixed64<P> {
        int64_t Fractions = x.value() & ((1LL << P) - 1);

        if (Fractions == 0) {
            return x;
        }

        int64_t result = x.value() >> P << P;
        result += 1LL << P;

        return Fixed64<P>(result, detail::nothing{});
    }

    /**
     * @brief Round to nearest integer
     * @param x Input value
     * @return Integer closest to the input
     */
    template <int P>
    [[nodiscard]] constexpr static auto Round(Fixed64<P> x) noexcept -> Fixed64<P> {
        int64_t fraction = x.value() & ((1LL << P) - 1);
        int64_t half = 1LL << (P - 1);

        if (fraction >= half) {
            return Fixed64<P>((x.value() >> P) + 1 << P, detail::nothing{});
        }

        return Fixed64<P>(x.value() >> P << P, detail::nothing{});
    }

    /**
     * @brief Return the smaller of two values
     */
    template <typename T1, typename T2>
    [[nodiscard]] constexpr static auto Min(const T1& a, const T2& b) noexcept
        -> detail::CompareType<T1, T2>::type {
        return a < b ? static_cast<detail::CompareType<T1, T2>::type>(a)
                     : static_cast<detail::CompareType<T1, T2>::type>(b);
    }

    /**
     * @brief Return the greater of two values
     */
    template <typename T1, typename T2>
    [[nodiscard]] constexpr static auto Max(const T1& a, const T2& b) noexcept
        -> detail::CompareType<T1, T2>::type {
        return a > b ? static_cast<detail::CompareType<T1, T2>::type>(a)
                     : static_cast<detail::CompareType<T1, T2>::type>(b);
    }

    /**
     * @brief Calculate absolute value
     */
    template <int P>
    [[nodiscard]] constexpr static auto Abs(Fixed64<P> x) noexcept -> Fixed64<P> {
        if (x.value() < 0) {
            return Fixed64<P>(~x.value() + 1, detail::nothing{});
        }
        return x;
    }

    /**
     * @brief Convert floating-point number to fixed-point, with range checking
     * @param x Input floating-point number
     * @return Converted fixed-point number
     */
    template <typename ToT, typename FromT>
        requires(::std::is_arithmetic_v<FromT>) && detail::IsFixed64<ToT>
    [[nodiscard]] static constexpr auto ClampedCast(FromT x) noexcept -> ToT {
        if (x >= static_cast<FromT>(ToT::Max())) {
            return ToT::Max();
        }
        if (x <= static_cast<FromT>(ToT::Min())) {
            return ToT::Min();
        }

        return static_cast<ToT>(x);
    }

    /**
     * @brief Clamp value within specified range
     * @param x Input value
     * @param min Minimum value
     * @param max Maximum value
     * @return Value clamped to [min,max] range
     */
    template <int P>
    [[nodiscard]] static auto Clamp(Fixed64<P> x, Fixed64<P> min, Fixed64<P> max) noexcept
        -> Fixed64<P> {
        if (x.value() < min.value())
            return min;
        if (x.value() > max.value())
            return max;
        return x;
    }

    /**
     * @brief Clamp value to [0,1] range
     * @param x Input value
     * @return Value clamped to [0,1] range
     */
    template <int P>
    [[nodiscard]] static auto Clamp01(Fixed64<P> x) noexcept -> Fixed64<P> {
        if (x < Fixed64<P>::Zero())
            return Fixed64<P>::Zero();
        if (x > Fixed64<P>::One())
            return Fixed64<P>::One();
        return x;
    }

    /**
     * @brief Linear interpolation
     * @param from Start value
     * @param to Target value
     * @param t Interpolation factor [0,1]
     * @return Interpolated result
     */
    template <int P>
    [[nodiscard]] static auto Lerp(Fixed64<P> from, Fixed64<P> to, Fixed64<P> t) noexcept
        -> Fixed64<P> {
        t = Clamp01(t);
        return from + (to - from) * t;
    }

    /**
     * @brief Linear interpolation without clamping the interpolation factor
     * @param from Start value
     * @param to Target value
     * @param t Interpolation factor
     * @return Interpolated result
     */
    template <int P>
    [[nodiscard]] static auto LerpUnclamped(Fixed64<P> from, Fixed64<P> to, Fixed64<P> t) noexcept
        -> Fixed64<P> {
        return from + (to - from) * t;
    }

    /**
     * @brief Calculate inverse linear interpolation
     * @param a Range start value
     * @param b Range end value
     * @param x Current value
     * @return Position of current value within range [0,1]
     */
    template <int P>
    [[nodiscard]] static auto InverseLerp(Fixed64<P> a, Fixed64<P> b, Fixed64<P> x) noexcept
        -> Fixed64<P> {
        if (a != b) {
            return Clamp01((x - a) / (b - a));
        }
        return Fixed64<P>::Zero();
    }

    /**
     * @brief Repeat a value within specified range
     * @param x Input value
     * @param length Range length
     * @return Mapped value [0,length)
     */
    template <int P>
    [[nodiscard]] static auto Repeat(Fixed64<P> x, Fixed64<P> length) noexcept -> Fixed64<P> {
        return Clamp(x - Floor(x / length) * length, Fixed64<P>::Zero(), length);
    }

    /**
     * @brief Linear interpolation between two angles, automatically choosing shortest path
     * @param start Start angle (radians)
     * @param end Target angle (radians)
     * @param t Interpolation factor [0,1]
     * @return Interpolated angle (radians)
     */
    template <int P>
    [[nodiscard]] static auto LerpAngle(Fixed64<P> start, Fixed64<P> end, Fixed64<P> t) noexcept
        -> Fixed64<P> {
        auto diff = end - start;

        if (diff > Fixed64<P>::Pi()) {
            diff -= Fixed64<P>::TwoPi();
        } else if (diff < -Fixed64<P>::Pi()) {
            diff += Fixed64<P>::TwoPi();
        }

        return start + diff * Clamp01(t);
    }

    template <int P>
    [[nodiscard]] constexpr static auto Sign(Fixed64<P> x) noexcept -> int {
        return x.value() < 0 ? -1 : 1;
    }

    /**
     * @brief Normalize angle to [0, 2π) range
     * @param angle Input angle (radians)
     * @return Normalized angle in [0, 2π) range
     */
    template <int P>
    static auto NormalizeAngle(Fixed64<P> angle) noexcept -> Fixed64<P> {
        const auto twoPi = Fixed64<P>::TwoPi();
        auto raw = angle.value();

        // Handle negative angles
        while (raw < 0) {
            raw += twoPi.value();
        }

        // Restrict to [0, 2π) range
        raw %= twoPi.value();

        return Fixed64<P>(raw, detail::nothing{});
    }
};
}  // namespace math::fp

namespace std {
// Trigonometric function support (requires P >= kMinTrigPrecision)
template <int P>
    requires(P >= ::math::fp::kMinTrigPrecision)
inline auto sin(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Sin(x);
}

template <int P>
    requires(P >= ::math::fp::kMinTrigPrecision)
inline auto cos(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Cos(x);
}

template <int P>
    requires(P >= ::math::fp::kMinTrigPrecision)
inline auto tan(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Tan(x);
}

template <int P>
    requires(P >= ::math::fp::kMinTrigPrecision)
inline auto asin(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Asin(x);
}

template <int P>
    requires(P >= ::math::fp::kMinTrigPrecision)
inline auto acos(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Acos(x);
}

template <int P>
inline auto atan(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Atan(x);
}

template <int P>
inline auto atan2(const ::math::fp::Fixed64<P>& y, const ::math::fp::Fixed64<P>& x) noexcept
    -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Atan2(y, x);
}

// Exponential and logarithmic functions
template <int P>
inline auto exp(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Exp(x);
}

template <int P>
inline auto sqrt(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Sqrt(x);
}

// Absolute value
template <int P>
inline auto abs(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Abs(x);
}

template <int P>
inline auto fabs(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return abs(x);
}

// Rounding functions
template <int P>
inline auto floor(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Floor(x);
}

template <int P>
inline auto trunc(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Trunc(x);
}

template <int P>
inline auto ceil(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Ceil(x);
}

template <int P>
inline auto round(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Round(x);
}
}  // namespace std
