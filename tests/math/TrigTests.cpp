#include <cmath>
#include <limits>
#include <vector>

#include "Fixed64.h"
#include "Fixed64Math.h"
#include "FixedTrigLut.h"
#include "gtest/gtest.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 (M_PI / 2.0)
#endif

namespace math::fp::tests {

// Helper function to check if values are within relative error bounds
// For values close to zero, uses absolute error instead
inline bool IsWithinRelativeError(double actual,
                                  double expected,
                                  double max_relative_error = 0.0001) {
    // Increase threshold for "near zero" determination to handle sin(π), etc.
    if (std::abs(expected) < 1e-6) {
        // For values near zero, use absolute error of 1e-6
        return std::abs(actual) < 1e-6;
    }
    double relative_error = std::abs((actual - expected) / expected);
    return relative_error <= max_relative_error;
}

// Macro that provides detailed error information when test fails
#define EXPECT_RELATIVE_ERROR(actual, expected, max_rel_error, message)                    \
    EXPECT_TRUE(IsWithinRelativeError(actual, expected, max_rel_error))                    \
        << message                                                                         \
        << " - Relative error: " << std::abs(((actual) - (expected)) / (expected)) * 100.0 \
        << "%, Actual: " << (actual) << ", Expected: " << (expected)

class Fixed64TrigTest : public ::testing::Test {
 protected:
    // Use appropriate precision constant
    using Fixed = math::fp::Fixed64<32>;  // Trigonometric functions need 32-bit precision

    // Follow best practices using Epsilon as error bound
    static constexpr double epsilon32 = static_cast<double>(Fixed::Epsilon());

    // Special angle values (π/6, π/4, π/3)
    const Fixed pi_6 = Fixed::Pi() / Fixed(6.0);
    const Fixed pi_4 = Fixed::QuarterPi();
    const Fixed pi_3 = Fixed::Pi() / Fixed(3.0);

    // Small angle value (0.01 degrees in radians)
    const Fixed small_angle = Fixed::Pi() / Fixed(180.0) * Fixed(0.01);

