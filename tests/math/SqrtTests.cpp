#include <cmath>
#include <limits>

#include "Fixed64.h"
#include "Fixed64Math.h"
#include "gtest/gtest.h"

namespace math::fp::tests {

// Test suite for square root and related functions
class Fixed64SqrtTest : public ::testing::Test {
 protected:
    // Use different precision levels for thorough testing
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;

    // Use proper epsilon values as per best practices
    static constexpr double epsilon16 = static_cast<double>(Fixed16::Epsilon());
    static constexpr double epsilon32 = static_cast<double>(Fixed32::Epsilon());
};

// Tests for basic square root function
TEST_F(Fixed64SqrtTest, BasicSqrtFunction) {
    // Test perfect squares
    EXPECT_EQ(Fixed64Math::Sqrt(Fixed16(0.0)), Fixed16(0.0));
    EXPECT_EQ(Fixed64Math::Sqrt(Fixed16(1.0)), Fixed16(1.0));
    EXPECT_EQ(Fixed64Math::Sqrt(Fixed16(4.0)), Fixed16(2.0));
    EXPECT_EQ(Fixed64Math::Sqrt(Fixed16(9.0)), Fixed16(3.0));
    EXPECT_EQ(Fixed64Math::Sqrt(Fixed16(16.0)), Fixed16(4.0));
    EXPECT_EQ(Fixed64Math::Sqrt(Fixed16(25.0)), Fixed16(5.0));
    EXPECT_EQ(Fixed64Math::Sqrt(Fixed16(100.0)), Fixed16(10.0));

    // Test non-perfect squares
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Sqrt(Fixed16(2.0))), std::sqrt(2.0), epsilon16);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Sqrt(Fixed16(3.0))), std::sqrt(3.0), epsilon16);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Sqrt(Fixed16(5.0))), std::sqrt(5.0), epsilon16);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Sqrt(Fixed16(10.0))), std::sqrt(10.0), epsilon16);

    // Test small values
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Sqrt(Fixed16(0.01))), 0.1, epsilon16 * 2);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Sqrt(Fixed16(0.25))), 0.5, epsilon16 * 2);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Sqrt(Fixed16(0.0625))), 0.25, epsilon16 * 2);
}

// Tests for square root with different precision levels
TEST_F(Fixed64SqrtTest, SqrtPrecisionTests) {
    // Test sqrt implementations at different precisions
    std::vector<double> testValues = {0.5, 2.0, 7.0, 10.0, 123.0, 500.0};

    for (double value : testValues) {
        // Compare 16-bit precision with 32-bit precision
        Fixed16 result16 = Fixed64Math::Sqrt(Fixed16(value));
        Fixed32 result32 = Fixed64Math::Sqrt(Fixed32(value));

        // Convert to common type (double) for comparison
        double r16 = static_cast<double>(result16);
        double r32 = static_cast<double>(result32);
        double expected = std::sqrt(value);

        // Both should be close to the expected value, but 32-bit should be closer
        EXPECT_NEAR(r16, expected, epsilon16 * 2) << "16-bit Sqrt failed for value " << value;
        EXPECT_NEAR(r32, expected, epsilon32 * 2) << "32-bit Sqrt failed for value " << value;

        // The 32-bit result should be more accurate than 16-bit
        EXPECT_LE(std::abs(r32 - expected), std::abs(r16 - expected))
            << "Higher precision sqrt should be more accurate for " << value;
    }
}

// Tests for edge cases and error handling
TEST_F(Fixed64SqrtTest, SqrtEdgeCasesAndErrorHandling) {
    // Test with negative values (should return 0 as per convention)
    EXPECT_EQ(Fixed64Math::Sqrt(Fixed16(-1.0)), Fixed16(0.0))
        << "Sqrt of negative number should return 0";
    EXPECT_EQ(Fixed64Math::Sqrt(Fixed16(-100.0)), Fixed16(0.0))
        << "Sqrt of negative number should return 0";

    // Test with maximum representable value
    Fixed16 maxValue = Fixed16::Max();
    Fixed16 sqrtMax = Fixed64Math::Sqrt(maxValue);

    // The square of sqrtMax should be less than or equal to maxValue
    Fixed16 sqrtMaxSquared = sqrtMax * sqrtMax;
    EXPECT_LE(sqrtMaxSquared.value(), maxValue.value())
        << "Square of Sqrt(Max) should not exceed Max";

    // Test with very small values
    Fixed16 tiny = Fixed16::Epsilon() * Fixed16(5.0);
    Fixed16 sqrtTiny = Fixed64Math::Sqrt(tiny);

    // Should handle values near epsilon correctly
    EXPECT_GT(sqrtTiny.value(), 0) << "Sqrt of tiny positive value should be positive";
}

// Tests for validating Sqrt against other operations
TEST_F(Fixed64SqrtTest, SqrtValidationTests) {
    // Validate that sqrt(x)² = x for various values
    std::vector<double> testValues = {0.01, 0.5, 1.0, 2.0, 10.0, 100.0};

    for (double value : testValues) {
        Fixed16 x(value);
        Fixed16 sqrtX = Fixed64Math::Sqrt(x);
        Fixed16 sqrtXSquared = sqrtX * sqrtX;

        EXPECT_NEAR(static_cast<double>(sqrtXSquared), value, epsilon16 * 3)
            << "Sqrt(x)² should equal x for value " << value;
    }

    // Validate sqrt(a*b) = sqrt(a) * sqrt(b)
    Fixed16 a(4.0);
    Fixed16 b(9.0);
    Fixed16 product = a * b;

    Fixed16 sqrtProduct = Fixed64Math::Sqrt(product);
    Fixed16 sqrtA = Fixed64Math::Sqrt(a);
    Fixed16 sqrtB = Fixed64Math::Sqrt(b);
    Fixed16 sqrtASqrtB = sqrtA * sqrtB;

    EXPECT_NEAR(static_cast<double>(sqrtProduct), static_cast<double>(sqrtASqrtB), epsilon16)
        << "Sqrt(a*b) should equal Sqrt(a)*Sqrt(b)";
}

}  // namespace math::fp::tests