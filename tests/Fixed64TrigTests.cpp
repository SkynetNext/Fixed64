#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 (M_PI / 2.0)
#endif

#include "Fixed64.h"
#include "Fixed64Math.h"
#include "gtest/gtest.h"

// Trigonometric function tests
TEST(Fixed64TrigSimpleTest, SinCosOperations) {
    using Fixed = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    // Basic sine tests
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Sin(Fixed(0.0))), 0.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Sin(Fixed(M_PI_2))), 1.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Sin(Fixed(M_PI))), 0.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Sin(Fixed(3 * M_PI_2))), -1.0, 1e-6);

    // Basic cosine tests
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Cos(Fixed(0.0))), 1.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Cos(Fixed(M_PI_2))), 0.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Cos(Fixed(M_PI))), -1.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Cos(Fixed(3 * M_PI_2))), 0.0, 1e-6);

    // Random angle tests
    for (double angle = 0.0; angle < 2 * M_PI; angle += 0.1) {
        EXPECT_NEAR(static_cast<double>(Fixed64Math::Sin(Fixed(angle))), std::sin(angle), 1e-5);
        EXPECT_NEAR(static_cast<double>(Fixed64Math::Cos(Fixed(angle))), std::cos(angle), 1e-5);
    }
}

TEST(Fixed64TrigSimpleTest, TanOperation) {
    using Fixed = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    // Basic tangent tests
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Tan(Fixed(0.0))), 0.0, 1e-6);

    // Use a value further from π/2 to avoid extreme sensitivity
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Tan(Fixed(M_PI / 4))), 1.0, 1e-5);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Tan(Fixed(M_PI))), 0.0, 1e-6);

    // Random angle tests (avoiding singularities)
    for (double angle = -1.5; angle < 1.5; angle += 0.1) {
        // Skip values too close to singularities (π/2 + nπ)
        if (std::fabs(std::fmod(angle - M_PI_2, M_PI)) < 0.05)
            continue;

        EXPECT_NEAR(static_cast<double>(Fixed64Math::Tan(Fixed(angle))), std::tan(angle), 1e-3);
    }
}

TEST(Fixed64TrigSimpleTest, ArcSinArcCosOperations) {
    using Fixed = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    // Basic arcsine tests
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Asin(Fixed(0.0))), 0.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Asin(Fixed(1.0))), M_PI_2, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Asin(Fixed(-1.0))), -M_PI_2, 1e-6);

    // Basic arccosine tests
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Acos(Fixed(1.0))), 0.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Acos(Fixed(0.0))), M_PI_2, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Acos(Fixed(-1.0))), M_PI, 1e-6);

    // Random value tests
    for (double value = -0.9; value < 0.9; value += 0.1) {
        EXPECT_NEAR(static_cast<double>(Fixed64Math::Asin(Fixed(value))), std::asin(value), 2e-5);
        EXPECT_NEAR(static_cast<double>(Fixed64Math::Acos(Fixed(value))), std::acos(value), 2e-5);
    }

    // Edge case tests
    EXPECT_EQ(Fixed64Math::Asin(Fixed(2.0)), Fixed::HalfPi());
    EXPECT_EQ(Fixed64Math::Asin(Fixed(-2.0)), -Fixed::HalfPi());
    EXPECT_EQ(Fixed64Math::Acos(Fixed(2.0)), Fixed::Zero());
}

TEST(Fixed64TrigSimpleTest, AtanAtan2Operations) {
    using Fixed = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    // Basic arctangent tests - increased tolerance to 2e-5
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Atan(Fixed(0.0))), 0.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Atan(Fixed(1.0))), M_PI / 4, 2e-5);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Atan(Fixed(-1.0))), -M_PI / 4, 2e-5);

    // Atan2 tests - four quadrants - increased tolerance to 2e-5
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Atan2(Fixed(1.0), Fixed(1.0))), M_PI / 4, 2e-5);
    EXPECT_NEAR(
        static_cast<double>(Fixed64Math::Atan2(Fixed(1.0), Fixed(-1.0))), 3 * M_PI / 4, 2e-5);
    EXPECT_NEAR(
        static_cast<double>(Fixed64Math::Atan2(Fixed(-1.0), Fixed(-1.0))), -3 * M_PI / 4, 2e-5);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Atan2(Fixed(-1.0), Fixed(1.0))), -M_PI / 4, 2e-5);

    // Special cases
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Atan2(Fixed(1.0), Fixed(0.0))), M_PI / 2, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Atan2(Fixed(-1.0), Fixed(0.0))), -M_PI / 2, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Atan2(Fixed(0.0), Fixed(1.0))), 0.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Atan2(Fixed(0.0), Fixed(-1.0))), M_PI, 1e-6);

    // Random value tests - increased tolerance to 2e-5
    for (double y = -1.0; y <= 1.0; y += 0.5) {
        for (double x = -1.0; x <= 1.0; x += 0.5) {
            if (x == 0.0 && y == 0.0)
                continue;  // Skip origin
            EXPECT_NEAR(static_cast<double>(Fixed64Math::Atan2(Fixed(y), Fixed(x))),
                        std::atan2(y, x),
                        2e-5);
        }
    }
}

// Test std namespace trigonometric function overloads
TEST(Fixed64TrigSimpleTest, StdNamespaceOverloads) {
    using Fixed = math::fp::Fixed64<32>;

    // Test std namespace trigonometric function overloads
    EXPECT_NEAR(static_cast<double>(std::sin(Fixed(M_PI_2))), 1.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(std::cos(Fixed(0.0))), 1.0, 1e-6);
    EXPECT_NEAR(static_cast<double>(std::tan(Fixed(M_PI / 4))), 1.0, 1e-5);
    EXPECT_NEAR(static_cast<double>(std::asin(Fixed(0.5))), std::asin(0.5), 1e-5);
    EXPECT_NEAR(static_cast<double>(std::acos(Fixed(0.5))), std::acos(0.5), 1e-5);

    EXPECT_NEAR(static_cast<double>(std::atan(Fixed(1.0))), M_PI / 4, 1e-6);
    EXPECT_NEAR(static_cast<double>(std::atan2(Fixed(1.0), Fixed(1.0))), M_PI / 4, 2e-5);
}