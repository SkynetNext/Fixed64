#include <cmath>
#include <limits>

#include "fixed64.h"
#include "fixed64_math.h"
#include "gtest/gtest.h"

namespace math::fp::tests {

// Tests for Floor function
TEST(Fixed64RoundingTest, FloorFunction) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with positive values
    EXPECT_EQ(Fixed64Math::Floor(Fixed(3.75)), Fixed(3.0));
    EXPECT_EQ(Fixed64Math::Floor(Fixed(3.0)), Fixed(3.0));
    EXPECT_EQ(Fixed64Math::Floor(Fixed(3.1)), Fixed(3.0));
    EXPECT_EQ(Fixed64Math::Floor(Fixed(3.5)), Fixed(3.0));
    EXPECT_EQ(Fixed64Math::Floor(Fixed(3.99)), Fixed(3.0));

    // Test with negative values
    EXPECT_EQ(Fixed64Math::Floor(Fixed(-3.25)), Fixed(-4.0));
    EXPECT_EQ(Fixed64Math::Floor(Fixed(-3.0)), Fixed(-3.0));
    EXPECT_EQ(Fixed64Math::Floor(Fixed(-3.1)), Fixed(-4.0));
    EXPECT_EQ(Fixed64Math::Floor(Fixed(-3.5)), Fixed(-4.0));
    EXPECT_EQ(Fixed64Math::Floor(Fixed(-3.99)), Fixed(-4.0));

    // Test with zero
    EXPECT_EQ(Fixed64Math::Floor(Fixed::Zero()), Fixed::Zero());

    // Test with very small values
    EXPECT_EQ(Fixed64Math::Floor(Fixed(0.1)), Fixed::Zero());
    EXPECT_EQ(Fixed64Math::Floor(Fixed(-0.1)), Fixed(-1.0));

    // Test with large values
    EXPECT_EQ(Fixed64Math::Floor(Fixed(1000.75)), Fixed(1000.0));
    EXPECT_EQ(Fixed64Math::Floor(Fixed(-1000.75)), Fixed(-1001.0));
}

// Tests for Ceil function
TEST(Fixed64RoundingTest, CeilFunction) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with positive values
    EXPECT_EQ(Fixed64Math::Ceil(Fixed(3.25)), Fixed(4.0));
    EXPECT_EQ(Fixed64Math::Ceil(Fixed(3.0)), Fixed(3.0));
    EXPECT_EQ(Fixed64Math::Ceil(Fixed(3.1)), Fixed(4.0));
    EXPECT_EQ(Fixed64Math::Ceil(Fixed(3.5)), Fixed(4.0));
    EXPECT_EQ(Fixed64Math::Ceil(Fixed(3.99)), Fixed(4.0));

    // Test with negative values
    EXPECT_EQ(Fixed64Math::Ceil(Fixed(-3.75)), Fixed(-3.0));
    EXPECT_EQ(Fixed64Math::Ceil(Fixed(-3.0)), Fixed(-3.0));
    EXPECT_EQ(Fixed64Math::Ceil(Fixed(-3.1)), Fixed(-3.0));
    EXPECT_EQ(Fixed64Math::Ceil(Fixed(-3.5)), Fixed(-3.0));
    EXPECT_EQ(Fixed64Math::Ceil(Fixed(-3.99)), Fixed(-3.0));

    // Test with zero
    EXPECT_EQ(Fixed64Math::Ceil(Fixed::Zero()), Fixed::Zero());

    // Test with very small values
    EXPECT_EQ(Fixed64Math::Ceil(Fixed(0.1)), Fixed(1.0));
    EXPECT_EQ(Fixed64Math::Ceil(Fixed(-0.1)), Fixed::Zero());

    // Test with large values
    EXPECT_EQ(Fixed64Math::Ceil(Fixed(1000.25)), Fixed(1001.0));
    EXPECT_EQ(Fixed64Math::Ceil(Fixed(-1000.25)), Fixed(-1000.0));
}

