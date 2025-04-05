#pragma once

#include <inttypes.h>
#include <algorithm>  // Include header for std::clamp
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>

#include "Fixed64TypeTraits.h"
#include "Primitives.h"

/**
 * @brief 64-bit fixed-point number type providing deterministic cross-platform
 * numerical computation
 * @tparam P Number of fractional bits, determining the fixed-point format
 * Q(63-P).P
 *
 * Features:
 * - Deterministic computation: Bit-level precise cross-platform consistency
 * - High performance: Optimized bit operations for floating-point conversions
 * - Automatic precision conversion: Safe conversion between fixed-point numbers
 * of different precision
 *
 * Implementation details:
 * - Uses 64-bit signed integer for storage
 * - Format is Q(63-P).P, supporting one sign bit and 63 value bits
 * - Fractional part occupies P bits, integer part occupies (63-P) bits
 *
 * Performance characteristics:
 * - Avoids indeterminism associated with floating-point operations
 * - Precise bit-level operations ensure deterministic and cross-platform
 * consistent floating-point conversions
 * - Compile-time constant computation support
 *
 */
namespace math::fp {
template <typename T>
struct Fixed64CastTraits;

namespace detail {
using namespace math::fp::fixed64_traits;  // Import type traits

struct nothing {};
}  // namespace detail

template <int P>
class Fixed64 {
 public:
    using value_type = int64_t;

    static constexpr int64_t kRawOne = int64_t(1) << P;  // Scaling factor, maps 1 to 2^P
    static constexpr int kFractionBits = P;              // Number of fractional bits

    // Basic constructors
    constexpr Fixed64() noexcept : value_(0) {}

    // Construct from raw value (internal use only)
    explicit constexpr Fixed64(int64_t raw_value, detail::nothing) noexcept : value_(raw_value) {}

    template <int Q>
    explicit constexpr Fixed64(const Fixed64<Q>& other) noexcept {
        constexpr int SHIFT = P > Q ? P - Q : Q - P;
        value_ = P > Q ? other.value() << SHIFT : other.value() >> SHIFT;
    }

    // Copy operations
    constexpr Fixed64(const Fixed64&) noexcept = default;
    constexpr Fixed64& operator=(const Fixed64&) noexcept = default;

    // Move operations
    constexpr Fixed64(Fixed64&&) noexcept = default;
    constexpr Fixed64& operator=(Fixed64&&) noexcept = default;

    // Construct from integer
    template <typename IntType>
        requires std::is_integral_v<IntType>
    explicit constexpr Fixed64(IntType i) noexcept : value_(static_cast<int64_t>(i) << P) {}

    // Convert to integer type
    template <typename IntType>
        requires std::is_integral_v<IntType>
    explicit constexpr operator IntType() const noexcept {
        return static_cast<IntType>(value_ >> P);
    }

    // Construct from floating-point
    template <typename FloatType>
        requires std::is_floating_point_v<FloatType>
    explicit constexpr Fixed64(FloatType f) noexcept {
        if constexpr (std::is_same_v<FloatType, float>) {
            value_ = Primitives::F32ToFixed64(f, P);
        } else {
            value_ = Primitives::F64ToFixed64(f, P);
        }
    }

    // Convert to floating-point
    template <typename FloatType>
        requires std::is_floating_point_v<FloatType>
    explicit constexpr operator FloatType() const noexcept {
        if constexpr (std::is_same_v<FloatType, float>) {
            return Primitives::Fixed64ToF32(value_, P);
        } else {
            return Primitives::Fixed64ToF64(value_, P);
        }
    }

    // Generic conversion constructor - only for custom types
    template <typename T>
        requires(detail::IsCustomConstructible<T>)
    explicit constexpr Fixed64(const T& v) noexcept
        : value_(Fixed64CastTraits<std::remove_cvref_t<T>>::template ToFixed64<P>(v)) {}

    // Convert to other types
    template <typename T>
        requires(detail::IsCustomConstructible<T>)
    explicit constexpr operator T() const noexcept {
        return Fixed64CastTraits<std::remove_cvref_t<T>>::template FromFixed64<P>(value_);
    }

    // Get raw value
    [[nodiscard]] constexpr auto value() const noexcept -> int64_t {
        return value_;
    }

    // Prefix increment/decrement operators
    constexpr auto operator++() noexcept -> Fixed64<P>& {
        value_ += kRawOne;
        return *this;
    }

    constexpr auto operator--() noexcept -> Fixed64<P>& {
        value_ -= kRawOne;
        return *this;
    }

