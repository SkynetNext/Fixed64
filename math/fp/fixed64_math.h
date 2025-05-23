#pragma once

#include <stdlib.h>

#include <array>
#include <concepts>
#include <type_traits>

#include "detail/acos_lut.h"
#include "detail/atan2_lut.h"
#include "detail/atan_lut.h"
#include "detail/sin_lut.h"
#include "detail/tan_lut.h"
#include "fixed64.h"
#include "primitives.h"

// Configuration macros for trigonometric function precision
#ifndef FIXED64_MATH_USE_FAST_TRIG
#define FIXED64_MATH_USE_FAST_TRIG 1  // Default to fast implementation
#endif

namespace math::fp {

constexpr int kTrigFractionBits =
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

        // Calculate 2^x = e^(x*ln(2))
        constexpr auto ln2 = Fixed64<P>::Ln2();
        auto y = fracPart * ln2;

        // Use more terms for better precision while maintaining efficiency with Estrin's algorithm
        constexpr Fixed64<P> c1(1);                                              // x^0
        constexpr Fixed64<P> c2(1);                                              // x^1
        constexpr Fixed64<P> c3 = Fixed64<P>::One() / 2;                         // x^2
        constexpr Fixed64<P> c4(0x0, 0x1555555555556157ULL, detail::nothing{});  // x^3
        constexpr Fixed64<P> c5(0x0, 0x0555555555556157ULL, detail::nothing{});  // x^4
        constexpr Fixed64<P> c6(0x0, 0x011111111111050eULL, detail::nothing{});  // x^5
        constexpr Fixed64<P> c7(0x0, 0x002d82d82d82dc2eULL, detail::nothing{});  // x^6

        // Calculate polynomial using improved Estrin's algorithm
        auto y2 = y * y;
        auto y4 = y2 * y2;

        // Compute coefficient groups in parallel
        auto p01 = c1 + y * c2;
        auto p23 = c3 + y * c4;
        auto p45 = c5 + y * c6;
        auto p6 = c7;

        // Combine results - properly using the pre-computed y4
        auto fracResult = p01 + y2 * p23 + y4 * (p45 + y2 * p6);

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
     * @brief Calculate natural logarithm (ln)
     * @param x Input value (must be positive)
     * @return Natural logarithm value
     */
    template <int P>
    [[nodiscard]] static auto Log(Fixed64<P> x) noexcept -> Fixed64<P> {
        // Handle special cases
        if (x <= Fixed64<P>::Zero()) {
            return Fixed64<P>::Min();  // Return minimum value to indicate error
        }

        if (x == Fixed64<P>::One()) {
            return Fixed64<P>::Zero();
        }

        // 1. Find the position of the most significant bit
        const uint64_t u_x = static_cast<uint64_t>(x.value());
        const int msb = 63 - Primitives::CountlZero(u_x);

        // Calculate exponent (similar to IEEE-754 format)
        const int exp = msb - P;

        // 2. Normalize x to [1,2) range by shifting
        int64_t normalized_x;
        if (msb >= P) {
            // Right shift to normalize
            normalized_x = x.value() >> (msb - P);
        } else {
            // Left shift to normalize
            normalized_x = x.value() << (P - msb);
        }

        // Convert to Fixed64 for calculation
        auto scaled_x = Fixed64<P>(normalized_x, detail::nothing{});

        // 3. Calculate z = (a-1)/(a+1), where a is the scaled value
        auto z = (scaled_x - Fixed64<P>::One()) / (scaled_x + Fixed64<P>::One());
        auto z2 = z * z;

        // 4. Use polynomial approximation for ln(x), based on Padé approximation
        // ln(x) ≈ 2z(1 + z²/3 + z⁴/5 + z⁶/7 + ...)
        constexpr Fixed64<P> c1(2);
        constexpr Fixed64<P> c3 = Fixed64<P>(2) / 3;
        constexpr Fixed64<P> c5 = Fixed64<P>(2) / 5;
        constexpr Fixed64<P> c7 = Fixed64<P>(2) / 7;
        constexpr Fixed64<P> c9 = Fixed64<P>(2) / 9;

        auto result = c9;
        result = result * z2 + c7;
        result = result * z2 + c5;
        result = result * z2 + c3;
        result = result * z2 + c1;
        result = result * z;

        // 5. Add exp*ln(2) to account for the normalization
        result = result + Fixed64<P>(exp) * Fixed64<P>::Ln2();

        return result;
    }

