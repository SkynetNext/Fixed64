#include <cmath>
#include <limits>

#include "Fixed64.h"
#include "Fixed64Math.h"
#include "gtest/gtest.h"

namespace math::fp::tests {

// Tests for Lerp function
TEST(Fixed64InterpolationTest, LerpFunction) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Calculate epsilon for precision comparisons
    double epsilon = static_cast<double>(Fixed::Epsilon());

    // Test basic linear interpolation
    Fixed a(2.0);
    Fixed b(10.0);

    // Test at different t values
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed::Zero())), 2.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed(0.25))), 4.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed(0.5))), 6.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed(0.75))), 8.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed::One())), 10.0, epsilon);

    // Test with negative values
    Fixed c(-5.0);
    Fixed d(5.0);

    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(c, d, Fixed::Zero())), -5.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(c, d, Fixed(0.5))), 0.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(c, d, Fixed::One())), 5.0, epsilon);

    // Test with both negative values
    Fixed e(-10.0);
    Fixed f(-2.0);

    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(e, f, Fixed::Zero())), -10.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(e, f, Fixed(0.5))), -6.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(e, f, Fixed::One())), -2.0, epsilon);
}

// Tests for Lerp extrapolation
TEST(Fixed64InterpolationTest, LerpExtrapolation) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Calculate epsilon for precision comparisons
    double epsilon = static_cast<double>(Fixed::Epsilon());

    // Test extrapolation (t outside [0,1])
    Fixed a(2.0);
    Fixed b(10.0);

    // t < 0 (extrapolate below a)
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed(-0.5))), -2.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed(-1.0))), -6.0, epsilon);

    // t > 1 (extrapolate beyond b)
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed(1.5))), 14.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed(2.0))), 18.0, epsilon);

    // Test with negative values
    Fixed c(-5.0);
    Fixed d(5.0);

    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(c, d, Fixed(-0.5))), -10.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(c, d, Fixed(1.5))), 10.0, epsilon);
}

// Tests for Lerp function with clamping behavior
TEST(Fixed64InterpolationTest, LerpClampingBehavior) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Calculate epsilon for precision comparisons
    double epsilon = static_cast<double>(Fixed::Epsilon());

    // Test basic linear interpolation (within [0,1])
    Fixed a(2.0);
    Fixed b(10.0);

    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed::Zero())), 2.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed(0.25))), 4.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed(0.5))), 6.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed(0.75))), 8.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed::One())), 10.0, epsilon);

    // Test clamping behavior (t outside [0,1])
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed(-0.5))), 2.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a, b, Fixed(1.5))), 10.0, epsilon);

    // Test with negative values
    Fixed c(-5.0);
    Fixed d(5.0);

    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(c, d, Fixed::Zero())), -5.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(c, d, Fixed(0.5))), 0.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(c, d, Fixed::One())), 5.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(c, d, Fixed(-0.5))), -5.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(c, d, Fixed(1.5))), 5.0, epsilon);
}

// Tests for InverseLerp function
TEST(Fixed64InterpolationTest, InverseLerpFunction) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Calculate epsilon for precision comparisons
    double epsilon = static_cast<double>(Fixed::Epsilon());

    // Test basic inverse linear interpolation
    Fixed a(2.0);
    Fixed b(10.0);

    // Test at different values
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(a, b, Fixed(2.0))), 0.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(a, b, Fixed(4.0))), 0.25, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(a, b, Fixed(6.0))), 0.5, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(a, b, Fixed(8.0))), 0.75, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(a, b, Fixed(10.0))), 1.0, epsilon);

    // Test with values outside the range
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(a, b, Fixed(0.0))), -0.25, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(a, b, Fixed(12.0))), 1.25, epsilon);

    // Test with negative values
    Fixed c(-5.0);
    Fixed d(5.0);

    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(c, d, Fixed(-5.0))), 0.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(c, d, Fixed(0.0))), 0.5, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(c, d, Fixed(5.0))), 1.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(c, d, Fixed(-10.0))), -0.5, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(c, d, Fixed(10.0))), 1.5, epsilon);
}

// Tests for InverseLerp with special cases
TEST(Fixed64InterpolationTest, InverseLerpSpecialCases) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Calculate epsilon for precision comparisons
    double epsilon = static_cast<double>(Fixed::Epsilon());

    // Test with a == b (division by zero case)
    Fixed a(5.0);
    Fixed b(5.0);

    // Implementation-dependent behavior, but typically returns 0 or 0.5
    // Just verify it doesn't crash and returns a valid Fixed value
    Fixed result = Fixed64Math::InverseLerp(a, b, Fixed(5.0));
    EXPECT_TRUE(result == Fixed::Zero() || result == Fixed(0.5) || result == Fixed::One());

    // Test with very close values
    Fixed c(5.0);
    Fixed d(5.0 + std::pow(2.0, -10));

    // Should be close to 0 or 1 depending on the value
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(c, d, c)), 0.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(c, d, d)), 1.0, epsilon);

    // Test with swapped min/max
    Fixed e(10.0);
    Fixed f(2.0);

    // Should invert the result compared to normal order
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(e, f, Fixed(10.0))), 0.0, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(e, f, Fixed(6.0))), 0.5, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(e, f, Fixed(2.0))), 1.0, epsilon);
}

// Tests for interpolation with different precisions
TEST(Fixed64InterpolationTest, DifferentPrecisions) {
    using Fixed8 = math::fp::Fixed64<8>;
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    // Calculate epsilon for each precision
    double epsilon8 = static_cast<double>(Fixed8::Epsilon());
    double epsilon16 = static_cast<double>(Fixed16::Epsilon());
    double epsilon32 = static_cast<double>(Fixed32::Epsilon());

    // Test Lerp with different precisions
    Fixed8 a8(2.0);
    Fixed8 b8(10.0);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a8, b8, Fixed8(0.5))), 6.0, epsilon8);

    Fixed16 a16(2.0);
    Fixed16 b16(10.0);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a16, b16, Fixed16(0.5))), 6.0, epsilon16);

    Fixed32 a32(2.0);
    Fixed32 b32(10.0);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a32, b32, Fixed32(0.5))), 6.0, epsilon32);

    // Test Lerp clamping behavior with different precisions
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a8, b8, Fixed8(1.5))), 10.0, epsilon8);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a16, b16, Fixed16(1.5))), 10.0, epsilon16);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Lerp(a32, b32, Fixed32(1.5))), 10.0, epsilon32);

    // Test InverseLerp with different precisions
    EXPECT_NEAR(static_cast<double>(Fixed64Math::InverseLerp(a8, b8, Fixed8(6.0))), 0.5, epsilon8);
    EXPECT_NEAR(
        static_cast<double>(Fixed64Math::InverseLerp(a16, b16, Fixed16(6.0))), 0.5, epsilon16);
    EXPECT_NEAR(
        static_cast<double>(Fixed64Math::InverseLerp(a32, b32, Fixed32(6.0))), 0.5, epsilon32);
}

}  // namespace math::fp::tests