    // Postfix increment/decrement operators
    constexpr auto operator++(int) noexcept -> Fixed64<P> {
        Fixed64<P> temp(*this);
        value_ += kRawOne;
        return temp;
    }

    constexpr auto operator--(int) noexcept -> Fixed64<P> {
        Fixed64<P> temp(*this);
        value_ -= kRawOne;
        return temp;
    }

    // Unary operators
    constexpr auto operator+() const noexcept -> Fixed64<P> {
        return *this;
    }

    constexpr auto operator-() const noexcept -> Fixed64<P> {
        return Fixed64<P>(-value_, detail::nothing{});
    }

    // Basic constants
    static constexpr auto Max() noexcept -> Fixed64<P> {
        return Fixed64<P>(INT64_MAX, detail::nothing{});
    }

    static constexpr auto Min() noexcept -> Fixed64<P> {
        return Fixed64<P>(INT64_MIN, detail::nothing{});
    }

    // Integer constants
    static constexpr auto Zero() noexcept -> Fixed64<P> {
        return Fixed64<P>(0);
    }

    static constexpr auto One() noexcept -> Fixed64<P> {
        return Fixed64<P>(1);
    }

    static constexpr auto Two() noexcept -> Fixed64<P> {
        return Fixed64<P>(2);
    }

    static constexpr auto Hundred() noexcept -> Fixed64<P> {
        return Fixed64<P>(100);
    }

    static constexpr auto Thousand() noexcept -> Fixed64<P> {
        return Fixed64<P>(1000);
    }

    static constexpr auto TenThousand() noexcept -> Fixed64<P> {
        return Fixed64<P>(10000);
    }

    // Fractional constants
    static constexpr auto Point1() noexcept -> Fixed64<P> {
        return One() / 10;
    }

    static constexpr auto Quarter() noexcept -> Fixed64<P> {
        return One() / 4;
    }

    static constexpr auto Half() noexcept -> Fixed64<P> {
        return One() / 2;
    }

    // Special constants
    static constexpr auto NegOne() noexcept -> Fixed64<P> {
        return Fixed64<P>(-1);
    }

    static constexpr auto Pi() noexcept -> Fixed64<P> {
        // Pi value (π * 2^63)
        constexpr int128_t PI_BITS =
            (static_cast<int128_t>(0x1921FB544ULL) << 32) | static_cast<uint64_t>(0x42D1846AULL);
        constexpr int shift = 63 - P;
        constexpr int128_t rounding = (shift > 0) ? (int128_t(1) << (shift - 1)) : 0;
        return Fixed64<P>(static_cast<int64_t>((PI_BITS + rounding) >> shift), detail::nothing{});
    }

    static constexpr auto TwoPi() noexcept -> Fixed64<P> {
        // 2*Pi value (2π * 2^63)
        constexpr int128_t TWO_PI_BITS =
            (static_cast<int128_t>(0x3243F6A88ULL) << 32) | static_cast<uint64_t>(0x85A308D3ULL);
        constexpr int shift = 63 - P;
        constexpr int128_t rounding = (shift > 0) ? (int128_t(1) << (shift - 1)) : 0;
        return Fixed64<P>(static_cast<int64_t>((TWO_PI_BITS + rounding) >> shift),
                          detail::nothing{});
    }

    static constexpr auto HalfPi() noexcept -> Fixed64<P> {
        // Pi/2 value (π/2 * 2^63)
        constexpr int128_t HALF_PI_BITS =
            (static_cast<int128_t>(0xC90FDAA2ULL) << 32) | static_cast<uint64_t>(0x2168C235ULL);
        constexpr int shift = 63 - P;
        constexpr int128_t rounding = (shift > 0) ? (int128_t(1) << (shift - 1)) : 0;
        return Fixed64<P>(static_cast<int64_t>((HALF_PI_BITS + rounding) >> shift),
                          detail::nothing{});
    }

    static constexpr auto QuarterPi() noexcept -> Fixed64<P> {
        // Pi/4 value (π/4 * 2^63)
        constexpr int128_t QUARTER_PI_BITS =
            (static_cast<int128_t>(0x6487ED51ULL) << 32) | static_cast<uint64_t>(0x10B4611AULL);
        constexpr int shift = 63 - P;
        constexpr int128_t rounding = (shift > 0) ? (int128_t(1) << (shift - 1)) : 0;
        return Fixed64<P>(static_cast<int64_t>((QUARTER_PI_BITS + rounding) >> shift),
                          detail::nothing{});
    }