// Tests for Round function
TEST(Fixed64RoundingTest, RoundFunction) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with positive values
    EXPECT_EQ(Fixed64Math::Round(Fixed(3.25)), Fixed(3.0));
    EXPECT_EQ(Fixed64Math::Round(Fixed(3.0)), Fixed(3.0));
    EXPECT_EQ(Fixed64Math::Round(Fixed(3.1)), Fixed(3.0));
    EXPECT_EQ(Fixed64Math::Round(Fixed(3.5)), Fixed(4.0));
    EXPECT_EQ(Fixed64Math::Round(Fixed(3.75)), Fixed(4.0));
    EXPECT_EQ(Fixed64Math::Round(Fixed(3.99)), Fixed(4.0));

    // Test with negative values
    EXPECT_EQ(Fixed64Math::Round(Fixed(-3.25)), Fixed(-3.0));
    EXPECT_EQ(Fixed64Math::Round(Fixed(-3.0)), Fixed(-3.0));
    EXPECT_EQ(Fixed64Math::Round(Fixed(-3.1)), Fixed(-3.0));
    EXPECT_EQ(Fixed64Math::Round(Fixed(-3.5)), Fixed(-4.0));
    EXPECT_EQ(Fixed64Math::Round(Fixed(-3.75)), Fixed(-4.0));
    EXPECT_EQ(Fixed64Math::Round(Fixed(-3.99)), Fixed(-4.0));

    // Test with zero
    EXPECT_EQ(Fixed64Math::Round(Fixed::Zero()), Fixed::Zero());

    // Test with very small values
    EXPECT_EQ(Fixed64Math::Round(Fixed(0.1)), Fixed::Zero());
    EXPECT_EQ(Fixed64Math::Round(Fixed(-0.1)), Fixed::Zero());
    EXPECT_EQ(Fixed64Math::Round(Fixed(0.5)), Fixed(1.0));
    EXPECT_EQ(Fixed64Math::Round(Fixed(-0.5)), Fixed(-1.0));

    // Test with large values
    EXPECT_EQ(Fixed64Math::Round(Fixed(1000.25)), Fixed(1000.0));
    EXPECT_EQ(Fixed64Math::Round(Fixed(1000.5)), Fixed(1001.0));
    EXPECT_EQ(Fixed64Math::Round(Fixed(-1000.25)), Fixed(-1000.0));
    EXPECT_EQ(Fixed64Math::Round(Fixed(-1000.5)), Fixed(-1001.0));
}

// Tests for Trunc function
TEST(Fixed64RoundingTest, TruncFunction) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with positive values
    EXPECT_EQ(Fixed64Math::Trunc(Fixed(3.75)), Fixed(3.0));
    EXPECT_EQ(Fixed64Math::Trunc(Fixed(3.0)), Fixed(3.0));
    EXPECT_EQ(Fixed64Math::Trunc(Fixed(3.1)), Fixed(3.0));
    EXPECT_EQ(Fixed64Math::Trunc(Fixed(3.5)), Fixed(3.0));
    EXPECT_EQ(Fixed64Math::Trunc(Fixed(3.99)), Fixed(3.0));

    // Test with negative values
    EXPECT_EQ(Fixed64Math::Trunc(Fixed(-3.75)), Fixed(-3.0));
    EXPECT_EQ(Fixed64Math::Trunc(Fixed(-3.0)), Fixed(-3.0));
    EXPECT_EQ(Fixed64Math::Trunc(Fixed(-3.1)), Fixed(-3.0));
    EXPECT_EQ(Fixed64Math::Trunc(Fixed(-3.5)), Fixed(-3.0));
    EXPECT_EQ(Fixed64Math::Trunc(Fixed(-3.99)), Fixed(-3.0));

    // Test with zero
    EXPECT_EQ(Fixed64Math::Trunc(Fixed::Zero()), Fixed::Zero());

    // Test with very small values
    EXPECT_EQ(Fixed64Math::Trunc(Fixed(0.1)), Fixed::Zero());
    EXPECT_EQ(Fixed64Math::Trunc(Fixed(-0.1)), Fixed::Zero());

    // Test with large values
    EXPECT_EQ(Fixed64Math::Trunc(Fixed(1000.75)), Fixed(1000.0));
    EXPECT_EQ(Fixed64Math::Trunc(Fixed(-1000.75)), Fixed(-1000.0));
}

// Tests for Fractions function
TEST(Fixed64RoundingTest, FractionsFunction) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Calculate epsilon for precision comparisons
    double epsilon = static_cast<double>(Fixed::Epsilon());

    // Test with positive values
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Fractions(Fixed(3.75))), 0.75, epsilon);
    EXPECT_EQ(Fixed64Math::Fractions(Fixed(3.0)), Fixed::Zero());
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Fractions(Fixed(3.25))), 0.25, epsilon);

    // Test with negative values
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Fractions(Fixed(-3.75))), 0.25, epsilon);
    EXPECT_EQ(Fixed64Math::Fractions(Fixed(-3.0)), Fixed::Zero());
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Fractions(Fixed(-3.25))), 0.75, epsilon);

    // Test with zero
    EXPECT_EQ(Fixed64Math::Fractions(Fixed::Zero()), Fixed::Zero());

    // Test with very small values
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Fractions(Fixed(0.1))), 0.1, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Fractions(Fixed(-0.1))), 0.9, epsilon);

    // Test with large values
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Fractions(Fixed(1000.75))), 0.75, epsilon);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Fractions(Fixed(-1000.75))), 0.25, epsilon);

    // Test with different precisions
    using Fixed32 = math::fp::Fixed64<32>;
    double epsilon32 = static_cast<double>(Fixed32::Epsilon());

    EXPECT_NEAR(static_cast<double>(Fixed64Math::Fractions(Fixed32(3.125))), 0.125, epsilon32);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Fractions(Fixed32(-3.125))), 0.875, epsilon32);
}

}  // namespace math::fp::tests