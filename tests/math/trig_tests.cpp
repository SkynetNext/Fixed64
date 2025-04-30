#include <cmath>
#include <limits>
#include <vector>

#include "fixed64.h"
#include "fixed64_math.h"
#include "gtest/gtest.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 (M_PI / 2.0)
#endif

namespace math::fp::tests {

class Fixed64TrigTest : public ::testing::Test {
 protected:
    // Use appropriate precision constant
    using Fixed = math::fp::Fixed64<32>;  // Trigonometric functions need 32-bit precision

#if FIXED64_MATH_USE_FAST_TRIG == 1
    const double epsilon = 1.5e-5;
#else
    const double epsilon = 2e-9;
#endif

    static constexpr double kEpsilonAtan2 = 1e-6;
    static constexpr double kEpsilonASinCos = 1e-5;

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

    // Helper to determine if an angle is a special case
    bool isSpecialAngle(double angle) {
        // Normalize angle to [0, 2π)
        double normalized = std::fmod(angle, 2 * M_PI);
        if (normalized < 0)
            normalized += 2 * M_PI;

        // Check if angle is near multiples of π/2
        for (int i = 0; i <= 4; i++) {
            if (std::abs(normalized - i * M_PI / 2) < 1e-6) {
                return true;
            }
        }
        return false;
    }
};

// Basic trigonometric function tests (sine, cosine, tangent)
TEST_F(Fixed64TrigTest, BasicTrigonometricFunctions) {
    for (const auto& angle : testAngles) {
        double dblAngle = static_cast<double>(angle);

        // Skip special angles
        if (isSpecialAngle(dblAngle)) {
            continue;
        }

        // Get sin and cos values
        double expectedSin = std::sin(dblAngle);
        double expectedCos = std::cos(dblAngle);
        double actualSin = static_cast<double>(Fixed64Math::Sin(angle));
        double actualCos = static_cast<double>(Fixed64Math::Cos(angle));

        // Test basic sin/cos accuracy using absolute error
        EXPECT_NEAR(actualSin, expectedSin, epsilon)
            << "Sin failed at angle " << dblAngle << " radians";
        EXPECT_NEAR(actualCos, expectedCos, epsilon)
            << "Cos failed at angle " << dblAngle << " radians";

        // Skip near-zero cos values to avoid division problems
        if (std::abs(actualCos) > 0.01) {
            // Test tan directly
            double expectedTan = std::tan(dblAngle);
            double actualTan = static_cast<double>(Fixed64Math::Tan(angle));
            EXPECT_NEAR(actualTan, expectedTan, epsilon)
                << "Tan failed at angle " << dblAngle << " radians";

            // Verify tan == sin/cos relationship
            double sinOverCos = actualSin / actualCos;
            EXPECT_NEAR(actualTan, sinOverCos, epsilon)
                << "Tan != Sin/Cos at angle " << dblAngle << " radians";
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
        EXPECT_NEAR(actualSin, std::sin(dblAngle), epsilon)
            << "Small angle approximation for sin failed at " << degrees << " degrees";

        // For very small angles, cos(x) ≈ 1
        double actualCos = static_cast<double>(Fixed64Math::Cos(angle));
        EXPECT_NEAR(actualCos, std::cos(dblAngle), epsilon)
            << "Small angle approximation for cos failed at " << degrees << " degrees";

        // For very small angles, sin(x) ≈ tan(x) ≈ x
        if (degrees < 0.5) {  // Only test for very small angles
            EXPECT_NEAR(actualSin, dblAngle, epsilon)
                << "Sin(x) ≈ x approximation failed at " << degrees << " degrees";

            double actualTan = static_cast<double>(Fixed64Math::Tan(angle));
            EXPECT_NEAR(actualTan, dblAngle, epsilon * 2)
                << "Tan(x) ≈ x approximation failed at " << degrees << " degrees";
        }
    }
}

// Inverse trigonometric function tests (arcsin, arccos, arctan)
TEST_F(Fixed64TrigTest, InverseTrigonometricFunctions) {
    // Test values for inverse trig functions
    std::vector<double> testValues = {
        -0.99, -0.9, -0.75, -0.5, -0.25, -0.1, 0.0, 0.1, 0.25, 0.5, 0.75, 0.9, 0.99};

    for (double value : testValues) {
        Fixed x(value);
        double dblX = static_cast<double>(x);

        // Arcsin test
        double expectedAsin = std::asin(dblX);
        double actualAsin = static_cast<double>(Fixed64Math::Asin(x));
        EXPECT_NEAR(actualAsin, expectedAsin, kEpsilonASinCos) << "Asin failed for value " << dblX;

        // Arccos test
        double expectedAcos = std::acos(dblX);
        double actualAcos = static_cast<double>(Fixed64Math::Acos(x));
        EXPECT_NEAR(actualAcos, expectedAcos, kEpsilonASinCos) << "Acos failed for value " << dblX;

        // Arctan test
        double expectedAtan = std::atan(dblX);
        double actualAtan = static_cast<double>(Fixed64Math::Atan(x));
        EXPECT_NEAR(actualAtan, expectedAtan, epsilon) << "Atan failed for value " << dblX;
    }
}

// Test for Atan2 function
TEST_F(Fixed64TrigTest, Atan2Function) {
    std::vector<std::pair<double, double>> testPoints = {{1.0, 1.0},
                                                         {-1.0, 1.0},
                                                         {-1.0, -1.0},
                                                         {1.0, -1.0},
                                                         {0.0, 1.0},
                                                         {1.0, 0.0},
                                                         {0.0, -1.0},
                                                         {-1.0, 0.0},
                                                         {3.0, 4.0},
                                                         {-3.0, 4.0},
                                                         {-3.0, -4.0},
                                                         {3.0, -4.0}};

    for (const auto& [y, x] : testPoints) {
        Fixed y_fixed(y);
        Fixed x_fixed(x);
        double dblY = static_cast<double>(y_fixed);
        double dblX = static_cast<double>(x_fixed);

        double expectedAtan2 = std::atan2(dblY, dblX);
        double actualAtan2 = static_cast<double>(Fixed64Math::Atan2(y_fixed, x_fixed));

        EXPECT_NEAR(actualAtan2, expectedAtan2, kEpsilonAtan2)
            << "Atan2 failed for point (" << dblY << ", " << dblX << ")";
    }
}

// Trigonometric identity tests
TEST_F(Fixed64TrigTest, TrigonometricIdentities) {
    // Test fundamental identities at various angles
    for (const auto& angle : testAngles) {
        double dblAngle = static_cast<double>(angle);

        Fixed sinVal = Fixed64Math::Sin(angle);
        Fixed cosVal = Fixed64Math::Cos(angle);

        // Pythagorean identity: sin²(θ) + cos²(θ) = 1
        Fixed identity1 = (sinVal * sinVal) + (cosVal * cosVal);
        EXPECT_NEAR(static_cast<double>(identity1), 1.0, epsilon)
            << "Pythagorean identity failed at angle " << static_cast<double>(angle);

        // Tangent identity: tan(θ) = sin(θ)/cos(θ)
        if (std::abs(static_cast<double>(cosVal)) > 0.01) {
            Fixed tanVal = Fixed64Math::Tan(angle);
            Fixed identity2 = sinVal / cosVal;
            EXPECT_NEAR(static_cast<double>(tanVal), static_cast<double>(identity2), epsilon)
                << "Tangent identity failed at angle " << static_cast<double>(angle);
        }

        // Double angle formula: cos(2θ) = cos²(θ) - sin²(θ)
        Fixed cos2x = Fixed64Math::Cos(angle * Fixed(2.0));
        Fixed identity3 = (cosVal * cosVal) - (sinVal * sinVal);

        EXPECT_NEAR(static_cast<double>(cos2x), static_cast<double>(identity3), epsilon)
            << "Double angle cosine identity failed at " << static_cast<double>(angle);
    }
}

// Edge case tests
TEST_F(Fixed64TrigTest, EdgeCases) {
    // Near π/2 tangent should be large but may be limited by implementation
    Fixed nearPiOver2 = Fixed::HalfPi() - Fixed::Epsilon() * Fixed(100);
    EXPECT_GT(static_cast<double>(Fixed64Math::Tan(nearPiOver2)), 10.0)
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

        // For small differences, use absolute error
        EXPECT_NEAR(sin1, sin2, epsilon) << "Sin lookup table should be continuous at " << base;

        double cos1 = static_cast<double>(Fixed64Math::Cos(angle1));
        double cos2 = static_cast<double>(Fixed64Math::Cos(angle2));
        EXPECT_NEAR(cos1, cos2, epsilon) << "Cos lookup table should be continuous at " << base;
    }

    // Test function periodicity
    for (int i = 0; i < 10; i++) {
        Fixed angle(i * 0.1);
        Fixed angle_plus_2pi = angle + Fixed::TwoPi();

        // Check sin(x) = sin(x + 2π)
        double sin1 = static_cast<double>(Fixed64Math::Sin(angle));
        double sin2 = static_cast<double>(Fixed64Math::Sin(angle_plus_2pi));

        // Use absolute error for periodic tests
        EXPECT_NEAR(sin1, sin2, epsilon)
            << "Sin should be periodic at " << static_cast<double>(angle);

        // Check cos(x) = cos(x + 2π)
        double cos1 = static_cast<double>(Fixed64Math::Cos(angle));
        double cos2 = static_cast<double>(Fixed64Math::Cos(angle_plus_2pi));
        EXPECT_NEAR(cos1, cos2, epsilon)
            << "Cos should be periodic at " << static_cast<double>(angle);

        // Check tan(x) = tan(x + π)
        if (std::abs(std::cos(static_cast<double>(angle))) > 0.1) {
            double tan1 = static_cast<double>(Fixed64Math::Tan(angle));
            double tan2 = static_cast<double>(Fixed64Math::Tan(angle_plus_2pi));
            EXPECT_NEAR(tan1, tan2, epsilon)
                << "Tan should be periodic at " << static_cast<double>(angle);
        }
    }

    // Test sign-change properties
    for (double angle = 0.1; angle < Fixed::Pi(); angle += 0.2) {
        Fixed angleFP(angle);
        Fixed neg_angleFP = -angleFP;

        // Check sin(-x) = -sin(x)
        double sin_pos = static_cast<double>(Fixed64Math::Sin(angleFP));
        double sin_neg = static_cast<double>(Fixed64Math::Sin(neg_angleFP));
        EXPECT_NEAR(sin_neg, -sin_pos, epsilon) << "Sin(-x) should equal -Sin(x) at " << angle;

        // Check cos(-x) = cos(x)
        double cos_pos = static_cast<double>(Fixed64Math::Cos(angleFP));
        double cos_neg = static_cast<double>(Fixed64Math::Cos(neg_angleFP));
        EXPECT_NEAR(cos_neg, cos_pos, epsilon) << "Cos(-x) should equal Cos(x) at " << angle;

        // Check tan(-x) = -tan(x)
        if (std::abs(std::cos(angle)) > 0.1) {
            double tan_pos = static_cast<double>(Fixed64Math::Tan(angleFP));
            double tan_neg = static_cast<double>(Fixed64Math::Tan(neg_angleFP));
            EXPECT_NEAR(tan_neg, -tan_pos, epsilon) << "Tan(-x) should equal -Tan(x) at " << angle;
        }
    }
}

}  // namespace math::fp::tests