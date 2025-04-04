#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 (M_PI / 2.0)
#endif

#include "Fixed64.h"
#include "Fixed64Math.h"
#include "gtest/gtest.h"

// Basic mathematical utility function tests
TEST(Fixed64MathTest, MinMaxOperations) {
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    // Basic value tests
    EXPECT_EQ(Fixed64Math::Max(Fixed16(1.5), Fixed16(0.5)), Fixed16(1.5));
    EXPECT_EQ(Fixed64Math::Min(Fixed16(-0.5), Fixed16(0.5)), Fixed16(-0.5));

    // Equal value tests
    Fixed16 sameValue(3.1415);
    EXPECT_EQ(Fixed64Math::Max(sameValue, sameValue), sameValue);
    EXPECT_EQ(Fixed64Math::Min(sameValue, sameValue), sameValue);

    // Mixed precision tests
    Fixed32 highPrec(2.71828);
    Fixed16 lowPrec(2.71828);

    // Convert to higher precision
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Max(highPrec, Fixed32(lowPrec))),
                2.71828,
                static_cast<double>(Fixed32::Epsilon()));
}

TEST(Fixed64MathTest, AbsClampOperations) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Absolute value tests
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Abs(Fixed(-2.5))), 2.5, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Abs(Fixed::Zero())), 0.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Abs(Fixed(3.7))), 3.7, 1e-5);

    // Clamp tests
    EXPECT_EQ(Fixed64Math::Clamp(Fixed(3.0), Fixed(1.0), Fixed(2.0)), Fixed(2.0));
    EXPECT_EQ(Fixed64Math::Clamp(Fixed(-1.0), Fixed(1.0), Fixed(3.0)), Fixed(1.0));
    EXPECT_EQ(Fixed64Math::Clamp(Fixed(2.0), Fixed(1.0), Fixed(3.0)), Fixed(2.0));

    // Clamp01 tests
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed(1.5)), Fixed::One());
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed(-0.5)), Fixed::Zero());
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed(0.5)), Fixed(0.5));
}

TEST(Fixed64MathTest, RoundingFunctions) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Floor tests
    EXPECT_EQ(static_cast<double>(Fixed64Math::Floor(Fixed(2.999))), 2.0);
    EXPECT_EQ(static_cast<double>(Fixed64Math::Floor(Fixed(-2.1))), -3.0);
    EXPECT_EQ(static_cast<double>(Fixed64Math::Floor(Fixed(3.0))), 3.0);

    // Ceiling tests
    EXPECT_EQ(static_cast<double>(Fixed64Math::Ceil(Fixed(2.1))), 3.0);
    EXPECT_EQ(static_cast<double>(Fixed64Math::Ceil(Fixed(-2.999))), -2.0);
    EXPECT_EQ(static_cast<double>(Fixed64Math::Ceil(Fixed(3.0))), 3.0);

    // Rounding tests
    EXPECT_EQ(static_cast<double>(Fixed64Math::Round(Fixed(2.6))), 3.0);
    EXPECT_EQ(static_cast<double>(Fixed64Math::Round(Fixed(2.4))), 2.0);
    EXPECT_EQ(static_cast<double>(Fixed64Math::Round(Fixed(-2.6))), -3.0);
    EXPECT_EQ(static_cast<double>(Fixed64Math::Round(Fixed(-2.4))), -2.0);
    EXPECT_EQ(static_cast<double>(Fixed64Math::Round(Fixed(2.5))), 3.0);
    EXPECT_EQ(static_cast<double>(Fixed64Math::Round(Fixed(-2.5))), -3.0)
        << "Fixed(-2.5).value()=" << Fixed(-2.5).value() << " HalfValue=" << Fixed::Half().value();

    // Truncation tests
    EXPECT_EQ(static_cast<double>(Fixed64Math::Trunc(Fixed(2.999))), 2.0);
    EXPECT_EQ(static_cast<double>(Fixed64Math::Trunc(Fixed(-2.999))), -2.0);
    EXPECT_EQ(static_cast<double>(Fixed64Math::Trunc(Fixed(0.0))), 0.0);
}

TEST(Fixed64MathTest, FractionsFunction) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test fractional part of positive numbers
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Fractions(Fixed(2.75))), 0.75, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Fractions(Fixed(3.25))), 0.25, 1e-6);

    // Test fractional part of negative numbers
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Fractions(Fixed(-2.25))), 0.75, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Fractions(Fixed(-3.75))), 0.25, 1e-6);

    // Test edge cases
    EXPECT_EQ(Fixed64Math::Fractions(Fixed::Zero()), Fixed::Zero());
    EXPECT_EQ(Fixed64Math::Fractions(Fixed(2.0)), Fixed::Zero());
}