    static constexpr auto InvPi() noexcept -> Fixed64<P> {
        // 1/Pi value (1/π * 2^63)
        constexpr int128_t INV_PI_BITS =
            (static_cast<int128_t>(0x28BE60DBULL) << 32) | static_cast<uint64_t>(0x9391054AULL);
        constexpr int shift = 63 - P;
        constexpr int128_t rounding = (shift > 0) ? (int128_t(1) << (shift - 1)) : 0;
        return Fixed64<P>(static_cast<int64_t>((INV_PI_BITS + rounding) >> shift),
                          detail::nothing{});
    }

    static constexpr auto E() noexcept -> Fixed64<P> {
        // e value (e * 2^63)
        constexpr int128_t E_BITS =
            (static_cast<int128_t>(0x15BF0A8B1ULL) << 32) | static_cast<uint64_t>(0x45769535ULL);
        constexpr int shift = 63 - P;
        constexpr int128_t rounding = (shift > 0) ? (int128_t(1) << (shift - 1)) : 0;
        return Fixed64<P>(static_cast<int64_t>((E_BITS + rounding) >> shift), detail::nothing{});
    }

    static constexpr auto Ln2() noexcept -> Fixed64<P> {
        // ln(2) value (ln(2) * 2^63)
        constexpr int128_t LN2_BITS =
            (static_cast<int128_t>(0x58B90BFBULL) << 32) | static_cast<uint64_t>(0xE8E7BCD6ULL);
        constexpr int shift = 63 - P;
        constexpr int128_t rounding = (shift > 0) ? (int128_t(1) << (shift - 1)) : 0;
        return Fixed64<P>(static_cast<int64_t>((LN2_BITS + rounding) >> shift), detail::nothing{});
    }

    static constexpr auto Log2E() noexcept -> Fixed64<P> {
        // log2(e) value (log2(e) * 2^63)
        constexpr int128_t LOG2E_BITS =
            (static_cast<int128_t>(0xB8AA3B29ULL) << 32) | static_cast<uint64_t>(0x5C17F0BCULL);
        constexpr int shift = 63 - P;
        constexpr int128_t rounding = (shift > 0) ? (int128_t(1) << (shift - 1)) : 0;
        return Fixed64<P>(static_cast<int64_t>((LOG2E_BITS + rounding) >> shift),
                          detail::nothing{});
    }

    // log10(2) constant
    static constexpr auto Log10Of2() noexcept -> Fixed64<P> {
        // log10(2) value (log10(2) * 2^63)
        constexpr int128_t LOG10_OF_2_BITS =
            (static_cast<int128_t>(0x268826A1ULL) << 32) | static_cast<uint64_t>(0x3EF3FDE6ULL);
        constexpr int shift = 63 - P;
        constexpr int128_t rounding = (shift > 0) ? (int128_t(1) << (shift - 1)) : 0;
        return Fixed64<P>(static_cast<int64_t>((LOG10_OF_2_BITS + rounding) >> shift),
                          detail::nothing{});
    }

    // Angle conversion constants
    static constexpr auto Deg2Rad() noexcept -> Fixed64<P> {
        // Degrees to radians conversion factor (π/180 * 2^63)
        constexpr int128_t DEG_TO_RAD_BITS =
            (static_cast<int128_t>(0x23BE8D4ULL) << 32) | static_cast<uint64_t>(0x4A53A723ULL);
        constexpr int shift = 63 - P;
        constexpr int128_t rounding = (shift > 0) ? (int128_t(1) << (shift - 1)) : 0;
        return Fixed64<P>(static_cast<int64_t>((DEG_TO_RAD_BITS + rounding) >> shift),
                          detail::nothing{});
    }

    static constexpr auto Rad2Deg() noexcept -> Fixed64<P> {
        // Radians to degrees conversion factor (180/π * 2^63)
        constexpr int128_t RAD_TO_DEG_BITS =
            (static_cast<int128_t>(0x1CA5DC1A63ULL) << 32) | static_cast<uint64_t>(0xC1F7B861ULL);
        constexpr int shift = 63 - P;
        constexpr int128_t rounding = (shift > 0) ? (int128_t(1) << (shift - 1)) : 0;
        return Fixed64<P>(static_cast<int64_t>((RAD_TO_DEG_BITS + rounding) >> shift),
                          detail::nothing{});
    }

    // Special values
    static constexpr auto Epsilon() noexcept -> Fixed64<P> {
        return Fixed64<P>(1LL, detail::nothing{});
    }

    static constexpr auto NaN() noexcept -> Fixed64<P> {
        return Min();
    }

    static constexpr auto Infinity() noexcept -> Fixed64<P> {
        return Max();
    }

    static constexpr auto NegInfinity() noexcept -> Fixed64<P> {
        return Fixed64<P>(INT64_MIN + 1, detail::nothing{});
    }

