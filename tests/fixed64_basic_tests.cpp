#include <cmath>
#include <limits>
#include <sstream>
#include <string>

#include "fixed64.h"
#include "fixed64_math.h"
#include "gtest/gtest.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 (M_PI / 2.0)
#endif

namespace math::fp::tests {
constexpr double epsilon8 = static_cast<double>(math::fp::Fixed64<8>::Epsilon());
constexpr double epsilon16 = static_cast<double>(math::fp::Fixed64<16>::Epsilon());
constexpr double epsilon32 = static_cast<double>(math::fp::Fixed64<32>::Epsilon());

// Basic construction and conversion tests
TEST(Fixed64BasicTest, ConstructionAndConversion) {
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;

    // Default constructor
    Fixed16 defaultValue;
    EXPECT_EQ(defaultValue.value(), 0);

    // Integer constructor
    Fixed16 intValue(42);
    EXPECT_EQ(static_cast<int>(intValue), 42);

    // Floating-point constructor
    Fixed16 floatValue(3.14159);
    EXPECT_NEAR(static_cast<double>(floatValue), 3.14159, epsilon16);

    // Copy constructor
    Fixed16 copyValue(floatValue);
    EXPECT_EQ(copyValue.value(), floatValue.value());

    // Precision conversion
    Fixed32 highPrecision(3.14159);
    Fixed16 convertedValue(highPrecision);
    EXPECT_NEAR(static_cast<double>(convertedValue), 3.14159, epsilon16);

    // Conversion to string
    std::string str = floatValue.ToString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("3.14"), std::string::npos);
}

// Arithmetic operations tests
TEST(Fixed64BasicTest, ArithmeticOperations) {
    using Fixed = math::fp::Fixed64<16>;

    // Addition
    Fixed a(5.5);
    Fixed b(2.25);
    EXPECT_NEAR(static_cast<double>(a + b), 7.75, epsilon16);

    // Subtraction
    EXPECT_NEAR(static_cast<double>(a - b), 3.25, epsilon16);

    // Multiplication
    EXPECT_NEAR(
        static_cast<double>(a * b), static_cast<double>(a) * static_cast<double>(b), epsilon16);

    // Division
    EXPECT_NEAR(
        static_cast<double>(a / b), static_cast<double>(a) / static_cast<double>(b), epsilon16);

    // Negation
    EXPECT_NEAR(static_cast<double>(-a), -5.5, epsilon16);

    // Compound assignment
    Fixed c(10.0);
    c += Fixed(5.0);
    EXPECT_NEAR(static_cast<double>(c), 15.0, epsilon16);

    c -= Fixed(3.0);
    EXPECT_NEAR(static_cast<double>(c), 12.0, epsilon16);

    c *= Fixed(2.0);
    EXPECT_NEAR(static_cast<double>(c), 24.0, epsilon16);

    c /= Fixed(4.0);
    EXPECT_NEAR(static_cast<double>(c), 6.0, epsilon16);

    // Integer operations
    Fixed d(10.5);
    EXPECT_NEAR(static_cast<double>(d + 5), 15.5, epsilon16);
    EXPECT_NEAR(static_cast<double>(d - 3), 7.5, epsilon16);
    EXPECT_NEAR(static_cast<double>(d * 2), 21.0, epsilon16);
    EXPECT_NEAR(static_cast<double>(d / 2), 5.25, epsilon16);

    // Reverse operations with integers
    EXPECT_NEAR(static_cast<double>(5 + d), 15.5, epsilon16);
    EXPECT_NEAR(static_cast<double>(15 - d), 4.5, epsilon16);
    EXPECT_NEAR(static_cast<double>(2 * d), 21.0, epsilon16);
    EXPECT_NEAR(static_cast<double>(21 / d), 2.0, epsilon16);

    // Very small values (near zero)
    Fixed tiny1(0.0001);
    Fixed tiny2(0.0002);
    EXPECT_NEAR(static_cast<double>(tiny1 + tiny2), 0.0003, epsilon16);
    EXPECT_NEAR(static_cast<double>(tiny1 - tiny2), -0.0001, epsilon16);
    EXPECT_NEAR(static_cast<double>(tiny1 * tiny2),
                static_cast<double>(tiny1) * static_cast<double>(tiny2),
                epsilon16);
    EXPECT_NEAR(static_cast<double>(tiny1 / tiny2),
                static_cast<double>(tiny1) / static_cast<double>(tiny2),
                epsilon16)
        << "tiny1 = " << tiny1 << " tiny2 = " << tiny2 << " tiny1 / tiny2 = " << tiny1 / tiny2;
}

