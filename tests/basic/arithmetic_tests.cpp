#include <cmath>
#include <limits>

#include "fixed64.h"
#include "fixed64_math.h"
#include "gtest/gtest.h"

namespace math::fp::tests {

// Tests for basic arithmetic operations with a wide range of values
TEST(Fixed64ArithmeticTest, BasicOperations) {
    using Fixed = math::fp::Fixed64<16>;

    // Calculate epsilon for precision comparisons
    double epsilon = static_cast<double>(Fixed::Epsilon());

    // Small positive values
    Fixed a(5.5);
    Fixed b(2.25);
    EXPECT_NEAR(static_cast<double>(a + b), 7.75, epsilon);
    EXPECT_NEAR(static_cast<double>(a - b), 3.25, epsilon);
    EXPECT_NEAR(static_cast<double>(a * b), 12.375, epsilon);
    EXPECT_NEAR(static_cast<double>(a / b), 2.444444, epsilon);
    EXPECT_NEAR(static_cast<double>(-a), -5.5, epsilon);

    // Very small values (near zero)
    Fixed tiny1(0.0001);
    Fixed tiny2(0.0002);
    EXPECT_NEAR(static_cast<double>(tiny1 + tiny2), 0.0003, epsilon);
    EXPECT_NEAR(static_cast<double>(tiny1 - tiny2), -0.0001, epsilon);
    EXPECT_NEAR(static_cast<double>(tiny1 * tiny2), 0.00000002, epsilon);
    EXPECT_NEAR(static_cast<double>(tiny1 / tiny2),
                static_cast<double>(tiny1) / static_cast<double>(tiny2),
                epsilon)
        << "tiny1 = " << tiny1 << " tiny2 = " << tiny2 << " tiny1 / tiny2 = " << tiny1 / tiny2
        << " epsilon = " << epsilon;

    // Very large values (near max)
    Fixed veryLarge1(1000000.0);
    Fixed veryLarge2(2000000.0);
    EXPECT_NEAR(static_cast<double>(veryLarge1 + veryLarge2), 3000000.0, epsilon);
    EXPECT_NEAR(static_cast<double>(veryLarge1 - veryLarge2), -1000000.0, epsilon);

    // Values near max/min of representable range
    Fixed nearMax = Fixed::Max() / Fixed(2);
    Fixed nearMin = Fixed::Min() / Fixed(2);
    // Test for expected behavior with values near limits
    EXPECT_TRUE(nearMax.value() > 0) << " nearMax = " << nearMax;
    EXPECT_TRUE(nearMin.value() < 0) << " nearMin = " << nearMin;
}

// Compound assignment tests with diverse values
TEST(Fixed64ArithmeticTest, CompoundAssignment) {
    using Fixed = math::fp::Fixed64<16>;

    // Calculate epsilon for precision comparisons
    double epsilon = static_cast<double>(Fixed::Epsilon());

    // Positive values - test each operation independently
    Fixed c1(10.0);
    c1 += Fixed(5.0);
    EXPECT_NEAR(static_cast<double>(c1), 15.0, epsilon);

    Fixed c2(12.0);
    c2 -= Fixed(3.0);
    EXPECT_NEAR(static_cast<double>(c2), 9.0, epsilon);

    Fixed c3(12.0);
    c3 *= Fixed(2.0);
    EXPECT_NEAR(static_cast<double>(c3), 24.0, epsilon);

    Fixed c4(24.0);
    c4 /= Fixed(4.0);
    EXPECT_NEAR(static_cast<double>(c4), 6.0, epsilon);

    // Negative values - test each operation independently
    Fixed d1(-10.0);
    d1 += Fixed(-5.0);
    EXPECT_NEAR(static_cast<double>(d1), -15.0, epsilon);

    Fixed d2(-10.0);
    d2 -= Fixed(-8.0);
    EXPECT_NEAR(static_cast<double>(d2), -2.0, epsilon);

    Fixed d3(-7.0);
    d3 *= Fixed(-3.0);
    EXPECT_NEAR(static_cast<double>(d3), 21.0, epsilon);

    Fixed d4(21.0);
    d4 /= Fixed(-7.0);
    EXPECT_NEAR(static_cast<double>(d4), -3.0, epsilon);

    // Large values - test each operation independently
    Fixed e1(100000.0);
    e1 += Fixed(50000.0);
    EXPECT_NEAR(static_cast<double>(e1), 150000.0, epsilon);

    Fixed e2(150000.0);
    e2 -= Fixed(100000.0);
    EXPECT_NEAR(static_cast<double>(e2), 50000.0, epsilon);

    // Small values - test each operation independently
    Fixed f1(1.0);
    f1 += Fixed(0.000001);
    EXPECT_NEAR(static_cast<double>(f1), 1.000001, epsilon);

    // Very small multiplication - may result in underflow
    Fixed f2(0.000001);
    f2 *= Fixed(0.000001);
    // For very small results, check if it's close to zero
    EXPECT_NEAR(static_cast<double>(f2), 0.0, epsilon);
}

// Integer operations tests with diverse values
TEST(Fixed64ArithmeticTest, IntegerOperations) {
    using Fixed = math::fp::Fixed64<16>;

    // Calculate epsilon for precision comparisons
    double epsilon = static_cast<double>(Fixed::Epsilon());

    // Positive fixed-point with positive integers
    Fixed d(10.5);
    EXPECT_NEAR(static_cast<double>(d + 5), 15.5, epsilon);
    EXPECT_NEAR(static_cast<double>(d - 3), 7.5, epsilon);
    EXPECT_NEAR(static_cast<double>(d * 2), 21.0, epsilon);
    EXPECT_NEAR(static_cast<double>(d / 2), 5.25, epsilon);

    // Reverse operations with integers
    EXPECT_NEAR(static_cast<double>(5 + d), 15.5, epsilon);
    EXPECT_NEAR(static_cast<double>(15 - d), 4.5, epsilon);
    EXPECT_NEAR(static_cast<double>(2 * d), 21.0, epsilon);
    EXPECT_NEAR(static_cast<double>(21 / d), 2.0, epsilon);

    // Negative fixed-point with positive integers
    Fixed e(-10.5);
    EXPECT_NEAR(static_cast<double>(e + 5), -5.5, epsilon);
    EXPECT_NEAR(static_cast<double>(e - 3), -13.5, epsilon);
    EXPECT_NEAR(static_cast<double>(e * 2), -21.0, epsilon);
    EXPECT_NEAR(static_cast<double>(e / 2), -5.25, epsilon);

    // Positive fixed-point with negative integers
    EXPECT_NEAR(static_cast<double>(d + (-5)), 5.5, epsilon);
    EXPECT_NEAR(static_cast<double>(d - (-3)), 13.5, epsilon);
    EXPECT_NEAR(static_cast<double>(d * (-2)), -21.0, epsilon);
    EXPECT_NEAR(static_cast<double>(d / (-2)), -5.25, epsilon);

    // Large integer operations
    Fixed f(0.5);
    EXPECT_NEAR(static_cast<double>(f * 1000000), 500000.0, epsilon);
    EXPECT_NEAR(static_cast<double>(f / 1000000), 0.0000005, epsilon);

    // Integer operations with values near limits
    Fixed g(100000.0);
    EXPECT_NEAR(static_cast<double>(g * 10), 1000000.0, epsilon);

    // Very small fixed-point with integers
    Fixed h = Fixed::One() / 1000;
    EXPECT_NEAR(static_cast<double>(h * 1e4), static_cast<double>(h) * 1e4, epsilon)
        << "h = " << h << " h * 1e4 = " << h * 1e4;
    EXPECT_NEAR(static_cast<double>(1e4 * h), static_cast<double>(h) * 1e4, epsilon)
        << "h = " << h << " 1e4 * h = " << 1e4 * h;
}

// Modulo and remainder operations with diverse values
TEST(Fixed64ArithmeticTest, ModuloOperations) {
    using Fixed = math::fp::Fixed64<16>;

    // Calculate epsilon for precision comparisons
    double epsilon = static_cast<double>(Fixed::Epsilon());

    // Positive values
    Fixed a(10.5);
    Fixed b(3.0);
    EXPECT_NEAR(static_cast<double>(a % b), 1.5, epsilon);

    // Negative dividend
    Fixed c(-10.5);
    EXPECT_NEAR(static_cast<double>(c % b), -1.5, epsilon);

    // Negative divisor
    EXPECT_NEAR(static_cast<double>(a % Fixed(-3.0)), 1.5, epsilon);

    // Both negative
    EXPECT_NEAR(static_cast<double>(c % Fixed(-3.0)), -1.5, epsilon);

    // Modulo with integer
    EXPECT_NEAR(static_cast<double>(a % 3), 1.5, epsilon);
    EXPECT_NEAR(static_cast<double>(10 % a), 10.0, epsilon);

    // Large values
    Fixed d(1000.75);
    Fixed e(10.5);
    EXPECT_NEAR(static_cast<double>(d % e),
                std::fmod(static_cast<double>(d), static_cast<double>(e)),
                epsilon);

    // Very small values
    Fixed f(0.0075);
    Fixed g(0.002);
    EXPECT_NEAR(static_cast<double>(f % g),
                std::fmod(static_cast<double>(f), static_cast<double>(g)),
                epsilon);

    // Divisor larger than dividend
    EXPECT_NEAR(static_cast<double>(b % a),
                std::fmod(static_cast<double>(b), static_cast<double>(a)),
                epsilon);

    // Modulo with very large values
    Fixed h(100000.5);
    Fixed i(10000.25);
    EXPECT_NEAR(static_cast<double>(h % i),
                std::fmod(static_cast<double>(h), static_cast<double>(i)),
                epsilon);
}

// Edge case arithmetic tests with diverse values
TEST(Fixed64ArithmeticTest, EdgeCases) {
    using Fixed = math::fp::Fixed64<16>;

    // Calculate epsilon for precision comparisons
    double epsilon = static_cast<double>(Fixed::Epsilon());

    // Division by zero
    Fixed a(5.5);
    Fixed zero = Fixed::Zero();
    EXPECT_TRUE(std::isinf(a / zero));

    // Division of zero by non-zero
    EXPECT_NEAR(static_cast<double>(zero / a), 0.0, epsilon);

    // Operations with NaN
    Fixed nan = Fixed::NaN();
    EXPECT_TRUE(!std::isnan(nan + a));
    EXPECT_TRUE(!std::isnan(nan - a));
    EXPECT_TRUE(!std::isnan(nan * a));
    EXPECT_TRUE(!std::isnan(nan / a));

    // Operations with negative values and NaN
    Fixed negVal(-5.5);
    EXPECT_TRUE(!std::isnan(nan + negVal));
    EXPECT_TRUE(!std::isnan(nan - negVal));
    EXPECT_TRUE(!std::isnan(nan * negVal));
    EXPECT_TRUE(!std::isnan(nan / negVal));

    // Operations with infinity
    Fixed inf = Fixed::Infinity();
    EXPECT_TRUE(!std::isinf(inf + a));
    EXPECT_TRUE(!std::isinf(inf * a));
    EXPECT_TRUE(!std::isnan(inf - inf));
    EXPECT_TRUE(!std::isnan(inf / inf));
    EXPECT_TRUE(std::isinf(a / Fixed::Zero()));

    // Operations with negative infinity
    Fixed negInf = Fixed::NegInfinity();
    EXPECT_TRUE(!std::isinf(negInf + a));
    EXPECT_TRUE(!std::isinf(negInf * a));
    EXPECT_TRUE(!std::isnan(negInf - negInf));

    // Operations with max and min values
    Fixed max = Fixed::Max();
    Fixed min = Fixed::Min();
    EXPECT_NEAR(static_cast<double>(max + min), 0.0, 1.0);

    // Operations with epsilon
    Fixed eps = Fixed::Epsilon();
    EXPECT_GT(a + eps, a);
    EXPECT_LT(a - eps, a);

    // Operations with values very close to zero
    Fixed almostZero(1.0e-10);
    EXPECT_NEAR(static_cast<double>(almostZero * almostZero), 0.0, 1e-15);
    EXPECT_NEAR(static_cast<double>(a + almostZero), static_cast<double>(a), 1e-9);
}

// Test for precision at different scales
TEST(Fixed64ArithmeticTest, PrecisionAtDifferentScales) {
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;
    using Fixed40 = math::fp::Fixed64<40>;

    // Calculate epsilon for each precision
    double epsilon16 = static_cast<double>(Fixed16::Epsilon());
    double epsilon32 = static_cast<double>(Fixed32::Epsilon());
    double epsilon40 = static_cast<double>(Fixed40::Epsilon());

    // Test precision with small values
    Fixed16 a16(0.0001);
    Fixed32 a32(0.0001);
    Fixed40 a40(0.0001);

    EXPECT_NEAR(static_cast<double>(a16 * a16), 0.00000001, epsilon16);
    EXPECT_NEAR(static_cast<double>(a32 * a32), 0.00000001, epsilon32);
    EXPECT_NEAR(static_cast<double>(a40 * a40), 0.00000001, epsilon40);

    // Test precision with large values
    Fixed16 b16(10000.0);
    Fixed32 b32(10000.0);
    Fixed40 b40(1000.0);  // Smaller range for higher precision

    EXPECT_NEAR(static_cast<double>(b16 * b16), 100000000.0, epsilon16);
    EXPECT_NEAR(static_cast<double>(b32 * b32), 100000000.0, epsilon32);
    EXPECT_NEAR(static_cast<double>(b40 * b40), 1000000.0, epsilon40);

    // Test division precision
    Fixed16 c16(1.0);
    Fixed32 c32(1.0);
    Fixed40 c40(1.0);

    EXPECT_NEAR(static_cast<double>(c16 / Fixed16(3.0)), 0.333333, epsilon16);
    EXPECT_NEAR(static_cast<double>(c32 / Fixed32(3.0)), 0.333333333333, epsilon32);
    EXPECT_NEAR(static_cast<double>(c40 / Fixed40(3.0)), 0.333333333333333, epsilon40);
}

// Test for Epsilon behavior
TEST(Fixed64ArithmeticTest, EpsilonBehavior) {
    using Fixed = math::fp::Fixed64<16>;

    // Get epsilon value - the smallest representable positive value
    Fixed eps = Fixed::Epsilon();

    // Epsilon should be positive
    EXPECT_GT(static_cast<double>(eps), 0.0);

    // Adding epsilon to zero should result in epsilon
    EXPECT_EQ(Fixed::Zero() + eps, eps);

    // Adding epsilon to a value should result in a slightly larger value
    Fixed a(1.0);
    EXPECT_GT(a + eps, a);

    // Subtracting epsilon from a value should result in a slightly smaller value
    EXPECT_LT(a - eps, a);

    // Multiplying by (1 + epsilon) should increase the value
    Fixed oneWithEps = Fixed::One() + eps;
    EXPECT_GT(a * oneWithEps, a);

    // Test that epsilon is the smallest representable increment
    Fixed justBelowEps(eps.value() - 1, detail::nothing{});
    EXPECT_EQ(justBelowEps, Fixed::Zero());

    // Test that epsilon + epsilon > epsilon
    EXPECT_GT(eps + eps, eps);

    // Test that epsilon is consistent with the definition (2^-P)
    double epsilon = static_cast<double>(Fixed::Epsilon());
    double expectedEpsilon = std::pow(2.0, -16);  // For Fixed64<16>
    EXPECT_NEAR(static_cast<double>(eps), expectedEpsilon, epsilon);
}

}  // namespace math::fp::tests