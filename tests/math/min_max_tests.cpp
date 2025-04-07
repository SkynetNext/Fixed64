#include <cmath>
#include <limits>

#include "fixed64.h"
#include "fixed64_math.h"
#include "gtest/gtest.h"

namespace math::fp::tests {

// Tests for Min/Max utility functions
TEST(Fixed64MinMaxTest, BasicMinMax) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with positive values
    Fixed a(3.5);
    Fixed b(2.25);
    EXPECT_EQ(Fixed64Math::Min(a, b), b);
    EXPECT_EQ(Fixed64Math::Max(a, b), a);

    // Test with negative values
    Fixed c(-1.5);
    Fixed d(-3.75);
    EXPECT_EQ(Fixed64Math::Min(c, d), d);
    EXPECT_EQ(Fixed64Math::Max(c, d), c);

    // Test with mixed signs
    EXPECT_EQ(Fixed64Math::Min(a, c), c);
    EXPECT_EQ(Fixed64Math::Max(a, c), a);

    // Test with equal values
    Fixed e(5.0);
    EXPECT_EQ(Fixed64Math::Min(e, e), e);
    EXPECT_EQ(Fixed64Math::Max(e, e), e);
}

// Tests for Min/Max with special values
TEST(Fixed64MinMaxTest, SpecialValues) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with zero
    Fixed zero = Fixed::Zero();
    Fixed positive(1.5);
    Fixed negative(-1.5);

    EXPECT_EQ(Fixed64Math::Min(zero, positive), zero);
    EXPECT_EQ(Fixed64Math::Min(zero, negative), negative);
    EXPECT_EQ(Fixed64Math::Max(zero, positive), positive);
    EXPECT_EQ(Fixed64Math::Max(zero, negative), zero);

    // Test with very small values
    Fixed tiny(0.0000001);
    EXPECT_EQ(Fixed64Math::Min(zero, tiny), zero);
    EXPECT_EQ(Fixed64Math::Max(zero, tiny), tiny);

    // Test with extreme values
    Fixed max = Fixed::Max();
    Fixed min = Fixed::Min();

    EXPECT_EQ(Fixed64Math::Min(max, positive), positive);
    EXPECT_EQ(Fixed64Math::Max(min, negative), negative);
    EXPECT_EQ(Fixed64Math::Min(max, min), min);
    EXPECT_EQ(Fixed64Math::Max(max, min), max);
}

// Tests for Min/Max with different precisions
TEST(Fixed64MinMaxTest, DifferentPrecisions) {
    using Fixed8 = math::fp::Fixed64<8>;
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    // Create values with different precisions
    Fixed8 value8(3.5);
    Fixed16 value16(2.25);
    Fixed32 value32(1.125);

    // Test Min/Max with same precision
    EXPECT_EQ(Fixed64Math::Min(value8, Fixed8(4.0)), value8);
    EXPECT_EQ(Fixed64Math::Min(value16, Fixed16(1.0)), Fixed16(1.0));
    EXPECT_EQ(Fixed64Math::Min(value32, Fixed32(2.0)), value32);

    EXPECT_EQ(Fixed64Math::Max(value8, Fixed8(4.0)), Fixed8(4.0));
    EXPECT_EQ(Fixed64Math::Max(value16, Fixed16(1.0)), value16);
    EXPECT_EQ(Fixed64Math::Max(value32, Fixed32(2.0)), Fixed32(2.0));

    // Test with values that would be equal at lower precision but different at higher precision
    Fixed32 precise(1.0 + std::pow(2.0, -20));
    Fixed16 lessPrecise(1.0);

    EXPECT_EQ(Fixed64Math::Min(precise, Fixed32(lessPrecise)), Fixed32(lessPrecise));
    EXPECT_EQ(Fixed64Math::Max(precise, Fixed32(lessPrecise)), precise);
}

// Tests for Abs function
TEST(Fixed64MinMaxTest, AbsFunction) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with positive values
    Fixed a(3.5);
    EXPECT_EQ(Fixed64Math::Abs(a), a);

    // Test with negative values
    Fixed b(-2.25);
    EXPECT_EQ(Fixed64Math::Abs(b), Fixed(2.25));

    // Test with zero
    Fixed zero = Fixed::Zero();
    EXPECT_EQ(Fixed64Math::Abs(zero), zero);

    // Test with very small values
    Fixed tiny(0.0000001);
    EXPECT_EQ(Fixed64Math::Abs(tiny), tiny);
    EXPECT_EQ(Fixed64Math::Abs(-tiny), tiny);

    // Test with extreme values
    Fixed max = Fixed::Max();
    Fixed min = Fixed::Min();

    EXPECT_EQ(Fixed64Math::Abs(max), max);
    // Note: Abs(Min) might overflow since |Min| > Max in two's complement
    // This test depends on implementation details
    EXPECT_EQ(Fixed64Math::Abs(min).value(), min.value())
        << "Abs(min) = " << Fixed64Math::Abs(min) << ", min = " << min
        << ", ~min.value() = " << ~min.value() << ", ~min.value() + 1 = " << (~min.value() + 1);
}

// Tests for Sign function
TEST(Fixed64MinMaxTest, SignFunction) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with positive values
    Fixed a(3.5);
    EXPECT_EQ(Fixed64Math::Sign(a), Fixed::One());

    // Test with negative values
    Fixed b(-2.25);
    EXPECT_EQ(Fixed64Math::Sign(b), -Fixed::One());

    // Test with zero
    Fixed zero = Fixed::Zero();
    EXPECT_EQ(Fixed64Math::Sign(zero), 0);

    // Test with very small values
    Fixed tiny = Fixed::Epsilon();
    EXPECT_EQ(Fixed64Math::Sign(tiny), Fixed::One());
    EXPECT_EQ(Fixed64Math::Sign(-tiny), -Fixed::One());

    // Test with extreme values
    Fixed max = Fixed::Max();
    Fixed min = Fixed::Min();

    EXPECT_EQ(Fixed64Math::Sign(max), Fixed::One());
    EXPECT_EQ(Fixed64Math::Sign(min), -Fixed::One());
}

}  // namespace math::fp::tests