TEST(Fixed64MathTest, InterpolationFunctions) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Linear interpolation
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(Fixed(2), Fixed(3), Fixed(0.5))), 2.5, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(Fixed(2), Fixed(3), Fixed(1.5))), 3.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(Fixed(2), Fixed(3), Fixed(-0.5))), 2.0, 1e-6);

    // Unclamped linear interpolation
    EXPECT_NEAR(
        static_cast<double>(Fixed64Math::LerpUnclamped(Fixed(2), Fixed(3), Fixed(2.0))), 4.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::LerpUnclamped(Fixed(2), Fixed(3), Fixed(-1.0))),
                1.0,
                1e-6);

    // Inverse linear interpolation
    EXPECT_NEAR(
        static_cast<double>(Fixed64Math::InverseLerp(Fixed(2), Fixed(4), Fixed(3))), 0.5, 1e-6)
        << "Expected 0.5, got " << Fixed64Math::InverseLerp(Fixed(2), Fixed(4), Fixed(3))
        << "x - a = " << Fixed(3) - Fixed(2) << " b - a = " << Fixed(4) - Fixed(2)
        << " (x - a) / (b - a) = " << (Fixed(3) - Fixed(2)) / (Fixed(4) - Fixed(2));
    EXPECT_EQ(Fixed64Math::InverseLerp(Fixed(2), Fixed(2), Fixed(3)), Fixed::Zero());
    EXPECT_EQ(Fixed64Math::InverseLerp(Fixed(2), Fixed(4), Fixed(1)), Fixed::Zero());
    EXPECT_EQ(Fixed64Math::InverseLerp(Fixed(2), Fixed(4), Fixed(5)), Fixed::One());
}

TEST(Fixed64MathTest, SpecialMathFunctions) {
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    // Square root tests
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Sqrt(Fixed16(25.0))), 5.0, 1e-9);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Sqrt(Fixed16(2.0))),
                std::sqrt(2.0),
                static_cast<double>(Fixed16::Epsilon()));
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Sqrt(Fixed16(0.0))), 0.0, 1e-9);

    // Exponential function tests
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Exp(Fixed32(1.0))), std::exp(1.0), 1e-9);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Exp(Fixed32(0.0))), 1.0, 1e-9);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Exp(Fixed32(2.0))), std::exp(2.0), 1e-9);

    // Logarithm function tests
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Log(Fixed32(1.0))), 0.0, 1e-9);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Log(Fixed32(2.0))), std::log(2.0), 1e-9);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Log(Fixed32(10.0))), std::log(10.0), 1e-9);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Log(Fixed32(std::exp(1.0)))), 1.0, 1e-9);

    // Nearly equal tests
    EXPECT_TRUE(Fixed64Math::IsNearlyEqual(Fixed16(1.0), Fixed16(1.0001), Fixed16(0.001)));
    EXPECT_FALSE(Fixed64Math::IsNearlyEqual(Fixed16(1.0), Fixed16(1.01), Fixed16(0.001)));
}

TEST(Fixed64MathTest, UtilityFunctions) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Sign function
    EXPECT_EQ(Fixed64Math::Sign(Fixed(0.1)), 1);
    EXPECT_EQ(Fixed64Math::Sign(Fixed(-0.1)), -1);
    EXPECT_EQ(Fixed64Math::Sign(Fixed(100.0)), 1);

    // Repeat mapping
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Repeat(Fixed(5.5), Fixed(4.0))), 1.5, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Repeat(Fixed(8.0), Fixed(4.0))), 0.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Repeat(Fixed(-1.5), Fixed(4.0))), 2.5, 1e-6);

    // Angle normalization
    Fixed angle(7.0);  // 7 radians ≈ 401°
    auto normalized = Fixed64Math::NormalizeAngle(angle);
    EXPECT_LT(normalized, Fixed::TwoPi());
    EXPECT_GE(normalized, Fixed::Zero());

    // Test negative angles
    Fixed negAngle(-1.0);
    auto normalizedNeg = Fixed64Math::NormalizeAngle(negAngle);
    EXPECT_LT(normalizedNeg, Fixed::TwoPi());
    EXPECT_GE(normalizedNeg, Fixed::Zero());
    EXPECT_NEAR(static_cast<double>(normalizedNeg), 2.0 * M_PI - 1.0, 1e-5);
}

