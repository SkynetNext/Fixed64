#include <cmath>
#include <string>
#include <vector>
#include "Fixed64.h"
#include "gtest/gtest.h"

namespace math::fp::tests {

// Tests for Fixed64::ToString method
TEST(Fixed64StringConversionTest, ToStringMethod) {
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;
    using Fixed40 = math::fp::Fixed64<40>;

    // Test integer values
    {
        Fixed16 zero;
        Fixed16 one(1);
        Fixed16 negOne(-1);
        Fixed16 large(1000000);

        EXPECT_EQ(zero.ToString(), "0");
        EXPECT_EQ(one.ToString(), "1");
        EXPECT_EQ(negOne.ToString(), "-1");
        EXPECT_NE(large.ToString().find("1000000"), std::string::npos);
    }

    // Test fractional values
    {
        Fixed16 pi(3.14159);
        Fixed32 piHighPrecision(3.14159265359);
        Fixed16 smallFraction(0.0001);
        Fixed16 negFraction(-42.5);

        // Verify basic fractional formatting
        std::string piStr = pi.ToString();
        EXPECT_NE(piStr.find("3.14"), std::string::npos);

        // Higher precision should have more decimal places
        std::string piHPStr = piHighPrecision.ToString();
        EXPECT_NE(piHPStr.find("3.141592"), std::string::npos);

        // Small fractions should be preserved
        std::string smallStr = smallFraction.ToString();
        EXPECT_NE(smallStr.find("0.0001"), std::string::npos);

        // Negative fractions should have correct sign
        std::string negStr = negFraction.ToString();
        EXPECT_NE(negStr.find("-42.5"), std::string::npos);
    }

    // Test precision boundaries
    {
        // Value near 1.0 but slightly less
        Fixed40 nearOne(Fixed40::One().value() - 1, detail::nothing{});
        EXPECT_NE(nearOne.ToString().find("0.9"), std::string::npos);

        // Smallest positive value (epsilon)
        Fixed16 epsilon(Fixed16::Epsilon());
        EXPECT_NE(epsilon.ToString().find("0.0"), std::string::npos);

        // Maximum value
        Fixed16 maxValue(Fixed16::Max());
        EXPECT_FALSE(maxValue.ToString().empty());

        // Minimum value
        Fixed16 minValue(Fixed16::Min());
        EXPECT_EQ(minValue.ToString()[0], '-');
    }

    // Test trailing zeros handling
    {
        Fixed16 exactInteger(42);
        EXPECT_EQ(exactInteger.ToString(), "42");  // No decimal part

        Fixed16 exactDecimal(42.5);
        EXPECT_EQ(exactDecimal.ToString(), "42.5");  // No trailing zeros

        Fixed16 trailingZeros(42.500);
        EXPECT_EQ(trailingZeros.ToString(), "42.5");  // Trailing zeros removed
    }

    // Test zero values
    {
        Fixed16 zero;
        EXPECT_EQ(zero.ToString(), "0");

        Fixed16 negZero(-0.0);
        EXPECT_EQ(negZero.ToString(), "0");  // Should normalize to "0"
    }
}

// Tests for FromString method
TEST(Fixed64StringConversionTest, FromStringMethod) {
    using Fixed16 = math::fp::Fixed64<16>;

    // Test normal values
    {
        Fixed16 parsed = Fixed16::FromString("123.456");
        EXPECT_NEAR(static_cast<double>(parsed), 123.456, 0.0001);

        Fixed16 parsedNegative = Fixed16::FromString("-987.654");
        EXPECT_NEAR(static_cast<double>(parsedNegative), -987.654, 0.0001);
    }

    // Test integer values
    {
        Fixed16 parsedInteger = Fixed16::FromString("42");
        EXPECT_EQ(parsedInteger.value(), Fixed16(42).value());

        Fixed16 parsedNegInteger = Fixed16::FromString("-42");
        EXPECT_EQ(parsedNegInteger.value(), Fixed16(-42).value());
    }

    // Test zero values
    {
        Fixed16 parsedZero = Fixed16::FromString("0");
        EXPECT_EQ(parsedZero.value(), 0);

        Fixed16 parsedZeroDecimal = Fixed16::FromString("0.0");
        EXPECT_EQ(parsedZeroDecimal.value(), 0);

        Fixed16 parsedNegZero = Fixed16::FromString("-0");
        EXPECT_EQ(parsedNegZero.value(), 0);
    }

    // Test various decimal formats
    {
        Fixed16 parsedNoIntPart = Fixed16::FromString(".5");
        EXPECT_NEAR(static_cast<double>(parsedNoIntPart), 0.5, 0.0001);

        Fixed16 parsedLeadingZeros = Fixed16::FromString("00123.456");
        EXPECT_NEAR(static_cast<double>(parsedLeadingZeros), 123.456, 0.0001);

        Fixed16 parsedTrailingZeros = Fixed16::FromString("123.4560000");
        EXPECT_NEAR(static_cast<double>(parsedTrailingZeros), 123.456, 0.0001);
    }

    // Test scientific notation
    {
        Fixed16 parsedScientific = Fixed16::FromString("1.23e2");
        EXPECT_NEAR(static_cast<double>(parsedScientific), 123.0, 0.1);

        Fixed16 parsedScientificNeg = Fixed16::FromString("1.23e-2");
        EXPECT_NEAR(static_cast<double>(parsedScientificNeg), 0.0123, 0.0001);

        Fixed16 parsedCapitalE = Fixed16::FromString("4.56E3");
        EXPECT_NEAR(static_cast<double>(parsedCapitalE), 4560.0, 0.1);
    }

    // Test invalid inputs
    {
        Fixed16 parsedEmpty = Fixed16::FromString("");
        EXPECT_EQ(parsedEmpty.value(), 0);

        Fixed16 parsedInvalid = Fixed16::FromString("not_a_number");
        EXPECT_EQ(parsedInvalid.value(), 0);

        Fixed16 parsedPartialValid = Fixed16::FromString("123abc");
        EXPECT_EQ(parsedPartialValid.value(), Fixed16(123).value());

        Fixed16 parsedSpaces = Fixed16::FromString("   ");
        EXPECT_EQ(parsedSpaces.value(), 0);
    }

    // Test boundary values
    {
        // Test near max value
        std::string largeValue = "134217727.0";  // Close to Fixed16::Max()
        Fixed16 parsedLarge = Fixed16::FromString(largeValue);
        EXPECT_LT(parsedLarge, Fixed16::Max());

        // Test very large value
        std::string tooLarge = "9999999999999.0";  // Beyond Fixed16::Max()
        Fixed16 parsedTooLarge = Fixed16::FromString(tooLarge);
        EXPECT_TRUE(parsedTooLarge.value() > 0);  // Should handle gracefully

        // Test very small value
        std::string verySmall = "0.0000001";  // Below Fixed16::Epsilon()
        Fixed16 parsedVerySmall = Fixed16::FromString(verySmall);
        // Either 0 or smallest representable value is acceptable
        EXPECT_TRUE(parsedVerySmall.value() == 0
                    || parsedVerySmall.value() == Fixed16::Epsilon().value());
    }
}

// Test round-trip conversion (ToString -> FromString)
TEST(Fixed64StringConversionTest, RoundTripConversion) {
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;

    std::vector<double> testValues = {0.0, 1.0, -1.0, 3.14159, -42.5, 0.0001, 1000000.0};

    for (double value : testValues) {
        // First conversion: double -> Fixed -> string
        Fixed16 fixed(value);
        std::string str = fixed.ToString();

        // Second conversion: string -> Fixed
        Fixed16 roundTrip = Fixed16::FromString(str);

        // Compare the original Fixed value with the round-trip value
        EXPECT_EQ(fixed.value(), roundTrip.value());
    }

    // Test with higher precision
    {
        Fixed32 highPrecision(3.14159265359);
        std::string str = highPrecision.ToString();
        Fixed32 roundTrip = Fixed32::FromString(str);

        // Expect very close values but maybe not exact due to string representation limits
        EXPECT_NEAR(static_cast<double>(highPrecision),
                    static_cast<double>(roundTrip),
                    static_cast<double>(Fixed32::Epsilon() * 20))
            << "highPrecision: " << str << ", roundTrip " << roundTrip;
    }
}

// Tests for cross-precision string conversion
TEST(Fixed64StringConversionTest, CrossPrecisionStringConversion) {
    using Fixed8 = math::fp::Fixed64<8>;
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;

    // Test converting from higher precision to lower precision via string
    {
        Fixed32 highPrecision(3.14159265359);
        std::string highStr = highPrecision.ToString();

        // Parse the high precision string into lower precision formats
        Fixed16 medium = Fixed16::FromString(highStr);
        Fixed8 low = Fixed8::FromString(highStr);

        // Verify expected precision loss
        EXPECT_NEAR(static_cast<double>(medium), 3.14159, 0.0001);
        EXPECT_NEAR(static_cast<double>(low), 3.14, 0.01);
    }

    // Test small values that might be below epsilon for some precisions
    {
        // Create a small value as string
        std::string smallValueStr = "0.000005";

        // Parse into different precision formats
        Fixed32 highPrecision = Fixed32::FromString(smallValueStr);
        Fixed16 mediumPrecision = Fixed16::FromString(smallValueStr);
        Fixed8 lowPrecision = Fixed8::FromString(smallValueStr);

        // Check if values are preserved based on epsilon
        bool highPreservesValue = static_cast<double>(highPrecision) > 0;
        bool mediumPreservesValue = static_cast<double>(mediumPrecision) > 0;
        bool lowPreservesValue = static_cast<double>(lowPrecision) > 0;

        // Higher precision should preserve smaller values
        if (0.000005 > static_cast<double>(Fixed32::Epsilon())) {
            EXPECT_TRUE(highPreservesValue);
        }

        if (0.000005 > static_cast<double>(Fixed16::Epsilon())) {
            EXPECT_TRUE(mediumPreservesValue);
        } else {
            EXPECT_FALSE(mediumPreservesValue);
        }

        if (0.000005 > static_cast<double>(Fixed8::Epsilon())) {
            EXPECT_TRUE(lowPreservesValue);
        } else {
            EXPECT_FALSE(lowPreservesValue);
        }
    }
}

}  // namespace math::fp::tests