    // Test angles in all quadrants
    std::vector<Fixed> testAngles = {
        Fixed::Zero(),                  // 0
        small_angle,                    // 0.01 degrees
        pi_6,                           // π/6
        pi_4,                           // π/4
        pi_3,                           // π/3
        Fixed::HalfPi(),                // π/2
        Fixed::Pi() - pi_3,             // 2π/3
        Fixed::Pi() - pi_4,             // 3π/4
        Fixed::Pi() - pi_6,             // 5π/6
        Fixed::Pi(),                    // π
        Fixed::Pi() + pi_6,             // 7π/6
        Fixed::Pi() + pi_4,             // 5π/4
        Fixed::Pi() + pi_3,             // 4π/3
        Fixed::Pi() + Fixed::HalfPi(),  // 3π/2
        Fixed::TwoPi() - pi_3,          // 5π/3
        Fixed::TwoPi() - pi_4,          // 7π/4
        Fixed::TwoPi() - pi_6,          // 11π/6
        Fixed::TwoPi() - small_angle,   // 360-0.01 degrees
        Fixed::TwoPi()                  // 2π
    };
};

// Basic trigonometric function tests (sine, cosine, tangent)
TEST_F(Fixed64TrigTest, BasicTrigonometricFunctions) {
    for (const auto& angle : testAngles) {
        double dblAngle = static_cast<double>(angle);

        // Normalize angle for more stable comparison
        double normalizedAngle = std::fmod(dblAngle, 2 * M_PI);
        if (normalizedAngle < 0)
            normalizedAngle += 2 * M_PI;

        // Special case handling for critical angles
        bool isSpecialAngle = false;

        // Check if angle is near a multiple of π
        double angleMod = std::fmod(std::abs(normalizedAngle), M_PI);
        if (angleMod < 1e-6 || std::abs(angleMod - M_PI) < 1e-6) {
            // Near 0, π, 2π, etc. - sin should be near 0
            isSpecialAngle = true;
            double actualSin = static_cast<double>(Fixed64Math::Sin(angle));
            EXPECT_NEAR(actualSin, 0.0, 1e-6)
                << "Sin should be near 0 at angle " << dblAngle << " radians";

            double actualTan = static_cast<double>(Fixed64Math::Tan(angle));
            EXPECT_NEAR(actualTan, 0.0, 1e-6)
                << "Tan should be near 0 at angle " << dblAngle << " radians";
        }

        // Check if angle is near a multiple of π/2
        double angleMod2 = std::fmod(std::abs(normalizedAngle), M_PI);
        if (!isSpecialAngle && std::abs(angleMod2 - M_PI / 2) < 1e-6) {
            // Near π/2, 3π/2, etc. - cos should be near 0
            isSpecialAngle = true;
            double actualCos = static_cast<double>(Fixed64Math::Cos(angle));
            EXPECT_NEAR(actualCos, 0.0, 1e-6)
                << "Cos should be near 0 at angle " << dblAngle << " radians";

            // Don't test tan at these points as it approaches infinity
        }

        // Regular tests for non-special angles
        if (!isSpecialAngle) {
            // Calculate expected values
            double expectedSin = std::sin(dblAngle);
            double expectedCos = std::cos(dblAngle);
            double expectedTan = std::tan(dblAngle);

            // Sine test
            double actualSin = static_cast<double>(Fixed64Math::Sin(angle));
            EXPECT_RELATIVE_ERROR(
                actualSin, expectedSin, 0.0001, "Sin failed at angle " << dblAngle << " radians");

            // Cosine test
            double actualCos = static_cast<double>(Fixed64Math::Cos(angle));
            EXPECT_RELATIVE_ERROR(
                actualCos, expectedCos, 0.0001, "Cos failed at angle " << dblAngle << " radians");

            // Tangent test (avoid infinite points)
            if (std::abs(std::cos(dblAngle)) > 0.01) {
                double actualTan = static_cast<double>(Fixed64Math::Tan(angle));
                EXPECT_RELATIVE_ERROR(actualTan,
                                      expectedTan,
                                      0.0001,
                                      "Tan failed at angle " << dblAngle << " radians");
            }
        }
    }
}

// Small angle approximation tests
TEST_F(Fixed64TrigTest, SmallAngleApproximations) {
    // Test very small angles where sin(x) ≈ x and cos(x) ≈ 1
    std::vector<double> smallDegrees = {0.01, 0.1, 0.5, 1.0};

    for (double degrees : smallDegrees) {
        // Convert degrees to radians
        Fixed angle = Fixed::Pi() / Fixed(180.0) * Fixed(degrees);
        double dblAngle = static_cast<double>(angle);

        // For very small angles, sin(x) ≈ x
        double actualSin = static_cast<double>(Fixed64Math::Sin(angle));
        EXPECT_RELATIVE_ERROR(actualSin,
                              std::sin(dblAngle),
                              0.0001,
                              "Sin approximation failed at " << degrees << " degrees");

        // For very small angles, verify sin(x) ≈ x approximation
        if (degrees <= 0.1) {
            EXPECT_RELATIVE_ERROR(
                actualSin,
                dblAngle,
                0.001,
                "Small angle approximation sin(x) ≈ x failed at " << degrees << " degrees");
        }

        // For very small angles, cos(x) ≈ 1
        double actualCos = static_cast<double>(Fixed64Math::Cos(angle));
        EXPECT_RELATIVE_ERROR(actualCos,
                              std::cos(dblAngle),
                              0.0001,
                              "Cos approximation failed at " << degrees << " degrees");

        // For very small angles, verify cos(x) ≈ 1 approximation
        if (degrees <= 0.1) {
            EXPECT_RELATIVE_ERROR(
                actualCos,
                1.0,
                0.0001,
                "Small angle approximation cos(x) ≈ 1 failed at " << degrees << " degrees");
        }

        // For very small angles, tan(x) ≈ x
        double actualTan = static_cast<double>(Fixed64Math::Tan(angle));
        EXPECT_RELATIVE_ERROR(actualTan,
                              std::tan(dblAngle),
                              0.0001,
                              "Tan approximation failed at " << degrees << " degrees");

        if (degrees <= 0.1) {
            EXPECT_RELATIVE_ERROR(
                actualTan,
                dblAngle,
                0.001,
                "Small angle approximation tan(x) ≈ x failed at " << degrees << " degrees");
        }
    }
}

// Inverse trigonometric function tests
TEST_F(Fixed64TrigTest, InverseTrigonometricFunctions) {
    // Test values from -1 to 1 range
    std::vector<Fixed> testValues = {Fixed(-1.0),
                                     Fixed(-0.9),
                                     Fixed(-0.7),
                                     Fixed(-0.5),
                                     Fixed(-0.3),
                                     Fixed(-0.1),
                                     Fixed::Zero(),
                                     Fixed(0.1),
                                     Fixed(0.3),
                                     Fixed(0.5),
                                     Fixed(0.7),
                                     Fixed(0.9),
                                     Fixed(1.0)};

    for (const auto& value : testValues) {
        double dblValue = static_cast<double>(value);

        // Arcsine test
        if (dblValue >= -1.0 && dblValue <= 1.0) {
            double expectedAsin = std::asin(dblValue);
            double actualAsin = static_cast<double>(Fixed64Math::Asin(value));
            EXPECT_RELATIVE_ERROR(
                actualAsin, expectedAsin, 0.0001, "Asin failed at value " << dblValue);
        }

        // Arccosine test
        if (dblValue >= -1.0 && dblValue <= 1.0) {
            double expectedAcos = std::acos(dblValue);
            double actualAcos = static_cast<double>(Fixed64Math::Acos(value));
            EXPECT_RELATIVE_ERROR(
                actualAcos, expectedAcos, 0.0001, "Acos failed at value " << dblValue);
        }

        // Arctangent test
        double expectedAtan = std::atan(dblValue);
        double actualAtan = static_cast<double>(Fixed64Math::Atan(value));
        EXPECT_RELATIVE_ERROR(
            actualAtan, expectedAtan, 0.0002, "Atan failed at value " << dblValue);
    }
}

// Atan2 function test
TEST_F(Fixed64TrigTest, Atan2Function) {
    // Test points in different quadrants
    struct Point {
        Fixed x;
        Fixed y;
    };

    std::vector<Point> testPoints = {
        {Fixed(1.0), Fixed(0.0)},    // 0 degrees
        {Fixed(1.0), Fixed(1.0)},    // 45 degrees
        {Fixed(0.0), Fixed(1.0)},    // 90 degrees
        {Fixed(-1.0), Fixed(1.0)},   // 135 degrees
        {Fixed(-1.0), Fixed(0.0)},   // 180 degrees
        {Fixed(-1.0), Fixed(-1.0)},  // -135 degrees
        {Fixed(0.0), Fixed(-1.0)},   // -90 degrees
        {Fixed(1.0), Fixed(-1.0)},   // -45 degrees
        {Fixed(0.0), Fixed(0.0)},    // Special case: origin
    };

    for (const auto& point : testPoints) {
        double x = static_cast<double>(point.x);
        double y = static_cast<double>(point.y);

        double expectedAtan2 = std::atan2(y, x);
        double actualAtan2 = static_cast<double>(Fixed64Math::Atan2(point.y, point.x));

        EXPECT_RELATIVE_ERROR(
            actualAtan2, expectedAtan2, 0.0001, "Atan2 failed at point (" << x << ", " << y << ")");
    }
}

// Trigonometric identity tests
TEST_F(Fixed64TrigTest, TrigonometricIdentities) {
    for (const auto& angle : testAngles) {
        // Pythagorean identity: sin²(θ) + cos²(θ) = 1
        Fixed sinVal = Fixed64Math::Sin(angle);
        Fixed cosVal = Fixed64Math::Cos(angle);
        Fixed identity1 = (sinVal * sinVal) + (cosVal * cosVal);
        EXPECT_RELATIVE_ERROR(
            static_cast<double>(identity1),
            1.0,
            0.0001,
            "Pythagorean identity failed at angle " << static_cast<double>(angle));

        // Tangent identity: tan(θ) = sin(θ)/cos(θ)
        if (std::abs(static_cast<double>(cosVal)) > 0.01) {
            Fixed tanVal = Fixed64Math::Tan(angle);
            Fixed identity2 = sinVal / cosVal;
            EXPECT_RELATIVE_ERROR(
                static_cast<double>(tanVal),
                static_cast<double>(identity2),
                0.0001,
                "Tangent identity failed at angle " << static_cast<double>(angle));
        }

        // Double angle formula: cos(2θ) = cos²(θ) - sin²(θ)
        Fixed cos2x = Fixed64Math::Cos(angle * Fixed(2.0));
        Fixed identity3 = (cosVal * cosVal) - (sinVal * sinVal);
        EXPECT_RELATIVE_ERROR(
            static_cast<double>(cos2x),
            static_cast<double>(identity3),
            0.0001,
            "Double angle cosine identity failed at " << static_cast<double>(angle));
    }
}

// Edge case tests
TEST_F(Fixed64TrigTest, EdgeCases) {
    // Near π/2 tangent (should be very large but not overflow)
    Fixed nearPiOver2 = Fixed::HalfPi() - Fixed::Epsilon() * Fixed(10);
    EXPECT_GT(static_cast<double>(Fixed64Math::Tan(nearPiOver2)), 100.0)
        << "Tan should be very large near π/2";

    // Inverse trigonometric function boundary tests
    EXPECT_EQ(Fixed64Math::Asin(Fixed(1.1)), Fixed::HalfPi()) << "Asin should clamp values > 1.0";
    EXPECT_EQ(Fixed64Math::Asin(Fixed(-1.1)), -Fixed::HalfPi())
        << "Asin should clamp values < -1.0";

    EXPECT_EQ(Fixed64Math::Acos(Fixed(1.1)), Fixed::Zero()) << "Acos should clamp values > 1.0";
    EXPECT_EQ(Fixed64Math::Acos(Fixed(-1.1)), Fixed::Pi()) << "Acos should clamp values < -1.0";

    // Atan2 special case at origin
    EXPECT_EQ(Fixed64Math::Atan2(Fixed::Zero(), Fixed::Zero()), Fixed::Zero())
        << "Atan2(0,0) should return 0";
}

// Lookup table implementation specific tests
TEST_F(Fixed64TrigTest, LookupTableImplementation) {
    // Test small increments for lookup table changes
    for (double base = 0.0; base < 6.0; base += 0.5) {
        Fixed angle1(base);
        Fixed angle2 = angle1 + Fixed::Epsilon();

        // Check if two very close values produce close results
        double sin1 = static_cast<double>(Fixed64Math::Sin(angle1));
        double sin2 = static_cast<double>(Fixed64Math::Sin(angle2));
        EXPECT_RELATIVE_ERROR(
            sin1, sin2, 0.0001, "Sin lookup table should be continuous at " << base);

        double cos1 = static_cast<double>(Fixed64Math::Cos(angle1));
        double cos2 = static_cast<double>(Fixed64Math::Cos(angle2));
        EXPECT_RELATIVE_ERROR(
            cos1, cos2, 0.0001, "Cos lookup table should be continuous at " << base);
    }

    // Test function periodicity
    for (int i = 0; i < 10; i++) {
        Fixed angle(i * 0.1);
        Fixed angle_plus_2pi = angle + Fixed::TwoPi();

        // Check sin(x) = sin(x + 2π)
        double sin1 = static_cast<double>(Fixed64Math::Sin(angle));
        double sin2 = static_cast<double>(Fixed64Math::Sin(angle_plus_2pi));
        EXPECT_RELATIVE_ERROR(
            sin1, sin2, 0.0001, "Sin should be periodic at " << static_cast<double>(angle));

        // Check cos(x) = cos(x + 2π)
        double cos1 = static_cast<double>(Fixed64Math::Cos(angle));
        double cos2 = static_cast<double>(Fixed64Math::Cos(angle_plus_2pi));
        EXPECT_RELATIVE_ERROR(
            cos1, cos2, 0.0001, "Cos should be periodic at " << static_cast<double>(angle));

        // Check tan(x) = tan(x + π)
        if (std::abs(std::cos(static_cast<double>(angle))) > 0.1) {
            double tan1 = static_cast<double>(Fixed64Math::Tan(angle));
            double tan2 = static_cast<double>(Fixed64Math::Tan(angle_plus_2pi));
            EXPECT_RELATIVE_ERROR(
                tan1, tan2, 0.0001, "Tan should be periodic at " << static_cast<double>(angle));
        }
    }

    // Test sign-change properties
    for (double angle = 0.1; angle < Fixed::Pi(); angle += 0.2) {
        Fixed angleFP(angle);
        Fixed neg_angleFP = -angleFP;

        // Check sin(-x) = -sin(x)
        double sin_pos = static_cast<double>(Fixed64Math::Sin(angleFP));
        double sin_neg = static_cast<double>(Fixed64Math::Sin(neg_angleFP));
        EXPECT_RELATIVE_ERROR(
            sin_neg, -sin_pos, 0.0001, "Sin(-x) should equal -Sin(x) at " << angle);

        // Check cos(-x) = cos(x)
        double cos_pos = static_cast<double>(Fixed64Math::Cos(angleFP));
        double cos_neg = static_cast<double>(Fixed64Math::Cos(neg_angleFP));
        EXPECT_RELATIVE_ERROR(cos_neg, cos_pos, 0.0001, "Cos(-x) should equal Cos(x) at " << angle);

        // Check tan(-x) = -tan(x)
        if (std::abs(std::cos(angle)) > 0.1) {
            double tan_pos = static_cast<double>(Fixed64Math::Tan(angleFP));
            double tan_neg = static_cast<double>(Fixed64Math::Tan(neg_angleFP));
            EXPECT_RELATIVE_ERROR(
                tan_neg, -tan_pos, 0.0001, "Tan(-x) should equal -Tan(x) at " << angle);
        }
    }
}

}  // namespace math::fp::tests