    /**
     * @brief Calculate power function (x^y)
     * @param x Base value (must be positive for non-integer exponents)
     * @param y Exponent value
     * @return x raised to the power of y
     */
    template <int P>
    [[nodiscard]] static auto Pow(Fixed64<P> x, Fixed64<P> y) noexcept -> Fixed64<P> {
        // Handle special cases
        if (x <= Fixed64<P>::Zero()) {
            // Only calculate when y is an integer and x is negative
            if (y == Floor(y)) {
                // If y is even, result is |x|^y
                // If y is odd, result is -|x|^y
                bool isYEven = (static_cast<int>(y) % 2 == 0);
                auto absX = Abs(x);
                auto result = Exp(y * Log(absX));
                return isYEven ? result : -result;
            }
            return Fixed64<P>::Zero();  // Undefined for negative base with non-integer exponent
        }

        if (x == Fixed64<P>::One()) {
            return Fixed64<P>::One();
        }

        if (y == Fixed64<P>::Zero()) {
            return Fixed64<P>::One();
        }

        if (y == Fixed64<P>::One()) {
            return x;
        }

        // Use logarithm: x^y = e^(y*ln(x))
        return Exp(y * Log(x));
    }

    /**
     * @brief Calculate power function with unsigned integer exponent (x^u)
     * @param x Base value
     * @param u Unsigned integer exponent
     * @return x^u
     * @note Uses binary exponentiation algorithm for efficient computation
     */
    template <int P, typename UnsignedIntegralType>
        requires std::unsigned_integral<UnsignedIntegralType>
    [[nodiscard]] static auto Pow(Fixed64<P> x, UnsignedIntegralType u) noexcept -> Fixed64<P> {
        // Handle special cases
        if (u == static_cast<UnsignedIntegralType>(0)) {
            return Fixed64<P>::One();
        } else if (u == static_cast<UnsignedIntegralType>(1)) {
            return x;
        } else if (u == static_cast<UnsignedIntegralType>(2)) {
            return x * x;
        } else {
            Fixed64<P> result = Fixed64<P>::One();
            Fixed64<P> base = x;

            // Use binary exponentiation algorithm
            for (auto p_local = static_cast<std::uint32_t>(u);
                 p_local != static_cast<std::uint32_t>(0);
                 p_local = static_cast<std::uint32_t>(p_local >> 1)) {
                if ((p_local & 1) != 0) {
                    result *= base;
                }

                base *= base;
            }

            return result;
        }
    }

    /**
     * @brief Calculate power function with signed integer exponent (x^n)
     * @param x Base value
     * @param n Signed integer exponent
     * @return x^n
     * @note Handles negative exponents by computing reciprocal of positive exponent result
     */
    template <int P, typename SignedIntegralType>
        requires std::signed_integral<SignedIntegralType>
    [[nodiscard]] static auto Pow(Fixed64<P> x, SignedIntegralType n) noexcept -> Fixed64<P> {
        // Handle special cases
        if (n < static_cast<SignedIntegralType>(0)) {
            return Fixed64<P>::One()
                   / Pow(x, static_cast<typename std::make_unsigned<SignedIntegralType>::type>(-n));
        } else if (n == static_cast<SignedIntegralType>(0)) {
            return Fixed64<P>::One();
        } else if (n == static_cast<SignedIntegralType>(1)) {
            return x;
        } else if (n == static_cast<SignedIntegralType>(2)) {
            return x * x;
        } else {
            Fixed64<P> result = Fixed64<P>::One();
            Fixed64<P> base = x;

            // Use binary exponentiation algorithm
            for (auto p_local = static_cast<std::uint32_t>(n);
                 p_local != static_cast<std::uint32_t>(0);
                 p_local = static_cast<std::uint32_t>(p_local >> 1)) {
                if ((p_local & 1) != 0) {
                    result *= base;
                }

                base *= base;
            }

            return result;
        }
    }