// Comparison operations tests
TEST(Fixed64BasicTest, ComparisonOperations) {
    using Fixed = math::fp::Fixed64<16>;

    Fixed a(5.5);
    Fixed b(5.5);
    Fixed c(10.0);
    Fixed d(-5.5);

    // Equality
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);

    // Inequality
    EXPECT_TRUE(a != c);
    EXPECT_FALSE(a != b);

    // Less than
    EXPECT_TRUE(a < c);
    EXPECT_FALSE(c < a);

    // Greater than
    EXPECT_TRUE(c > a);
    EXPECT_FALSE(a > c);

    // Less than or equal
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a <= c);
    EXPECT_FALSE(c <= a);

    // Greater than or equal
    EXPECT_TRUE(a >= b);
    EXPECT_TRUE(c >= a);
    EXPECT_FALSE(a >= c);

    // Comparison with zero
    EXPECT_TRUE(a > Fixed::Zero());
    EXPECT_TRUE(d < Fixed::Zero());
    EXPECT_TRUE(Fixed::Zero() == Fixed(0));

    // Comparison with epsilon
    Fixed almostEqual = a + Fixed::Epsilon();
    EXPECT_TRUE(a != almostEqual);
    EXPECT_TRUE(a < almostEqual);
}

// Modulo and remainder operations
TEST(Fixed64BasicTest, ModuloOperations) {
    using Fixed = math::fp::Fixed64<16>;

    Fixed a(10.5);
    Fixed b(3.0);

    // Modulo operation
    EXPECT_NEAR(static_cast<double>(a % b),
                std::fmod(static_cast<double>(a), static_cast<double>(b)),
                epsilon16);

    // Negative modulo
    Fixed c(-10.5);
    EXPECT_NEAR(static_cast<double>(c % b),
                std::fmod(static_cast<double>(c), static_cast<double>(b)),
                epsilon16);

    // Modulo with integer
    EXPECT_NEAR(static_cast<double>(a % 3), std::fmod(static_cast<double>(a), 3.0), epsilon16);
    EXPECT_NEAR(static_cast<double>(10 % a), std::fmod(10.0, static_cast<double>(a)), epsilon16);

    // Large values
    Fixed d(1000.75);
    Fixed e(10.5);
    EXPECT_NEAR(static_cast<double>(d % e),
                std::fmod(static_cast<double>(d), static_cast<double>(e)),
                epsilon16);

    // Very small values
    Fixed f(0.0075);
    Fixed g(0.002);
    EXPECT_NEAR(static_cast<double>(f % g),
                std::fmod(static_cast<double>(f), static_cast<double>(g)),
                epsilon16);
}

// Special values tests
TEST(Fixed64BasicTest, SpecialValues) {
    using Fixed = math::fp::Fixed64<16>;

    // Constants
    EXPECT_EQ(static_cast<double>(Fixed::Zero()), 0.0);
    EXPECT_EQ(static_cast<double>(Fixed::One()), 1.0);
    EXPECT_NEAR(static_cast<double>(Fixed::Pi()), M_PI, epsilon16);
    EXPECT_NEAR(static_cast<double>(Fixed::HalfPi()), M_PI / 2, epsilon16);
    EXPECT_NEAR(static_cast<double>(Fixed::TwoPi()), 2 * M_PI, epsilon16);

    // Min/Max values
    EXPECT_GT(Fixed::Max(), Fixed(1000000));
    EXPECT_LT(Fixed::Min(), Fixed(-1000000));

    // Epsilon
    EXPECT_GT(Fixed::Epsilon(), Fixed::Zero());
    EXPECT_LT(Fixed::Epsilon(), Fixed(0.01));

    // Special values
    EXPECT_TRUE(std::isinf(Fixed::Infinity()));
    EXPECT_TRUE(std::isinf(-Fixed::Infinity()));
    EXPECT_TRUE(std::isnan(Fixed::NaN()));

    // Verify that epsilon is the smallest representable positive value
    Fixed almostZero = Fixed::Zero() + Fixed::Epsilon();
    EXPECT_GT(static_cast<double>(almostZero), 0.0);

    Fixed tooSmall = Fixed::Zero() + Fixed::Epsilon() / 2;
    EXPECT_EQ(static_cast<double>(tooSmall), 0.0);
}