    // ENotation constants
    static constexpr auto ENotation1() noexcept -> Fixed64<P> {
        return One() / 10;  // 0.1
    }

    static constexpr auto ENotation2() noexcept -> Fixed64<P> {
        return One() / 100;  // 0.01
    }

    static constexpr auto ENotation3() noexcept -> Fixed64<P> {
        return One() / 1000;  // 0.001
    }

    static constexpr auto ENotation4() noexcept -> Fixed64<P> {
        return One() / 10000;  // 0.0001
    }

    // region String conversion interfaces
    // Convert to string (high precision version)
    // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays,
    // cppcoreguidelines-pro-bounds-pointer-arithmetic)
    [[nodiscard]] auto ToString() const noexcept -> std::string {
        char buffer[64];
        char* ptr = buffer;

        // Handle sign
        if (value_ < 0) {
            *ptr++ = '-';
        }

        // Get integer and fractional parts
        int64_t absValue = std::abs(value_);
        int64_t intPart = absValue >> P;
        uint64_t fracPart = absValue & ((1LL << P) - 1);

        // Handle integer part
        if (intPart == 0) {
            *ptr++ = '0';
        } else {
            int64_t temp = intPart;
            char* start = ptr;
            while (temp > 0) {
                *ptr++ = '0' + (temp % 10);
                temp /= 10;
            }
            std::reverse(start, ptr);
        }

        // Handle fractional part
        if (P > 0) {
            *ptr++ = '.';

            // Use fixed-point constants to calculate decimal places
            // Using Log10Of2 constant to ensure cross-platform consistency
            constexpr int DECIMAL_PLACES =
                static_cast<int>(static_cast<int64_t>(P) * Log10Of2().value() >> P) + 2;

            // Calculate scale
            constexpr auto pow10 = [](int n) constexpr -> int128_t {
                int128_t result = 1;
                for (int i = 0; i < n; ++i) {
                    result *= 10;
                }
                return result;
            };
            constexpr int128_t scale = pow10(DECIMAL_PLACES);

            // Use 128-bit integer for precise calculation
            int64_t scaled = (static_cast<int128_t>(fracPart) * scale) >> P;
            // Format fractional part
            char decimalBuffer[32];
            int decimalLength = sprintf(decimalBuffer, "%0*" PRId64, DECIMAL_PLACES, scaled);

            // Remove trailing zeros, but keep at least one digit
            while (decimalLength > 1 && decimalBuffer[decimalLength - 1] == '0') {
                decimalLength--;
            }

            // Copy to final buffer
            for (int i = 0; i < decimalLength; i++) {
                *ptr++ = decimalBuffer[i];
            }
        }

        *ptr = '\0';
        return std::string(buffer);
    }