    /**
     * @brief Calculate power of e (e^x) using improved algorithm
     * @param x Exponent
     * @return e^x
     */
    template <int P>
    [[nodiscard]] static auto Exp(Fixed64<P> x) noexcept -> Fixed64<P> {
        // Handle overflow cases
        if (x > Fixed64<P>(30)) {
            return Fixed64<P>::Max();
        }

        if (x < Fixed64<P>(-30)) {
            return Fixed64<P>::Zero();
        }

        // Split x into integer and fractional parts
        auto intPart = Floor(x);
        auto fracPart = x - intPart;
        int n = static_cast<int>(intPart);

        // Use Padé approximation to calculate e^fracPart
        // Using (6,6) Padé approximation: e^x ≈ (1 + x/2 + x²/10 + x³/120) / (1 - x/2 + x²/10 -
        // x³/120)
        auto x2 = fracPart * fracPart;
        auto x3 = x2 * fracPart;

        constexpr Fixed64<P> c1(1);
        constexpr Fixed64<P> c2 = Fixed64<P>::Half();
        constexpr Fixed64<P> c3 = Fixed64<P>::Point1();
        constexpr Fixed64<P> c4 = Fixed64<P>::One() / 120;

        auto num = c1 + fracPart * c2 + x2 * c3 + x3 * c4;
        auto den = c1 - fracPart * c2 + x2 * c3 - x3 * c4;

        auto fracResult = num / den;

        // Calculate e^n
        Fixed64<P> intResult = Fixed64<P>::One();
        Fixed64<P> e = Fixed64<P>::E();

        // Use binary exponentiation algorithm to calculate e^n
        if (n >= 0) {
            while (n > 0) {
                if (n & 1) {
                    intResult = intResult * e;
                }
                e = e * e;
                n >>= 1;
            }
        } else {
            n = -n;
            while (n > 0) {
                if (n & 1) {
                    intResult = intResult / e;
                }
                e = e * e;
                n >>= 1;
            }
        }

        return intResult * fracResult;
    }

    /**
     * @brief Determines if two values are nearly equal within a tolerance
     * @param a First value
     * @param b Second value
     * @param tolerance Maximum allowed difference (defaults to a small multiple of epsilon)
     * @return True if the values are considered equal within tolerance
     *
     * Industry standard approach using both absolute and relative error metrics.
     * This handles both large and small values appropriately.
     */
    template <int P>
    [[nodiscard]] static bool IsNearlyEqual(Fixed64<P> a,
                                            Fixed64<P> b,
                                            Fixed64<P> tolerance = Fixed64<P>::Epsilon() * 4) {
        // Get absolute difference
        auto abs_diff = Abs(a - b);

        // Direct comparison for exact equality
        if (abs_diff == Fixed64<P>::Zero()) {
            return true;
        }

        // For very small values or values close to zero, use absolute tolerance
        auto abs_a = Abs(a);
        auto abs_b = Abs(b);
        auto max_magnitude = Max(abs_a, abs_b);

        if (max_magnitude < Fixed64<P>::One()) {
            // For small values, use absolute comparison
            return abs_diff <= tolerance;
        } else {
            // For larger values, use relative comparison
            // This scales the tolerance with the magnitude of the values
            return abs_diff <= max_magnitude * tolerance;
        }
    }

    // === Basic trigonometric functions (supports Q31.32 format and above) ===

    /**
     * @brief Calculate sine value
     * @param x Angle (in radians)
     * @return Sine value [-1,1]
     */
    template <int P>
        requires(P >= kTrigFractionBits)
    [[nodiscard]] static auto Sin(Fixed64<P> x) noexcept -> Fixed64<P> {
        if constexpr (FIXED64_MATH_USE_FAST_TRIG) {
            return Fixed64<P>(detail::LookupSinFast(x.value(), P), detail::nothing{});
        } else {
            return Fixed64<P>(detail::LookupSin(x.value(), P), detail::nothing{});
        }
    }