// String conversion tests
TEST(Fixed64BasicTest, StringConversion) {
    using Fixed = math::fp::Fixed64<16>;

    // ToString
    Fixed a(3.14159);
    std::string str = a.ToString();
    EXPECT_FALSE(str.empty());

    // FromString
    Fixed b = Fixed::FromString("3.14159");
    EXPECT_NEAR(static_cast<double>(b), 3.14159, epsilon16);

    // Invalid string
    Fixed c = Fixed::FromString("invalid");
    EXPECT_EQ(c, Fixed::Zero());

    // std::to_string
    std::string stdStr = std::to_string(a);
    EXPECT_FALSE(stdStr.empty());

    // Stream operators
    std::stringstream ss;
    ss << a;
    Fixed d;
    ss >> d;
    EXPECT_NEAR(static_cast<double>(d), 3.14159, epsilon16);

    // Edge cases for string conversion
    Fixed large(123456.789);
    std::string largeStr = large.ToString();
    EXPECT_NE(largeStr.find("123456.789"), std::string::npos);

    Fixed negative(-42.5);
    std::string negStr = negative.ToString();
    EXPECT_NE(negStr.find("-42.5"), std::string::npos);
}

// std namespace function tests
TEST(Fixed64BasicTest, StdFunctions) {
    using Fixed = math::fp::Fixed64<16>;

    // abs
    EXPECT_EQ(std::abs(Fixed(-5.5)), Fixed(5.5));

    // fabs
    EXPECT_EQ(std::fabs(Fixed(-5.5)), Fixed(5.5));

    // floor
    EXPECT_EQ(std::floor(Fixed(5.7)), Fixed(5.0));
    EXPECT_EQ(std::floor(Fixed(-5.7)), Fixed(-6.0));

    // ceil
    EXPECT_EQ(std::ceil(Fixed(5.2)), Fixed(6.0));
    EXPECT_EQ(std::ceil(Fixed(-5.2)), Fixed(-5.0));

    // round
    EXPECT_EQ(std::round(Fixed(5.4)), Fixed(5.0));
    EXPECT_EQ(std::round(Fixed(5.5)), Fixed(6.0));
    EXPECT_EQ(std::round(Fixed(-5.5)), Fixed(-6));
    EXPECT_EQ(std::round(Fixed(-5.4)), Fixed(-5.0));

    // trunc
    EXPECT_EQ(std::trunc(Fixed(5.7)), Fixed(5.0));
    EXPECT_EQ(std::trunc(Fixed(-5.7)), Fixed(-5.0));

    // fmod
    EXPECT_NEAR(
        static_cast<double>(std::fmod(Fixed(10.5), Fixed(3.0))), std::fmod(10.5, 3.0), epsilon16);

    // Special value checks
    EXPECT_TRUE(std::isnan(Fixed::NaN()));
    EXPECT_TRUE(std::isinf(Fixed::Infinity()));
    EXPECT_TRUE(std::isinf(-Fixed::Infinity()));
    EXPECT_FALSE(std::isfinite(Fixed::Infinity()));
    EXPECT_TRUE(std::isfinite(Fixed(5.5)));

    // Sign bit
    EXPECT_TRUE(std::signbit(Fixed(-5.5)));
    EXPECT_FALSE(std::signbit(Fixed(5.5)));

    // Copy sign
    EXPECT_EQ(std::copysign(Fixed(5.5), Fixed(-1.0)), Fixed(-5.5));
    EXPECT_EQ(std::copysign(Fixed(-5.5), Fixed(1.0)), Fixed(5.5));
}

// Numeric limits tests
TEST(Fixed64BasicTest, NumericLimits) {
    using Fixed = math::fp::Fixed64<16>;
    using limits = std::numeric_limits<Fixed>;

    EXPECT_TRUE(limits::is_specialized);
    EXPECT_TRUE(limits::is_signed);
    EXPECT_FALSE(limits::is_integer);
    EXPECT_TRUE(limits::is_exact);
    EXPECT_TRUE(limits::has_infinity);
    EXPECT_TRUE(limits::has_quiet_NaN);

    EXPECT_EQ(limits::min(), Fixed::Min());
    EXPECT_EQ(limits::max(), Fixed::Max());
    EXPECT_EQ(limits::lowest(), Fixed::Min());
    EXPECT_EQ(limits::epsilon(), Fixed::Epsilon());
    EXPECT_EQ(limits::infinity(), Fixed::Infinity());
    EXPECT_EQ(limits::quiet_NaN(), Fixed::NaN());

    // Verify that numeric_limits values match our expectations
    EXPECT_GT(static_cast<double>(limits::max()), 1e6);
    EXPECT_LT(static_cast<double>(limits::min()), -1e6);
    EXPECT_GT(static_cast<double>(limits::epsilon()), 0.0);
    EXPECT_LT(static_cast<double>(limits::epsilon()), 0.01);
}