    /**
     * Parse a fixed-point number from string, using pure integer operations
     * to ensure cross-platform consistency
     * Implementation based on LLVM libc's Simple Decimal Conversion algorithm
     */
    [[nodiscard]] static auto FromString(const std::string& str) noexcept -> Fixed64<P> {
        // Handle empty string
        if (str.empty()) {
            return Fixed64<P>();
        }

        const char* src = str.c_str();
        size_t index = 0;

        // Skip leading whitespace
        while (src[index] && isspace(static_cast<unsigned char>(src[index]))) {
            ++index;
        }

        // Handle sign
        bool is_negative = false;
        if (src[index] == '-') {
            is_negative = true;
            ++index;
        } else if (src[index] == '+') {
            ++index;
        }

        // Parse integer part
        int64_t mantissa = 0;
        bool seen_digit = false;
        bool after_decimal = false;
        int32_t decimal_exponent = 0;

        // Safe integer limit: prevent overflow in multiplication operations
        constexpr int64_t MAX_SAFE_INT = INT64_MAX / 10;

        // Define minimum allowed value for decimal_exponent
        // Use existing Log10Of2 for precise binary-to-decimal conversion
        constexpr int32_t MIN_SAFE_DECIMAL_EXPONENT =
            -static_cast<int32_t>((static_cast<int64_t>(P) * Log10Of2().value()) >> P) - 1;

        // Parse numeric part (integer + fractional)
        while (true) {
            if (src[index] && isdigit(static_cast<unsigned char>(src[index]))) {
                int digit = src[index] - '0';
                seen_digit = true;

                // Check if decimal_exponent is already below the minimum allowed value
                if (!after_decimal || decimal_exponent >= MIN_SAFE_DECIMAL_EXPONENT) {
                    // Prevent integer overflow
                    if (mantissa < MAX_SAFE_INT) {
                        mantissa = mantissa * 10 + digit;
                        if (after_decimal) {
                            --decimal_exponent;
                        }
                    } else if (!after_decimal) {
                        // Integer part exceeds representable range, convert to exponent
                        // adjustment
                        ++decimal_exponent;
                    }
                }

                ++index;
                continue;
            }

            if (src[index] == '.') {
                if (after_decimal) {
                    break;  // Second decimal point, end parsing
                }
                after_decimal = true;
                ++index;
                continue;
            }

            break;  // Non-digit, non-decimal point character - end numeric parsing
        }

        if (!seen_digit) {
            return Fixed64<P>();  // No valid digits found
        }

        // Handle scientific notation exponent
        if (src[index] && (tolower(src[index]) == 'e')) {
            ++index;
            int32_t exp_value = 0;
            bool exp_negative = false;

            if (src[index] == '-') {
                exp_negative = true;
                ++index;
            } else if (src[index] == '+') {
                ++index;
            }

            bool valid_exp = false;
            while (src[index] && isdigit(static_cast<unsigned char>(src[index]))) {
                valid_exp = true;
                // Only process exponents within reasonable range to prevent overflow
                if (exp_value < 10000) {
                    exp_value = exp_value * 10 + (src[index] - '0');
                }
                ++index;
            }

            if (valid_exp) {
                decimal_exponent += exp_negative ? -exp_value : exp_value;
            }
        }

        // 1. First left shift mantissa by P bits to construct basic fixed-point
        // representation
        int64_t result = 0;

        while (mantissa > (INT64_MAX >> P)) {
            // Due to MIN_SAFE_DECIMAL_EXPONENT, mantissa can exceed the safe range by multiple
            // digits
            // We need to divide by 10 repeatedly until it fits, adjusting decimal_exponent
            // accordingly
            int64_t remainder = mantissa % 10;
            mantissa /= 10;
            if (remainder >= 5) {
                mantissa += 1;  // Round up if remainder is 5 or greater
            }
            decimal_exponent += 1;
        }

        result = mantissa << P;

        // 2. Handle decimal exponent (decimal_exponent)
        if (decimal_exponent != 0) {
            // Calculate 10^|decimal_exponent|
            int64_t power_of_10 = 1;
            int32_t abs_exp = decimal_exponent < 0 ? -decimal_exponent : decimal_exponent;

            for (int i = 0; i < abs_exp && power_of_10 <= INT64_MAX / 10; ++i) {
                power_of_10 *= 10;
            }

            if (decimal_exponent > 0) {
                // Positive exponent: multiply by 10^decimal_exponent
                if (result <= INT64_MAX / power_of_10) {
                    result *= power_of_10;
                } else {
                    // Overflow handling
                    return is_negative ? Fixed64<P>(INT64_MIN, detail::nothing{})
                                       : Fixed64<P>(INT64_MAX, detail::nothing{});
                }
            } else {
                // Negative exponent: divide by 10^|decimal_exponent| with rounding
                int64_t remainder = result % power_of_10;
                result /= power_of_10;

                // Round to nearest
                if (remainder >= power_of_10 / 2) {
                    result += 1;
                }
            }
        }

        // Apply sign
        if (is_negative && result > 0) {
            result = -result;
        }

        return Fixed64<P>(result, detail::nothing{});
    }

    // NOLINTEND

    // Stream output support
    friend std::ostream& operator<<(std::ostream& os, const Fixed64& num) noexcept {
        return os << num.ToString();
    }

    // Stream input support
    friend std::istream& operator>>(std::istream& is, Fixed64& num) noexcept {
        std::string str;
        is >> str;
        num = FromString(str);
        return is;
    }

    // endregion

 private:
    int64_t value_;

    // Declare all operators as friends
    template <int Q, int R>
    friend constexpr auto operator+=(Fixed64<Q>&, const Fixed64<R>&) noexcept -> Fixed64<Q>&;

    template <int Q, int R>
    friend constexpr auto operator-=(Fixed64<Q>&, const Fixed64<R>&) noexcept -> Fixed64<Q>&;

    template <int Q, int R>
    friend constexpr auto operator*=(Fixed64<Q>&, const Fixed64<R>&) noexcept -> Fixed64<Q>&;

    template <int Q, typename IntType>
        requires std::is_integral_v<IntType>
    friend constexpr auto operator*=(Fixed64<Q>&, const IntType&) noexcept -> Fixed64<Q>&;

    template <int Q, int R>
    friend constexpr auto operator/=(Fixed64<Q>&, const Fixed64<R>&) noexcept -> Fixed64<Q>&;