    /**
     * @brief Calculate cosine value
     * @param x Angle (in radians)
     * @return Cosine value [-1,1]
     */
    template <int P>
        requires(P >= kTrigFractionBits)
    [[nodiscard]] static auto Cos(Fixed64<P> x) noexcept -> Fixed64<P> {
        return Sin(x + Fixed64<P>::HalfPi());
    }

    /**
     * @brief Calculate tangent value
     * @param x Angle (in radians)
     * @return Tangent value
     */
    template <int P>
        requires(P >= kTrigFractionBits)
    [[nodiscard]] static auto Tan(Fixed64<P> x) noexcept -> Fixed64<P> {
        if constexpr (FIXED64_MATH_USE_FAST_TRIG) {
            return Fixed64<P>(detail::LookupTanFast(x.value(), P), detail::nothing{});
        } else {
            return Fixed64<P>(detail::LookupTan(x.value(), P), detail::nothing{});
        }
    }

    /**
     * @brief Calculate arc cosine value
     * @param x Input value [-1,1]
     * @return Angle (in radians) [0,π]
     * @note For values outside [-1,1]: returns 0 if x>1, returns π if x<-1
     */
    template <int P>
        requires(P >= kTrigFractionBits)
    [[nodiscard]] static auto Acos(Fixed64<P> x) noexcept -> Fixed64<P> {
        if (x > Fixed64<P>::One()) {
            return Fixed64<P>::Zero();
        }
        if (x < -Fixed64<P>::One()) {
            return Fixed64<P>::Pi();
        }

        return Fixed64<P>(detail::LookupAcos(x.value(), P), detail::nothing{});
    }

    /**
     * @brief Calculate arc sine value
     * @param x Input value [-1,1]
     * @return Angle (in radians) [-π/2,π/2]
     * @note For values outside [-1,1]: returns π/2 if x>1, returns -π/2 if x<-1
     */
    template <int P>
        requires(P >= kTrigFractionBits)
    [[nodiscard]] static auto Asin(Fixed64<P> x) noexcept -> Fixed64<P> {
        if (x > Fixed64<P>::One()) {
            return Fixed64<P>::HalfPi();
        }
        if (x < -Fixed64<P>::One()) {
            return -Fixed64<P>::HalfPi();
        }

        // Use the identity: asin(x) = π/2 - acos(x)
        return Fixed64<P>::HalfPi() - Acos(x);
    }

    /**
     * @brief Calculate arctangent using lookup table with linear interpolation
     *
     * @tparam P Precision of fixed-point number
     * @param x Input value
     * @return Fixed64<P> Arctangent result in radians
     */
    template <int P>
    static auto Atan(Fixed64<P> x) noexcept -> Fixed64<P> {
        if constexpr (FIXED64_MATH_USE_FAST_TRIG) {
            return Fixed64<P>(detail::LookupAtanFast(x.value(), P), detail::nothing{});
        } else {
            return Fixed64<P>(detail::LookupAtan(x.value(), P), detail::nothing{});
        }
    }

    /**
     * @brief Computes two-argument arctangent with quadrant determination
     *
     * @param y Y-coordinate component
     * @param x X-coordinate component
     * @return Angle in radians in range [-π,π]
     *
     * @note Special cases: atan2(0,0)=0, atan2(±y,0)=±π/2, atan2(0,±x)=0 or ±π.
     * Precision limited by 256-entry LUT with linear interpolation.
     */
    template <int P>
    [[nodiscard]] static auto Atan2(Fixed64<P> y, Fixed64<P> x) noexcept -> Fixed64<P> {
        // Handle special cases
        if (x == Fixed64<P>::Zero() && y == Fixed64<P>::Zero()) {
            return Fixed64<P>::Zero();  // Undefined case, return 0 by convention
        }

        if (x == Fixed64<P>::Zero()) {
            return y > Fixed64<P>::Zero() ? Fixed64<P>::HalfPi() : -Fixed64<P>::HalfPi();
        }

        // Determine octant and compute ratio in first octant [0, π/4]
        bool x_neg = x < Fixed64<P>::Zero();
        bool y_neg = y < Fixed64<P>::Zero();
        bool swapped = Abs(y) > Abs(x);

        // Calculate ratio (always in [0,1] range)
        Fixed64<P> ratio;
        if (swapped) {
            ratio = Abs(x) / Abs(y);
        } else {
            ratio = Abs(y) / Abs(x);
        }

        // Use lookup table with linear interpolation
        Fixed64<P> angle = Fixed64<P>(detail::LookupAtan2(ratio.value(), P), detail::nothing{});

        // Apply octant correction
        if (swapped) {
            angle = Fixed64<P>::HalfPi() - angle;
        }

        // Apply quadrant correction
        if (x_neg && y_neg) {
            return -Fixed64<P>::Pi() + angle;  // Third quadrant
        } else if (x_neg) {
            return Fixed64<P>::Pi() - angle;  // Second quadrant
        } else if (y_neg) {
            return -angle;  // Fourth quadrant
        } else {
            return angle;  // First quadrant
        }
    }

