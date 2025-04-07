#include <cmath>
#include <limits>

#include "fixed64.h"
#include "fixed64_math.h"
#include "gtest/gtest.h"

namespace math::fp::tests {

// Test suite for Abs and Sign functions
class Fixed64AbsSignTest : public ::testing::Test {
 protected:
    // Use different precision levels for thorough testing
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;

    // Use proper epsilon values as per best practices
    static constexpr double epsilon16 = static_cast<double>(Fixed16::Epsilon());
    static constexpr double epsilon32 = static_cast<double>(Fixed32::Epsilon());
};

// Tests for the Abs function
TEST_F(Fixed64AbsSignTest, AbsFunction) {
    // Test positive values (should remain unchanged)
    EXPECT_EQ(Fixed64Math::Abs(Fixed16(5.0)), Fixed16(5.0))
        << "Abs of positive value should remain unchanged";
    EXPECT_EQ(Fixed64Math::Abs(Fixed16(0.5)), Fixed16(0.5))
        << "Abs of positive fraction should remain unchanged";

    // Test negative values (should become positive)
    EXPECT_EQ(Fixed64Math::Abs(Fixed16(-5.0)), Fixed16(5.0))
        << "Abs of negative value should return positive equivalent";
    EXPECT_EQ(Fixed64Math::Abs(Fixed16(-0.5)), Fixed16(0.5))
        << "Abs of negative fraction should return positive equivalent";

    // Test zero
    EXPECT_EQ(Fixed64Math::Abs(Fixed16::Zero()), Fixed16::Zero())
        << "Abs of zero should remain zero";

    // Test precision boundary values
    Fixed16 epsilon = Fixed16::Epsilon();
    EXPECT_EQ(Fixed64Math::Abs(epsilon), epsilon) << "Abs of epsilon should remain epsilon";
    EXPECT_EQ(Fixed64Math::Abs(-epsilon), epsilon)
        << "Abs of negative epsilon should return positive epsilon";

    // Test extreme values
    Fixed16 maxValue = Fixed16::Max();
    Fixed16 minValue = Fixed16::Min();

    EXPECT_EQ(Fixed64Math::Abs(maxValue), maxValue) << "Abs of max value should remain max value";

    // This might depend on implementation - Min could be one more than Max in magnitude
    if (-static_cast<double>(minValue) <= static_cast<double>(maxValue)) {
        EXPECT_EQ(Fixed64Math::Abs(minValue), -minValue)
            << "Abs of min value should return positive equivalent";
    } else {
        // If Min is one larger in magnitude than Max, Abs might saturate
        EXPECT_EQ(Fixed64Math::Abs(minValue), maxValue)
            << "Abs of min value should saturate at max value";
    }
}

// Tests for the Sign function
TEST_F(Fixed64AbsSignTest, SignFunction) {
    // Test positive values
    EXPECT_EQ(Fixed64Math::Sign(Fixed16(5.0)), 1) << "Sign of positive value should be 1";
    EXPECT_EQ(Fixed64Math::Sign(Fixed16(0.01)), 1) << "Sign of small positive value should be 1";

    // Test negative values
    EXPECT_EQ(Fixed64Math::Sign(Fixed16(-5.0)), -1) << "Sign of negative value should be -1";
    EXPECT_EQ(Fixed64Math::Sign(Fixed16(-0.01)), -1) << "Sign of small negative value should be -1";

    // Test zero - according to industry standard conventions
    EXPECT_EQ(Fixed64Math::Sign(Fixed16::Zero()), 0)
        << "Sign of zero should be 0 according to standard math conventions";

    // Test precision boundary values
    Fixed16 epsilon = Fixed16::Epsilon();
    EXPECT_EQ(Fixed64Math::Sign(epsilon), 1) << "Sign of epsilon should be 1";
    EXPECT_EQ(Fixed64Math::Sign(-epsilon), -1) << "Sign of negative epsilon should be -1";
}

// Tests for Abs-Sign relationship (Abs(x) * Sign(x) = x)
TEST_F(Fixed64AbsSignTest, AbsSignRelationship) {
    std::vector<double> testValues = {
        -100.0, -10.0, -1.0, -0.1, -0.01, 0.0, 0.01, 0.1, 1.0, 10.0, 100.0};

    for (double value : testValues) {
        Fixed16 fixedValue(value);
        Fixed16 absValue = Fixed64Math::Abs(fixedValue);
        int signValue = Fixed64Math::Sign(fixedValue);

        // Skip zero for multiplication test (0 * anything = 0)
        if (signValue == 0) {
            // For zero, just verify abs(0) is 0
            EXPECT_EQ(absValue, Fixed16::Zero()) << "abs(0) should equal 0";
        } else {
            // For non-zero, verify abs(x) * sign(x) = x
            Fixed16 product = absValue * Fixed16(signValue);
            EXPECT_NEAR(static_cast<double>(product), static_cast<double>(fixedValue), epsilon16)
                << "sign(x) * abs(x) should equal x for value " << value;
        }

        // Test that abs(x) ≥ 0 for all x
        EXPECT_GE(absValue.value(), Fixed16::Zero().value())
            << "abs(x) should always be non-negative for value " << value;

        // Test that sign(x) is -1, 0, or 1
        EXPECT_TRUE(signValue == -1 || signValue == 0 || signValue == 1)
            << "sign(x) should be -1, 0, or 1 for value " << value;
    }
}

// Tests with different precision levels
TEST_F(Fixed64AbsSignTest, PrecisionTests) {
    // Compare implementations across different precision levels
    std::vector<double> testValues = {-100.0, -1.5, -0.01, 0.0, 0.01, 1.5, 100.0};

    for (double value : testValues) {
        Fixed16 value16(value);
        Fixed32 value32(value);

        // Abs should behave the same regardless of precision
        Fixed16 abs16 = Fixed64Math::Abs(value16);
        Fixed32 abs32 = Fixed64Math::Abs(value32);

        EXPECT_NEAR(static_cast<double>(abs16), static_cast<double>(abs32), epsilon16)
            << "Abs should be precision-independent for value " << value;

        // Sign should behave the same regardless of precision
        int sign16 = Fixed64Math::Sign(value16);
        int sign32 = Fixed64Math::Sign(value32);

        EXPECT_EQ(sign16, sign32) << "Sign should be precision-independent for value " << value;
    }
}

}  // namespace math::fp::tests