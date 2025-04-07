#include <cmath>
#include <limits>

#include "fixed64.h"
#include "fixed64_math.h"
#include "gtest/gtest.h"

namespace math::fp::tests {

// Test suite for comparison functions
class Fixed64ComparisonTest : public ::testing::Test {
 protected:
    // Use different precision levels
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;

    // Use proper epsilon values as per best practices
    static constexpr double epsilon16 = static_cast<double>(Fixed16::Epsilon());
    static constexpr double epsilon32 = static_cast<double>(Fixed32::Epsilon());
};

// Tests for the IsNearlyEqual function
TEST_F(Fixed64ComparisonTest, IsNearlyEqualTests) {
    // Test exact equality
    EXPECT_TRUE(Fixed64Math::IsNearlyEqual(Fixed16(5.0), Fixed16(5.0)))
        << "Exactly equal values should be considered nearly equal";

    // Test values within default epsilon
    Fixed16 a(1.0);
    Fixed16 b = a + Fixed16::Epsilon() * Fixed16(0.5);
    EXPECT_TRUE(Fixed64Math::IsNearlyEqual(a, b)) << "Values within epsilon should be nearly equal";

    // Test values just outside default epsilon
    Fixed16 c = a + Fixed16::Epsilon() * 5;
    EXPECT_FALSE(Fixed64Math::IsNearlyEqual(a, c))
        << "Values beyond epsilon should not be nearly equal";

    // Test with custom tolerance
    Fixed16 tolerance(0.1);
    Fixed16 d(1.0);
    Fixed16 e(1.05);
    EXPECT_TRUE(Fixed64Math::IsNearlyEqual(d, e, tolerance))
        << "Values within custom tolerance should be nearly equal";

    Fixed16 f(1.15);
    EXPECT_FALSE(Fixed64Math::IsNearlyEqual(d, f, tolerance))
        << "Values beyond custom tolerance should not be nearly equal";

    // Test with negative values
    EXPECT_TRUE(Fixed64Math::IsNearlyEqual(Fixed16(-5.0), Fixed16(-5.0)))
        << "Exactly equal negative values should be nearly equal";

    Fixed16 neg1(-1.0);
    Fixed16 neg2 = neg1 - Fixed16::Epsilon() * Fixed16(0.5);
    EXPECT_TRUE(Fixed64Math::IsNearlyEqual(neg1, neg2))
        << "Nearly equal negative values should be detected";
}

// Test for values with different precision comparisons (using double as intermediary)
TEST_F(Fixed64ComparisonTest, MixedPrecisionTests) {
    // Same values with different precisions
    Fixed16 value16(1.0);
    Fixed32 value32(1.0);

    // Convert to common type (double) for comparison
    double v16 = static_cast<double>(value16);
    double v32 = static_cast<double>(value32);
    EXPECT_NEAR(v16, v32, epsilon16)
        << "Same value with different precision should be nearly equal";

    // Test with values that are close but not exactly equal
    Fixed16 a16(1.0);
    Fixed32 a32(1.0 + static_cast<double>(Fixed32::Epsilon()) * 0.5);

    double a16Double = static_cast<double>(a16);
    double a32Double = static_cast<double>(a32);

    // Use the larger epsilon for comparison
    double compareEpsilon = std::max(epsilon16, epsilon32);
    EXPECT_NEAR(a16Double, a32Double, compareEpsilon)
        << "Comparison between different precision types should use appropriate epsilon";
}

// Test for boundary conditions in IsNearlyEqual
TEST_F(Fixed64ComparisonTest, BoundaryConditions) {
    // Test epsilon calculation correctness
    Fixed16 a(1.0);
    Fixed16 b = a + Fixed16::Epsilon();
    Fixed16 c = a - Fixed16::Epsilon();

    // Default tolerance is usually ENotation3() which should be > Epsilon()
    EXPECT_TRUE(Fixed64Math::IsNearlyEqual(a, b))
        << "Value one epsilon away should be nearly equal with default tolerance";
    EXPECT_TRUE(Fixed64Math::IsNearlyEqual(a, c))
        << "Value one epsilon away should be nearly equal with default tolerance";

    // Test exact boundary of tolerance
    Fixed16 customTolerance = Fixed16::Epsilon() * Fixed16(2.0);
    Fixed16 d = a + customTolerance;
    Fixed16 e = a - customTolerance;

    EXPECT_TRUE(Fixed64Math::IsNearlyEqual(a, d, customTolerance))
        << "Value exactly at tolerance boundary should be considered nearly equal";
    EXPECT_TRUE(Fixed64Math::IsNearlyEqual(a, e, customTolerance))
        << "Value exactly at tolerance boundary should be considered nearly equal";

    // Just beyond tolerance
    Fixed16 f = a + customTolerance + Fixed16::Epsilon();
    Fixed16 g = a - customTolerance - Fixed16::Epsilon();

    EXPECT_FALSE(Fixed64Math::IsNearlyEqual(a, f, customTolerance))
        << "Value just beyond tolerance should not be considered nearly equal";
    EXPECT_FALSE(Fixed64Math::IsNearlyEqual(a, g, customTolerance))
        << "Value just beyond tolerance should not be considered nearly equal";
}

// Test for extreme values with IsNearlyEqual
TEST_F(Fixed64ComparisonTest, ExtremeValues) {
    // Test max values
    Fixed16 max = Fixed16::Max();
    Fixed16 almostMax = max - Fixed16::Epsilon() * Fixed16(0.5);

    EXPECT_TRUE(Fixed64Math::IsNearlyEqual(max, almostMax))
        << "Max value and value close to max should be nearly equal";

    // Test min values
    Fixed16 min = Fixed16::Min();
    Fixed16 almostMin = min + Fixed16::Epsilon() * Fixed16(0.5);

    EXPECT_TRUE(Fixed64Math::IsNearlyEqual(min, almostMin))
        << "Min value and value close to min should be nearly equal";

    // Test IsNearlyEqual with very small values
    Fixed16 tiny1 = Fixed16::Epsilon() * Fixed16(2.0);
    Fixed16 tiny2 = Fixed16::Epsilon() * Fixed16(2.5);

    // For very small values, relative errors can be large
    // so we need a custom tolerance
    Fixed16 smallTolerance = Fixed16::Epsilon() * Fixed16(1.0);
    EXPECT_TRUE(Fixed64Math::IsNearlyEqual(tiny1, tiny2, smallTolerance))
        << "Small values should be compared with appropriate tolerance";
}

}  // namespace math::fp::tests