    /**
     * @brief Computes square root using optimized algorithm
     *
     * @param x Input value (must be non-negative)
     * @return Square root of x
     *
     * @note Returns 0 for negative inputs. Maximum absolute error limited by
     * Fixed64<P>'s resolution (2^-P).
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
     * @brief Round to nearest integer (round-half-away-from-zero)
     * @param x Input value
     * @return Integer closest to the input, with ties rounded away from zero
     * @note This matches the behavior of C++11's std::round
     */
    template <int P>
    [[nodiscard]] constexpr static auto Round(Fixed64<P> x) noexcept -> Fixed64<P> {
        constexpr int64_t kHalf = 1LL << (P - 1);
        constexpr int64_t kFractionMask = (1LL << P) - 1;

        // Get sign and absolute value
        int64_t value = x.value();
        bool is_negative = value < 0;

        // Use bit operations to get absolute value, avoiding INT64_MIN issue
        uint64_t abs_value;
        if (is_negative) {
            abs_value = ~static_cast<uint64_t>(value) + 1;  // Two's complement
        } else {
            abs_value = static_cast<uint64_t>(value);
        }

        // Process absolute value
        uint64_t fraction = abs_value & kFractionMask;

        // Round half away from zero
        uint64_t result_value;
        if (fraction >= kHalf) {
            result_value = ((abs_value >> P) + 1) << P;
        } else {
            result_value = abs_value >> P << P;
        }

        // Restore sign
        if (is_negative) {
            return Fixed64<P>(~result_value + 1, detail::nothing{});
        } else {
            return Fixed64<P>(result_value, detail::nothing{});
        }
    }

    /**
     * @brief Return the smaller of two values
     */
    template <typename T1, typename T2>
    [[nodiscard]] constexpr static auto Min(const T1& a, const T2& b) noexcept ->
        typename detail::CompareType<T1, T2>::type {
        return a < b ? static_cast<typename detail::CompareType<T1, T2>::type>(a)
                     : static_cast<typename detail::CompareType<T1, T2>::type>(b);
    }

    /**
     * @brief Return the greater of two values
     */
    template <typename T1, typename T2>
    [[nodiscard]] constexpr static auto Max(const T1& a, const T2& b) noexcept ->
        typename detail::CompareType<T1, T2>::type {
        return a > b ? static_cast<typename detail::CompareType<T1, T2>::type>(a)
                     : static_cast<typename detail::CompareType<T1, T2>::type>(b);
    }

    /**
     * @brief Calculate absolute value
     */
    template <int P>
    [[nodiscard]] constexpr static auto Abs(Fixed64<P> x) noexcept -> Fixed64<P> {
        if (x.value() < 0) {
            // Cast to unsigned, perform negation (well-defined wrap-around),
            // then cast back to signed for the constructor.
            return Fixed64<P>(static_cast<int64_t>(~static_cast<uint64_t>(x.value()) + 1),
                              detail::nothing{});
        }
        return x;
    }

