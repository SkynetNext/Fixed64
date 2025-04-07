#include <cmath>
#include <limits>

#include "fixed64.h"
#include "fixed64_math.h"
#include "gtest/gtest.h"

namespace math::fp::tests {

// Test angle conversion functions between degrees and radians
TEST(Fixed64AngleConversionTest, DegreeRadianConversion) {
    using Fixed = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    // Calculate epsilon for precision comparisons
    double epsilon = std::sqrt(static_cast<double>(Fixed::Epsilon()));

    // Test degree to radian conversion
    // Test common angle values
    EXPECT_NEAR(static_cast<double>(Fixed(0.0) * Fixed::Deg2Rad()), 0.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed(90.0) * Fixed::Deg2Rad()),
                static_cast<double>(Fixed::HalfPi()),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed(180.0) * Fixed::Deg2Rad()),
                static_cast<double>(Fixed::Pi()),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed(270.0) * Fixed::Deg2Rad()),
                static_cast<double>(Fixed::Pi() + Fixed::HalfPi()),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed(360.0) * Fixed::Deg2Rad()),
                static_cast<double>(Fixed::TwoPi()),
                epsilon);

    // Test negative angle values
    EXPECT_NEAR(static_cast<double>(Fixed(-90.0) * Fixed::Deg2Rad()),
                -static_cast<double>(Fixed::HalfPi()),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed(-180.0) * Fixed::Deg2Rad()),
                -static_cast<double>(Fixed::Pi()),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed(-360.0) * Fixed::Deg2Rad()),
                -static_cast<double>(Fixed::TwoPi()),
                epsilon);

    // Test non-standard angle values
    EXPECT_NEAR(static_cast<double>(Fixed(45.0) * Fixed::Deg2Rad()),
                static_cast<double>(Fixed::QuarterPi()),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed(30.0) * Fixed::Deg2Rad()),
                static_cast<double>(Fixed::Pi() / Fixed(6.0)),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed(60.0) * Fixed::Deg2Rad()),
                static_cast<double>(Fixed::Pi() / Fixed(3.0)),
                epsilon);

    // Test large angle values
    EXPECT_NEAR(static_cast<double>(Fixed(720.0) * Fixed::Deg2Rad()),
                static_cast<double>(Fixed::TwoPi() * Fixed(2.0)),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed(1080.0) * Fixed::Deg2Rad()),
                static_cast<double>(Fixed::TwoPi() * Fixed(3.0)),
                epsilon);

    // Test radian to degree conversion
    // Test common radian values
    EXPECT_NEAR(static_cast<double>(Fixed(0.0) * Fixed::Rad2Deg()), 0.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed::HalfPi() * Fixed::Rad2Deg()), 90.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed::Pi() * Fixed::Rad2Deg()), 180.0, epsilon);
    EXPECT_NEAR(
        static_cast<double>((Fixed::Pi() + Fixed::HalfPi()) * Fixed::Rad2Deg()), 270.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed::TwoPi() * Fixed::Rad2Deg()), 360.0, epsilon);

    // Test negative radian values
    EXPECT_NEAR(static_cast<double>(-Fixed::HalfPi() * Fixed::Rad2Deg()), -90.0, epsilon);
    EXPECT_NEAR(static_cast<double>(-Fixed::Pi() * Fixed::Rad2Deg()), -180.0, epsilon);
    EXPECT_NEAR(static_cast<double>(-Fixed::TwoPi() * Fixed::Rad2Deg()), -360.0, epsilon);

    // Test non-standard radian values
    EXPECT_NEAR(static_cast<double>(Fixed::QuarterPi() * Fixed::Rad2Deg()), 45.0, epsilon);
    EXPECT_NEAR(static_cast<double>((Fixed::Pi() / Fixed(6.0)) * Fixed::Rad2Deg()), 30.0, epsilon);
    EXPECT_NEAR(static_cast<double>((Fixed::Pi() / Fixed(3.0)) * Fixed::Rad2Deg()), 60.0, epsilon);

    // Test large radian values
    EXPECT_NEAR(
        static_cast<double>((Fixed::TwoPi() * Fixed(2.0)) * Fixed::Rad2Deg()), 720.0, epsilon);
    EXPECT_NEAR(
        static_cast<double>((Fixed::TwoPi() * Fixed(3.0)) * Fixed::Rad2Deg()), 1080.0, epsilon);

    // Test conversion consistency (degrees->radians->degrees)
    for (int angle = -720; angle <= 720; angle += 45) {
        Fixed degrees(angle);
        Fixed radians = degrees * Fixed::Deg2Rad();
        Fixed backToDegrees = radians * Fixed::Rad2Deg();
        EXPECT_NEAR(static_cast<double>(backToDegrees), static_cast<double>(degrees), epsilon);
    }

    // Test conversion consistency (radians->degrees->radians)
    for (double rad = -4.0 * static_cast<double>(Fixed::Pi());
         rad <= 4.0 * static_cast<double>(Fixed::Pi());
         rad += static_cast<double>(Fixed::Pi()) / 4.0) {
        Fixed radians(rad);
        Fixed degrees = radians * Fixed::Rad2Deg();
        Fixed backToRadians = degrees * Fixed::Deg2Rad();
        EXPECT_NEAR(static_cast<double>(backToRadians), static_cast<double>(radians), epsilon);
    }
}

