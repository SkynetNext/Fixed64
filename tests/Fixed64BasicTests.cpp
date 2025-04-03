#include <cmath>
#include <limits>
#include <sstream>
#include <string>

#include "Fixed64.h"
#include "Fixed64Math.h"
#include "gtest/gtest.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 (M_PI / 2.0)
#endif

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
    EXPECT_NEAR(static_cast<double>(floatValue), 3.14159, 1e-4);

    // Copy constructor
    Fixed16 copyValue(floatValue);
    EXPECT_EQ(copyValue.value(), floatValue.value());

    // Precision conversion
    Fixed32 highPrecision(3.14159);
    Fixed16 convertedValue(highPrecision);
    EXPECT_NEAR(static_cast<double>(convertedValue), 3.14159, 1e-4);

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
    EXPECT_NEAR(static_cast<double>(a + b), 7.75, 1e-4);

    // Subtraction
    EXPECT_NEAR(static_cast<double>(a - b), 3.25, 1e-4);

    // Multiplication
    EXPECT_NEAR(static_cast<double>(a * b), 12.375, 1e-4);

    // Division
    EXPECT_NEAR(static_cast<double>(a / b), 2.444444, 1e-4);

    // Negation
    EXPECT_NEAR(static_cast<double>(-a), -5.5, 1e-4);

    // Compound assignment
    Fixed c(10.0);
    c += Fixed(5.0);
    EXPECT_NEAR(static_cast<double>(c), 15.0, 1e-4);

    c -= Fixed(3.0);
    EXPECT_NEAR(static_cast<double>(c), 12.0, 1e-4);

    c *= Fixed(2.0);
    EXPECT_NEAR(static_cast<double>(c), 24.0, 1e-4);

    c /= Fixed(4.0);
    EXPECT_NEAR(static_cast<double>(c), 6.0, 1e-4);

    // Integer operations
    Fixed d(10.5);
    EXPECT_NEAR(static_cast<double>(d + 5), 15.5, 1e-4);
    EXPECT_NEAR(static_cast<double>(d - 3), 7.5, 1e-4);
    EXPECT_NEAR(static_cast<double>(d * 2), 21.0, 1e-4);
    EXPECT_NEAR(static_cast<double>(d / 2), 5.25, 1e-4);

    // Reverse operations with integers
    EXPECT_NEAR(static_cast<double>(5 + d), 15.5, 1e-4);
    EXPECT_NEAR(static_cast<double>(15 - d), 4.5, 1e-4);
    EXPECT_NEAR(static_cast<double>(2 * d), 21.0, 1e-4);
    EXPECT_NEAR(static_cast<double>(21 / d), 2.0, 1e-4);
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
}

// Modulo and remainder operations
TEST(Fixed64BasicTest, ModuloOperations) {
    using Fixed = math::fp::Fixed64<16>;

    Fixed a(10.5);
    Fixed b(3.0);

    // Modulo operation
    EXPECT_NEAR(static_cast<double>(a % b), 1.5, 1e-4);

    // Negative modulo
    Fixed c(-10.5);
    EXPECT_NEAR(static_cast<double>(c % b), -1.5, 1e-4);

    // Modulo with integer
    EXPECT_NEAR(static_cast<double>(a % 3), 1.5, 1e-4);
    EXPECT_NEAR(static_cast<double>(10 % a), 10.0, 1e-4);
}

// Special values tests
TEST(Fixed64BasicTest, SpecialValues) {
    using Fixed = math::fp::Fixed64<16>;

    // Constants
    EXPECT_EQ(static_cast<double>(Fixed::Zero()), 0.0);
    EXPECT_EQ(static_cast<double>(Fixed::One()), 1.0);
    EXPECT_NEAR(static_cast<double>(Fixed::Pi()), M_PI, 1e-4);
    EXPECT_NEAR(static_cast<double>(Fixed::HalfPi()), M_PI / 2, 1e-4);
    EXPECT_NEAR(static_cast<double>(Fixed::TwoPi()), 2 * M_PI, 1e-4);

    // Min/Max values
    EXPECT_GT(Fixed::Max(), Fixed(1000000));
    EXPECT_LT(Fixed::Min(), Fixed(-1000000));

    // Epsilon
    EXPECT_GT(Fixed::Epsilon(), Fixed::Zero());
    EXPECT_LT(Fixed::Epsilon(), Fixed(0.01));

    // Special values
    EXPECT_TRUE(std::isinf(Fixed::Infinity()));
    EXPECT_TRUE(std::isinf(Fixed::NegInfinity()));
    EXPECT_TRUE(std::isnan(Fixed::NaN()));
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
    EXPECT_NEAR(static_cast<double>(b), 3.14159, 1e-4);

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
    EXPECT_NEAR(static_cast<double>(d), 3.14159, 1e-4);
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
    EXPECT_EQ(std::round(Fixed(-5.5)), Fixed(-5.0));

    // trunc
    EXPECT_EQ(std::trunc(Fixed(5.7)), Fixed(5.0));
    EXPECT_EQ(std::trunc(Fixed(-5.7)), Fixed(-5.0));

    // fmod
    EXPECT_NEAR(static_cast<double>(std::fmod(Fixed(10.5), Fixed(3.0))), 1.5, 1e-4);

    // Special value checks
    EXPECT_TRUE(std::isnan(Fixed::NaN()));
    EXPECT_TRUE(std::isinf(Fixed::Infinity()));
    EXPECT_TRUE(std::isinf(Fixed::NegInfinity()));
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
    EXPECT_NEAR(static_cast<double>(highPrecision), 3.14159265359, 1e-9);
    EXPECT_NEAR(static_cast<double>(mediumPrecision), 3.14159265359, 1e-4);
    EXPECT_NEAR(static_cast<double>(lowPrecision), 3.14159265359, 1e-2);

    // Lower to higher precision
    Fixed8 original(3.14);
    Fixed16 higher1(original);
    Fixed32 higher2(original);

    // Verify no additional precision is gained
    EXPECT_NEAR(static_cast<double>(original), static_cast<double>(higher1), 1e-6);
    EXPECT_NEAR(static_cast<double>(original), static_cast<double>(higher2), 1e-6);
}

// Edge case tests
TEST(Fixed64BasicTest, EdgeCases) {
    using Fixed = math::fp::Fixed64<16>;

    // Division by zero
    Fixed a(5.5);
    Fixed zero = Fixed::Zero();

    // Should return infinity
    EXPECT_TRUE(std::isinf(a / zero));

    // Operations with NaN
    Fixed nan = Fixed::NaN();
    EXPECT_TRUE(!std::isnan(nan + a));
    EXPECT_TRUE(!std::isnan(nan - a));
    EXPECT_TRUE(!std::isnan(nan * a));
    EXPECT_TRUE(!std::isnan(nan / a));

    // Operations with infinity
    Fixed inf = Fixed::Infinity();
    EXPECT_TRUE(!std::isinf(inf + a));
    EXPECT_TRUE(!std::isinf(inf * a));
    EXPECT_TRUE(!std::isnan(inf - inf));
    EXPECT_TRUE(!std::isnan(inf / inf));
    EXPECT_TRUE(std::isinf(a / Fixed::Zero()));
}