// Hash support tests
TEST(Fixed64BasicTest, HashSupport) {
    using Fixed = math::fp::Fixed64<16>;

    Fixed a(3.14159);
    Fixed b(3.14159);
    Fixed c(2.71828);

    std::hash<Fixed> hasher;

    // Same values should have same hash
    EXPECT_EQ(hasher(a), hasher(b));

    // Different values should (likely) have different hashes
    // Note: This is not guaranteed by hash functions, but is likely for distinct values
    EXPECT_NE(hasher(a), hasher(c));

    // Hash of zero should be consistent
    EXPECT_EQ(hasher(Fixed::Zero()), hasher(Fixed(0)));

    // Hash of special values
    EXPECT_NE(hasher(Fixed::NaN()), hasher(Fixed::Zero()));
    EXPECT_NE(hasher(Fixed::Infinity()), hasher(-Fixed::Infinity()));
}

// Precision conversion tests
TEST(Fixed64BasicTest, PrecisionConversion) {
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;
    using Fixed8 = math::fp::Fixed64<8>;

    // Higher to lower precision
    Fixed32 highPrecision(3.14159265359);
    Fixed16 mediumPrecision(highPrecision);
    Fixed8 lowPrecision(highPrecision);

    // Verify precision loss is as expected
    EXPECT_NEAR(static_cast<double>(highPrecision), 3.14159265359, epsilon32);
    EXPECT_NEAR(static_cast<double>(mediumPrecision), 3.14159265359, epsilon16);
    EXPECT_NEAR(static_cast<double>(lowPrecision), 3.14159265359, epsilon8);

    // Lower to higher precision
    Fixed8 original(3.14);
    Fixed16 higher1(original);
    Fixed32 higher2(original);

    // Verify no additional precision is gained
    EXPECT_NEAR(static_cast<double>(original), static_cast<double>(higher1), epsilon16);
    EXPECT_NEAR(static_cast<double>(original), static_cast<double>(higher2), epsilon32);

    // Test with extreme values
    Fixed32 veryPrecise(0.0000001);
    Fixed8 lessPrecsie(veryPrecise);
    EXPECT_NEAR(static_cast<double>(lessPrecsie), 0.0, epsilon8);

    Fixed8 roughValue(100.5);
    Fixed32 convertedUp(roughValue);
    EXPECT_NEAR(static_cast<double>(convertedUp), 100.5, epsilon32);
}

// Edge case tests
TEST(Fixed64BasicTest, EdgeCases) {
    using Fixed = math::fp::Fixed64<16>;

    // Division by zero
    Fixed a(5.5);
    Fixed zero = Fixed::Zero();

    // Should return infinity
    EXPECT_TRUE(std::isinf(a / zero));

    // Division of zero by non-zero
    EXPECT_NEAR(static_cast<double>(zero / a), 0.0, epsilon16);

    // Operations near limits
    Fixed nearMax = Fixed::Max() - Fixed(1);
    Fixed nearMin = Fixed::Min() + Fixed(1);

    // These operations should not overflow
    EXPECT_FALSE(std::isinf(nearMax + Fixed(0.5)));
    EXPECT_FALSE(std::isinf(nearMin - Fixed(0.5)));
}

// Additional test for very small and very large values
TEST(Fixed64BasicTest, ExtremeValues) {
    using Fixed = math::fp::Fixed64<16>;

    // Very small positive value
    Fixed tiny(1e-10);
    EXPECT_NEAR(static_cast<double>(tiny), 0.0, epsilon16);

    // Very large value
    Fixed huge(1e10);
    EXPECT_NEAR(static_cast<double>(huge), 1e10, epsilon16);

    // Operations with extreme values
    EXPECT_NEAR(static_cast<double>(tiny + tiny), 0.0, epsilon16);
    EXPECT_NEAR(static_cast<double>(huge + huge), 2e10, epsilon16);
    EXPECT_NEAR(static_cast<double>(huge * tiny), 0.0, epsilon16);

    // Verify that operations with extreme values don't produce unexpected results
    EXPECT_FALSE(std::isnan(huge * tiny));
    EXPECT_FALSE(std::isinf(huge * tiny));
}

}  // namespace math::fp::tests