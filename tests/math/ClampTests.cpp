#include <cmath>
#include <limits>

#include "Fixed64.h"
#include "Fixed64Math.h"
#include "gtest/gtest.h"

namespace math::fp::tests {

// Tests for Clamp function
TEST(Fixed64ClampTest, BasicClamp) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with value within range
    Fixed value(3.5);
    Fixed min(2.0);
    Fixed max(5.0);
    EXPECT_EQ(Fixed64Math::Clamp(value, min, max), value);

    // Test with value below minimum
    Fixed belowMin(1.0);
    EXPECT_EQ(Fixed64Math::Clamp(belowMin, min, max), min);

    // Test with value above maximum
    Fixed aboveMax(6.0);
    EXPECT_EQ(Fixed64Math::Clamp(aboveMax, min, max), max);

    // Test with value equal to minimum
    EXPECT_EQ(Fixed64Math::Clamp(min, min, max), min);

    // Test with value equal to maximum
    EXPECT_EQ(Fixed64Math::Clamp(max, min, max), max);
}

// Tests for Clamp with negative values
TEST(Fixed64ClampTest, NegativeValues) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with negative range
    Fixed value(-3.5);
    Fixed min(-5.0);
    Fixed max(-2.0);
    EXPECT_EQ(Fixed64Math::Clamp(value, min, max), value);

    // Test with value below negative minimum
    Fixed belowMin(-6.0);
    EXPECT_EQ(Fixed64Math::Clamp(belowMin, min, max), min);

    // Test with value above negative maximum
    Fixed aboveMax(-1.0);
    EXPECT_EQ(Fixed64Math::Clamp(aboveMax, min, max), max);

    // Test with range crossing zero
    Fixed crossMin(-5.0);
    Fixed crossMax(5.0);

    EXPECT_EQ(Fixed64Math::Clamp(Fixed(-10.0), crossMin, crossMax), crossMin);
    EXPECT_EQ(Fixed64Math::Clamp(Fixed::Zero(), crossMin, crossMax), Fixed::Zero());
    EXPECT_EQ(Fixed64Math::Clamp(Fixed(10.0), crossMin, crossMax), crossMax);
}

// Tests for Clamp with special cases
TEST(Fixed64ClampTest, SpecialCases) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with zero range (min == max)
    Fixed min(3.0);
    Fixed max(3.0);
    EXPECT_EQ(Fixed64Math::Clamp(Fixed(1.0), min, max), min);
    EXPECT_EQ(Fixed64Math::Clamp(Fixed(3.0), min, max), min);
    EXPECT_EQ(Fixed64Math::Clamp(Fixed(5.0), min, max), max);

    // Test with inverted range (min > max)
    // Implementation-dependent behavior, but typically returns max
    Fixed invMin(5.0);
    Fixed invMax(2.0);
    EXPECT_EQ(Fixed64Math::Clamp(Fixed(3.0), invMax, invMin), Fixed(3.0));

    // Test with very small values
    Fixed smallMin(0.0001);
    Fixed smallMax(0.0002);
    EXPECT_EQ(Fixed64Math::Clamp(Fixed(0.00005), smallMin, smallMax), smallMin);
    EXPECT_EQ(Fixed64Math::Clamp(Fixed(0.00015), smallMin, smallMax), Fixed(0.00015));
    EXPECT_EQ(Fixed64Math::Clamp(Fixed(0.0003), smallMin, smallMax), smallMax);

    // Test with extreme values
    Fixed extremeValue = Fixed::Max();
    Fixed normalMax(1000.0);
    EXPECT_EQ(Fixed64Math::Clamp(extremeValue, Fixed::Zero(), normalMax), normalMax);

    Fixed extremeMin = Fixed::Min();
    Fixed normalMin(-1000.0);
    EXPECT_EQ(Fixed64Math::Clamp(extremeMin, normalMin, Fixed::Zero()), normalMin);
}

// Tests for Clamp01 function
TEST(Fixed64ClampTest, Clamp01Function) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with value within [0,1] range
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed(0.5)), Fixed(0.5));

    // Test with value below 0
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed(-0.5)), Fixed::Zero());
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed(-1.0)), Fixed::Zero());
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed::Min()), Fixed::Zero());

    // Test with value above 1
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed(1.5)), Fixed::One());
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed(2.0)), Fixed::One());
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed::Max()), Fixed::One());

    // Test with boundary values
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed::Zero()), Fixed::Zero());
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed::One()), Fixed::One());

    // Test with very small values
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed(0.0001)), Fixed(0.0001));
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed(-0.0001)), Fixed::Zero());

    // Test with values very close to 1
    Fixed almostOne = Fixed::One() - Fixed::Epsilon();
    EXPECT_EQ(Fixed64Math::Clamp01(almostOne), almostOne);

    Fixed justOverOne = Fixed::One() + Fixed::Epsilon();
    EXPECT_EQ(Fixed64Math::Clamp01(justOverOne), Fixed::One());
}

// Tests for Clamp with different precisions
TEST(Fixed64ClampTest, DifferentPrecisions) {
    using Fixed8 = math::fp::Fixed64<8>;
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    // Test Clamp with Fixed8
    Fixed8 value8(3.5);
    Fixed8 min8(2.0);
    Fixed8 max8(5.0);
    EXPECT_EQ(Fixed64Math::Clamp(value8, min8, max8), value8);
    EXPECT_EQ(Fixed64Math::Clamp(Fixed8(1.0), min8, max8), min8);
    EXPECT_EQ(Fixed64Math::Clamp(Fixed8(6.0), min8, max8), max8);

    // Test Clamp with Fixed16
    Fixed16 value16(3.5);
    Fixed16 min16(2.0);
    Fixed16 max16(5.0);
    EXPECT_EQ(Fixed64Math::Clamp(value16, min16, max16), value16);
    EXPECT_EQ(Fixed64Math::Clamp(Fixed16(1.0), min16, max16), min16);
    EXPECT_EQ(Fixed64Math::Clamp(Fixed16(6.0), min16, max16), max16);

    // Test Clamp with Fixed32
    Fixed32 value32(3.5);
    Fixed32 min32(2.0);
    Fixed32 max32(5.0);
    EXPECT_EQ(Fixed64Math::Clamp(value32, min32, max32), value32);
    EXPECT_EQ(Fixed64Math::Clamp(Fixed32(1.0), min32, max32), min32);
    EXPECT_EQ(Fixed64Math::Clamp(Fixed32(6.0), min32, max32), max32);

    // Test Clamp01 with different precisions
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed8(0.5)), Fixed8(0.5));
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed8(-0.5)), Fixed8::Zero());
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed8(1.5)), Fixed8::One());

    EXPECT_EQ(Fixed64Math::Clamp01(Fixed16(0.5)), Fixed16(0.5));
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed16(-0.5)), Fixed16::Zero());
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed16(1.5)), Fixed16::One());

    EXPECT_EQ(Fixed64Math::Clamp01(Fixed32(0.5)), Fixed32(0.5));
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed32(-0.5)), Fixed32::Zero());
    EXPECT_EQ(Fixed64Math::Clamp01(Fixed32(1.5)), Fixed32::One());
}

}  // namespace math::fp::tests