TEST(Fixed64MathTest, AngleInterpolation) {
    using Fixed = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    // Basic angle interpolation
    EXPECT_NEAR(static_cast<double>(Fixed64Math::LerpAngle(Fixed(0.0), Fixed(M_PI), Fixed(0.5))),
                M_PI_2,
                1e-6);

    // Interpolation across 2π
    EXPECT_NEAR(
        static_cast<double>(Fixed64Math::LerpAngle(Fixed(0.0), Fixed(M_PI * 2), Fixed(1.0))),
        0.0,
        1e-6);

    // Choose shortest path
    EXPECT_NEAR(
        static_cast<double>(Fixed64Math::LerpAngle(Fixed(M_PI * 1.5), Fixed(M_PI * 2), Fixed(0.5))),
        M_PI * 1.75,
        1e-6);

    // Test reverse shortest path
    // When angles are far apart, LerpAngle chooses the shortest path around the circle
    // For angles 0.1 and M_PI*1.9, the shortest path is to go backwards
    // So instead of going from 0.1 to ~5.97, it goes from 0.1 to ~-0.1
    auto resultFixed = Fixed64Math::LerpAngle(Fixed(0.1), Fixed::Pi() * 1.9, Fixed::Half());
    double result = static_cast<double>(resultFixed);

    // Compare directly in fixed-point domain
    double start = 0.1;
    double end = M_PI * 1.9;
    double diff = end - start;
    if (diff > M_PI)
        diff -= 2 * M_PI;
    double expectedDouble = start + diff * 0.5;
    Fixed expectedFixed = Fixed(expectedDouble);

    bool isNearExpected =
        Fixed64Math::IsNearlyEqual(resultFixed, expectedFixed, Fixed(1e-6))
        || Fixed64Math::IsNearlyEqual(resultFixed, expectedFixed + Fixed::TwoPi(), Fixed(1e-6))
        || Fixed64Math::IsNearlyEqual(resultFixed + Fixed::TwoPi(), expectedFixed, Fixed(1e-6));

    EXPECT_TRUE(isNearExpected) << "Result: " << result
                                << ", Expected: " << static_cast<double>(expectedFixed);
}

TEST(Fixed64MathTest, Pow2Function) {
    using Fixed = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    // Integer power tests
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow2(Fixed(0.0))), 1.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow2(Fixed(1.0))), 2.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow2(Fixed(2.0))), 4.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow2(Fixed(3.0))), 8.0, 1e-6);

    // Fractional power tests - using relaxed tolerance due to fixed-point precision limitations
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow2(Fixed(0.5))), 1.4142135, 1e-6);  // √2
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow2(Fixed(1.5))), 2.8284271, 1e-6);  // 2√2

    // Negative power tests
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow2(Fixed(-1.0))), 0.5, 1e-9);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow2(Fixed(-2.0))), 0.25, 1e-9);

    // Edge case tests
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow2(Fixed(-30.0))), std::pow(2.0, -30.0), 1e-6);
    // Test overflow protection
    EXPECT_EQ(Fixed64Math::Pow2(Fixed(100.0)), Fixed::Max());
    EXPECT_EQ(Fixed64Math::Pow2(Fixed(-100.0)), Fixed::Zero());

    // Power function tests with various bases and exponents
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow(Fixed(2), Fixed(-1.0))), 0.5, 1e-9);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow(Fixed(2), Fixed(-2.0))), 0.25, 1e-7);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow(Fixed(2), Fixed(0.5))), std::sqrt(2.0), 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow(Fixed(3), Fixed(2.0))), 9.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow(Fixed(4), Fixed(0.5))), 2.0, 2e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow(Fixed(10), 3)), 1000.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow(Fixed(0.5), Fixed(2.0))), 0.25, 1e-7);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow(Fixed(0.1), Fixed(3.0))), 0.001, 1e-7);

    // Integer power tests - signed and unsigned versions
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow(Fixed(2), 3)), 8.0, 1e-9);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow(Fixed(2), -3)), 0.125, 1e-9);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow(Fixed(3), 4)), 81.0, 1e-9);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow(Fixed(3), -2)), 0.111111, 1e-6);

    // Unsigned integer power tests
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow(Fixed(2), 3u)), 8.0, 1e-9);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow(Fixed(3), 4u)), 81.0, 1e-9);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Pow(Fixed(1.5), 5u)), 7.59375, 1e-6);

    // Edge cases for integer powers
    EXPECT_EQ(Fixed64Math::Pow(Fixed(5), 0), Fixed::One());
    EXPECT_EQ(Fixed64Math::Pow(Fixed(5), 0u), Fixed::One());
    EXPECT_EQ(Fixed64Math::Pow(Fixed(0), 5), Fixed::Zero());
    EXPECT_EQ(Fixed64Math::Pow(Fixed(0), 5u), Fixed::Zero());
    EXPECT_EQ(Fixed64Math::Pow(Fixed(1), 100), Fixed::One());
    EXPECT_EQ(Fixed64Math::Pow(Fixed(1), 100u), Fixed::One());
}

TEST(Fixed64MathTest, ClampedCast) {
    using Fixed16 = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Normal range conversion
    EXPECT_NEAR(static_cast<double>(Fixed64Math::ClampedCast<Fixed16>(1.5)), 1.5, 1e-6);

    // Out-of-range conversion
    double maxValue = static_cast<double>(Fixed16::Max());
    double minValue = static_cast<double>(Fixed16::Min());

    EXPECT_EQ(Fixed64Math::ClampedCast<Fixed16>(maxValue * 2), Fixed16::Max());
    EXPECT_EQ(Fixed64Math::ClampedCast<Fixed16>(minValue * 2), Fixed16::Min());
}