    /**
     * @brief Convert floating-point number to fixed-point, with range checking
     * @param x Input floating-point number
     * @return Converted fixed-point number
     */
    template <typename DestType, typename SourceType>
        requires(::std::is_arithmetic_v<SourceType>) && detail::IsFixed64<DestType>
    [[nodiscard]] static constexpr auto ClampedCast(SourceType x) noexcept -> DestType {
        if (x >= static_cast<SourceType>(DestType::Max())) {
            return DestType::Max();
        }
        if (x <= static_cast<SourceType>(DestType::Min())) {
            return DestType::Min();
        }

        return static_cast<DestType>(x);
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
     * @return Position of current value within range [0,1] if x is between a and b,
     *         or proportional value outside this range if x is outside [a,b]
     *
     * This function calculates the relative position of `x` between `a` and `b`.
     * If `x` equals `a`, the result is 0.
     * If `x` equals `b`, the result is 1.
     * If `x` is between `a` and `b`, the result is between 0 and 1.
     * If `x` is less than `a`, the result will be less than 0.
     * If `x` is greater than `b`, the result will be greater than 1.
     * If `a` equals `b`, the function returns 0.5 to avoid division by zero.
     */
    template <int P>
    [[nodiscard]] static auto InverseLerp(Fixed64<P> a, Fixed64<P> b, Fixed64<P> x) noexcept
        -> Fixed64<P> {
        if (IsNearlyEqual(a, b)) {
            return Fixed64<P>::Half();  // Avoid division by zero
        }
        return (x - a) / (b - a);
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

    /**
     * @brief Returns the sign of a value
     * @param x Input value
     * @return -1 if negative, 0 if zero, 1 if positive
     */
    template <int P>
    [[nodiscard]] constexpr static auto Sign(Fixed64<P> x) noexcept -> int {
        if (x.value() < 0)
            return -1;
        if (x.value() > 0)
            return 1;
        return 0;  // x is exactly zero
    }

    /**
     * @brief Normalize angle to [0, 2π) range
     * @param angle Input angle (radians)
     * @return Normalized angle in [0, 2π) range
     *
     * Implementation follows game industry best practices for efficient and accurate angle
     * normalization.
     */
    template <int P>
    static auto NormalizeAngle(Fixed64<P> angle) noexcept -> Fixed64<P> {
        constexpr auto kTwoPi = Fixed64<P>::TwoPi();

        // Fast path for common case - already in range
        if (angle >= Fixed64<P>::Zero() && angle < kTwoPi) {
            return angle;
        }

        // Optimized implementation to handle both positive and negative angles
        // Avoid division when possible, use incremental adjustment for small values
        if (angle >= -kTwoPi && angle < kTwoPi) {
            // For angles close to range, simple addition is more precise
            if (angle < Fixed64<P>::Zero()) {
                return angle + kTwoPi;
            }
            return angle;  // Already in range
        }

        // For larger out-of-range values, use modulo approach
        // But implement it in a more numerically stable way

        // Get the fractional part (this is more precise than division+multiplication)
        // Use repeated subtraction or addition for better precision
        auto result = angle;

        if (result >= kTwoPi) {
            // Optimize for common case of small multiples of 2π
            while (result >= kTwoPi) {
                result -= kTwoPi;
            }
        } else if (result < Fixed64<P>::Zero()) {
            // Optimize for common case of small negative angles
            while (result < Fixed64<P>::Zero()) {
                result += kTwoPi;
            }
        }

        // Final range check to guarantee [0, 2π) range due to potential precision issues
        if (result >= kTwoPi) {
            result = Fixed64<P>::Zero();  // Handle precision edge case
        }

        return result;
    }
};
}  // namespace math::fp

namespace std {
// Trigonometric function support (requires P >= kTrigFractionBits)
template <int P>
    requires(P >= ::math::fp::kTrigFractionBits)
inline auto sin(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Sin(x);
}

template <int P>
    requires(P >= ::math::fp::kTrigFractionBits)
inline auto cos(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Cos(x);
}

template <int P>
    requires(P >= ::math::fp::kTrigFractionBits)
inline auto tan(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Tan(x);
}

template <int P>
    requires(P >= ::math::fp::kTrigFractionBits)
inline auto asin(const ::math::fp::Fixed64<P>& x) noexcept -> ::math::fp::Fixed64<P> {
    return ::math::fp::Fixed64Math::Asin(x);
}

template <int P>
    requires(P >= ::math::fp::kTrigFractionBits)
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