    template <int Q, typename IntType>
        requires std::is_integral_v<IntType>
    friend constexpr auto operator/=(Fixed64<Q>& a, const IntType& b) noexcept -> Fixed64<Q>&;

    template <int Q, int R>
    friend constexpr auto operator%=(Fixed64<Q>&, const Fixed64<R>&) noexcept -> Fixed64<Q>&;

    // Declare all other Fixed64 precisions as friends
    template <int Q>
    friend class Fixed64;
};

// Three-way comparison operator
template <int P, int Q>
constexpr auto operator<=>(const Fixed64<P>& a, const Fixed64<Q>& b) noexcept
    -> std::strong_ordering {
    // Always convert to higher precision for comparison
    if constexpr (P == Q) {
        return a.value() <=> b.value();
    } else if constexpr (P > Q) {
        return a.value() <=> (b.value() << (P - Q));
    } else {
        return (a.value() << (Q - P)) <=> b.value();
    }
}

template <int P, int Q>
constexpr auto operator==(const Fixed64<P>& a, const Fixed64<Q>& b) noexcept -> bool {
    return (a <=> b) == 0;
}

// Unified handling for arithmetic types and soft_double
template <int P, typename T>
    requires(std::is_arithmetic_v<T>)
constexpr auto operator<=>(const Fixed64<P>& a, T b) noexcept -> std::strong_ordering {
    return a <=> Fixed64<P>(b);
}

// Reverse comparison version
template <typename T, int P>
    requires(std::is_arithmetic_v<T>)
constexpr auto operator<=>(T a, const Fixed64<P>& b) noexcept -> std::strong_ordering {
    return Fixed64<P>(a) <=> b;
}

// Equality operator
template <int P, typename T>
    requires(std::is_arithmetic_v<T>)
constexpr auto operator==(const Fixed64<P>& a, T b) noexcept -> bool {
    return a == Fixed64<P>(b);
}

// Reverse equality operator
template <typename T, int P>
    requires(std::is_arithmetic_v<T>)
constexpr auto operator==(T a, const Fixed64<P>& b) noexcept -> bool {
    return Fixed64<P>(a) == b;
}

// 1. Fixed64 += Fixed64 operation (handles all precisions)
template <int Q, int R>
constexpr auto operator+=(Fixed64<Q>& a, const Fixed64<R>& b) noexcept -> Fixed64<Q>& {
    if constexpr (Q == R) {
        a.value_ += b.value_;
    } else if constexpr (Q > R) {
        a.value_ += b.value_ << (Q - R);
    } else {
        a.value_ += b.value_ >> (R - Q);
    }
    return a;
}

// 2. Arithmetic type += operation
template <int P, typename T>
    requires std::is_arithmetic_v<T>
constexpr auto operator+=(Fixed64<P>& a, const T& b) noexcept -> Fixed64<P>& {
    a += Fixed64<P>(b);
    return a;
}

template <int P, typename T>
constexpr auto operator+(Fixed64<P> a, const T& b) noexcept -> Fixed64<P> {
    a += b;
    return a;
}

template <typename T, int P>
    requires std::is_arithmetic_v<T>
constexpr auto operator+(const T& a, const Fixed64<P>& b) noexcept -> Fixed64<P> {
    return Fixed64<P>(a) + b;
}

// Subtraction operators
template <int Q, int R>
constexpr auto operator-=(Fixed64<Q>& a, const Fixed64<R>& b) noexcept -> Fixed64<Q>& {
    if constexpr (Q == R) {
        a.value_ -= b.value_;
    } else if constexpr (Q > R) {
        a.value_ -= b.value_ << (Q - R);
    } else {
        a.value_ -= b.value_ >> (R - Q);
    }
    return a;
}

template <int P, typename T>
    requires std::is_arithmetic_v<T>
constexpr auto operator-=(Fixed64<P>& a, const T& b) noexcept -> Fixed64<P>& {
    a -= Fixed64<P>(b);
    return a;
}

template <int P, typename T>
constexpr auto operator-(Fixed64<P> a, const T& b) noexcept -> Fixed64<P> {
    a -= b;
    return a;
}

template <typename T, int P>
    requires std::is_arithmetic_v<T>
constexpr auto operator-(const T& a, const Fixed64<P>& b) noexcept -> Fixed64<P> {
    return Fixed64<P>(a) - b;
}

// Multiplication operators
template <int Q, int R>
constexpr auto operator*=(Fixed64<Q>& a, const Fixed64<R>& b) noexcept -> Fixed64<Q>& {
    // Use leading zero count to determine if 64-bit multiplication is safe
    if constexpr (Q + R <= 32) {
        // Fast path: if the sum of leading zeros in both operands is >= Q, no
        // overflow will occur
        if ((a.value_ >> 32) == 0 && (b.value_ >> 32) == 0) {
            // Safe to use 64-bit multiplication, no need for 128-bit integer
            a.value_ = (a.value_ * b.value_) >> R;

            return a;
        }

        a.value_ = Primitives::Fixed64MulBitStyle(a.value_, b.value_, R);

        return a;
    }

    a.value_ = Primitives::Fixed64Mul(a.value_, b.value_, R);

    return a;
}

template <int Q, typename IntType>
    requires std::is_integral_v<IntType>
constexpr auto operator*=(Fixed64<Q>& a, const IntType& b) noexcept -> Fixed64<Q>& {
    a.value_ *= b;
    return a;
}

template <int P, typename FloatType>
    requires std::is_floating_point_v<FloatType>
constexpr auto operator*=(Fixed64<P>& a, const FloatType& b) noexcept -> Fixed64<P>& {
    a *= Fixed64<P>(b);
    return a;
}

template <int P, typename T>
constexpr auto operator*(Fixed64<P> a, const T& b) noexcept -> Fixed64<P> {
    a *= b;
    return a;
}

template <typename IntType, int P>
    requires std::is_integral_v<IntType>
constexpr auto operator*(const IntType& a, const Fixed64<P>& b) noexcept -> Fixed64<P> {
    return Fixed64<P>(b.value() * a, detail::nothing{});
}

template <typename FloatType, int P>
    requires std::is_floating_point_v<FloatType>
constexpr auto operator*(const FloatType& a, const Fixed64<P>& b) noexcept -> Fixed64<P> {
    return Fixed64<P>(a) * b;
}

// Division operators
template <int Q, int R>
constexpr auto operator/=(Fixed64<Q>& a, const Fixed64<R>& b) noexcept -> Fixed64<Q>& {
    // Handle division by zero
    if (b.value_ == 0) {
        a = (a.value_ >= 0) ? Fixed64<Q>::Infinity() : Fixed64<Q>::NegInfinity();
        return a;
    }

    // For low precision fixed-point numbers, use runtime optimization
    if constexpr (Q + R <= 32) {
        // Check if left shifting by Q bits is safe (won't lose significant bits)
        if ((a.value_ << R) >> R == a.value_) {
            // Safe path: use direct 64-bit integer division, avoiding complex
            // calculations
            a.value_ = (a.value_ << R) / b.value_;

            return a;
        }

        a.value_ = Primitives::Fixed64DivBitStyle(a.value_, b.value_, R);

        return a;
    }

    a.value_ = Primitives::Fixed64Div(a.value_, b.value_, R);

    return a;
}

// Division operator - integer specialization
template <int Q, typename IntType>
    requires std::is_integral_v<IntType>
constexpr auto operator/=(Fixed64<Q>& a, const IntType& b) noexcept -> Fixed64<Q>& {
    a.value_ /= b;
    return a;
}

// Division operator - floating-point version
template <int P, typename FloatType>
    requires std::is_floating_point_v<FloatType>
constexpr auto operator/=(Fixed64<P>& a, const FloatType& b) noexcept -> Fixed64<P>& {
    a /= Fixed64<P>(b);
    return a;
}

template <int P, typename T>
constexpr auto operator/(Fixed64<P> a, const T& b) noexcept -> Fixed64<P> {
    a /= b;
    return a;
}

template <typename T, int P>
    requires std::is_arithmetic_v<T>
constexpr auto operator/(const T& a, const Fixed64<P>& b) noexcept -> Fixed64<P> {
    return Fixed64<P>(a) / b;
}

// Modulo operators
template <int P, int Q>
constexpr auto operator%=(Fixed64<P>& a, const Fixed64<Q>& b) noexcept -> Fixed64<P>& {
    if (b.value_ == 0) {
        a = Fixed64<P>::NaN();
        return a;
    }
    if constexpr (P == Q) {
        a.value_ %= b.value_;
    } else if constexpr (P > Q) {
        a.value_ %= (b.value_ << (P - Q));
    } else {
        a.value_ %= (b.value_ >> (Q - P));
    }
    return a;
}

template <int P, typename T>
    requires std::is_arithmetic_v<T>
constexpr auto operator%=(Fixed64<P>& a, const T& b) noexcept -> Fixed64<P>& {
    a %= Fixed64<P>(b);
    return a;
}

template <int P, typename T>
constexpr auto operator%(Fixed64<P> a, const T& b) noexcept -> Fixed64<P> {
    a %= b;
    return a;
}

template <typename T, int P>
    requires std::is_arithmetic_v<T>
constexpr auto operator%(const T& a, const Fixed64<P>& b) noexcept -> Fixed64<P> {
    return Fixed64<P>(a) % b;
}

/**
 * @brief Q47.16 fixed-point number, suitable for general game calculations
 * @details Range and precision:
 *   - Range: ±1.4e14 [-140,737,488,355,328.0, +140,737,488,355,327.999984]
 *   - Precision: 1.5e-5 (2^-16 ≈ 0.000015)
 *   - Maximum square root: ±11,863,283.20301
 *
 */
using Fixed64_16 = Fixed64<16>;

/**
 * @brief Q31.32 fixed-point number, specialized for trigonometric functions
 * @details Range and precision:
 *   - Range: ±2.1e9 [-2,147,483,648.0, +2,147,483,647.99999999976]
 *   - Precision: 2.3e-10 (2^-32 ≈ 0.00000000023)
 *   - Maximum square root: ±46,340.9500118414
 *
 * Key use cases:
 *   - Trigonometric calculations (sin/cos/tan)
 */
using Fixed64_32 = Fixed64<32>;

/**
 * @brief Q23.40 fixed-point number, used for high-precision geometric
 * calculations
 * @details Range and precision:
 *   - Range: ±8.3e6 [-8,388,608.0, +8,388,607.99999999999909]
 *   - Precision: 9.1e-13 (2^-40 ≈ 0.0000000000009)
 *   - Maximum square root: ±2,896.3093757400984
 *
 */
using Fixed64_40 = Fixed64<40>;
}  // namespace math::fp