// Test angle normalization functions
TEST(Fixed64AngleConversionTest, AngleNormalization) {
    using Fixed = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    // Calculate epsilon for precision comparisons
    double epsilon = static_cast<double>(Fixed::Epsilon()) * 10;

    // Test NormalizeAngle function (normalizes angle to [0, 2π) range)
    // Test angles already in range
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(Fixed(0.0))),
                static_cast<double>(Fixed(0.0)),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(Fixed::HalfPi())),
                static_cast<double>(Fixed::HalfPi()),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(Fixed::Pi())),
                static_cast<double>(Fixed::Pi()),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(-Fixed::HalfPi())),
                static_cast<double>(Fixed::TwoPi() - Fixed::HalfPi()),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(-Fixed::Pi())),
                static_cast<double>(Fixed::TwoPi() - Fixed::Pi()),
                epsilon);

    // Test angles that need normalization
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(Fixed::Pi() + Fixed::HalfPi())),
                static_cast<double>(Fixed::Pi() + Fixed::HalfPi()),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(Fixed::TwoPi())),
                static_cast<double>(Fixed(0.0)),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(Fixed::TwoPi() + Fixed::HalfPi())),
                static_cast<double>(Fixed::HalfPi()),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(Fixed::TwoPi() + Fixed::Pi())),
                static_cast<double>(Fixed::Pi()),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(Fixed::TwoPi() * Fixed(2.0))),
                static_cast<double>(Fixed(0.0)),
                epsilon);

    // Test normalization of negative angles
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(-Fixed::Pi() - Fixed::HalfPi())),
                static_cast<double>(Fixed::HalfPi()),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(-Fixed::TwoPi())),
                static_cast<double>(Fixed(0.0)),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(-Fixed::TwoPi() - Fixed::HalfPi())),
                static_cast<double>(Fixed::TwoPi() - Fixed::HalfPi()),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(-Fixed::TwoPi() - Fixed::Pi())),
                static_cast<double>(Fixed::TwoPi() - Fixed::Pi()),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(-Fixed::TwoPi() * Fixed(2.0))),
                static_cast<double>(Fixed(0.0)),
                epsilon);

    // Test edge cases
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(Fixed::Pi() + Fixed(0.1))),
                static_cast<double>(Fixed::Pi() + Fixed(0.1)),
                epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::NormalizeAngle(-Fixed::Pi() - Fixed(0.1))),
                static_cast<double>(Fixed::TwoPi() - Fixed::Pi() - Fixed(0.1)),
                epsilon);
}

}  // namespace math::fp::tests