// Standard library support
namespace std {
// 1. Numeric limits
template <int P>
struct numeric_limits<::math::fp::Fixed64<P>> {
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = true;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = true;
    static constexpr bool has_quiet_NaN = true;

    static constexpr auto min() noexcept -> ::math::fp::Fixed64<P> {
        return ::math::fp::Fixed64<P>::Min();
    }

    static constexpr auto max() noexcept -> ::math::fp::Fixed64<P> {
        return ::math::fp::Fixed64<P>::Max();
    }

    static constexpr auto lowest() noexcept -> ::math::fp::Fixed64<P> {
        return ::math::fp::Fixed64<P>::Min();
    }

    static constexpr auto epsilon() noexcept -> ::math::fp::Fixed64<P> {
        return ::math::fp::Fixed64<P>::Epsilon();
    }

    static constexpr auto infinity() noexcept -> ::math::fp::Fixed64<P> {
        return ::math::fp::Fixed64<P>::Infinity();
    }

    static constexpr auto quiet_NaN() noexcept -> ::math::fp::Fixed64<P> {
        return ::math::fp::Fixed64<P>::NaN();
    }
};

// 2. Hash support
template <int P>
struct hash<::math::fp::Fixed64<P>> {
    auto operator()(const ::math::fp::Fixed64<P>& x) const noexcept -> size_t {
        return hash<int64_t>{}(x.value());
    }
};

// 3. Special value checks
template <int P>
inline auto isnan(const ::math::fp::Fixed64<P>& x) noexcept -> bool {
    return x == ::math::fp::Fixed64<P>::NaN();
}

template <int P>
inline auto isinf(const ::math::fp::Fixed64<P>& x) noexcept -> bool {
    return x == ::math::fp::Fixed64<P>::Infinity() || x == ::math::fp::Fixed64<P>::NegInfinity();
}

template <int P>
inline auto isfinite(const ::math::fp::Fixed64<P>& x) noexcept -> bool {
    return !isnan(x) && !isinf(x);
}

template <int P>
inline auto signbit(const ::math::fp::Fixed64<P>& x) noexcept -> bool {
    return x.value() < 0;
}

template <int P>
inline auto copysign(const ::math::fp::Fixed64<P>& mag, const ::math::fp::Fixed64<P>& sgn) noexcept
    -> ::math::fp::Fixed64<P> {
    return signbit(mag) == signbit(sgn) ? mag : -mag;
}

// 4. Basic math functions
template <int P>
inline auto fmod(const ::math::fp::Fixed64<P>& x, const ::math::fp::Fixed64<P>& y) noexcept
    -> ::math::fp::Fixed64<P> {
    return x % y;
}

// 5. String conversion
template <int P>
string to_string(const ::math::fp::Fixed64<P>& num) {
    return num.ToString();
}

template <int P>
::math::fp::Fixed64<P> stof64(const string& str) {
    return ::math::fp::Fixed64<P>::FromString(str);
}
}  // namespace std
