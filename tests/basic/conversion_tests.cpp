#include <cmath>
#include <limits>
#include <string>

#include "fixed64.h"
#include "fixed64_math.h"
#include "gtest/gtest.h"

namespace math::fp::tests {

// Tests for conversion between different precisions
TEST(Fixed64ConversionTest, PrecisionConversion) {
    using Fixed8 = math::fp::Fixed64<8>;
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;
    using Fixed48 = math::fp::Fixed64<48>;

    // Calculate epsilon for each precision
    double epsilon8 = static_cast<double>(Fixed8::Epsilon());
    double epsilon16 = static_cast<double>(Fixed16::Epsilon());
    double epsilon32 = static_cast<double>(Fixed32::Epsilon());

    // Test conversion from lower to higher precision
    Fixed8 value8(3.14159);
    Fixed16 value16(value8);
    Fixed32 value32(value8);
    Fixed48 value48(value8);

    // Verify value remains the same after conversion
    EXPECT_NEAR(static_cast<double>(value8), static_cast<double>(value16), epsilon8);
    EXPECT_NEAR(static_cast<double>(value8), static_cast<double>(value32), epsilon8);
    EXPECT_NEAR(static_cast<double>(value8), static_cast<double>(value48), epsilon8);

    // Test conversion from higher to lower precision
    Fixed48 highValue48(3.14159265359);
    Fixed32 highValue32(highValue48);
    Fixed16 highValue16(highValue48);
    Fixed8 highValue8(highValue48);

    // Verify value is within error margin of lower precision
    EXPECT_NEAR(static_cast<double>(highValue48), static_cast<double>(highValue32), epsilon32);
    EXPECT_NEAR(static_cast<double>(highValue48), static_cast<double>(highValue16), epsilon16);
    EXPECT_NEAR(static_cast<double>(highValue48), static_cast<double>(highValue8), epsilon8);

    // Test precision loss
    Fixed48 precise(1.0 + std::pow(2.0, -40));  // 1.0 + small fraction
    Fixed16 lessPrecise(precise);

    // Verify fractional part is lost in lower precision
    EXPECT_NE(static_cast<double>(precise), static_cast<double>(Fixed48(lessPrecise)));
    EXPECT_NEAR(static_cast<double>(lessPrecise), 1.0, epsilon16);
}

// Tests for conversion between Fixed64 and floating-point types
TEST(Fixed64ConversionTest, FloatingPointConversion) {
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;

    // Calculate epsilon for precision comparisons
    double epsilon16 = static_cast<double>(Fixed16::Epsilon());
    double epsilon32 = static_cast<double>(Fixed32::Epsilon());

    // Test conversion from floating-point to fixed-point
    float floatValue = 3.14159f;
    double doubleValue = 3.14159265359;

    Fixed16 fromFloat16(floatValue);
    Fixed32 fromFloat32(floatValue);
    Fixed16 fromDouble16(doubleValue);
    Fixed32 fromDouble32(doubleValue);

    // Verify conversion precision
    EXPECT_NEAR(static_cast<float>(fromFloat16), floatValue, epsilon16);
    EXPECT_NEAR(static_cast<float>(fromFloat32), floatValue, epsilon32);
    EXPECT_NEAR(static_cast<double>(fromDouble16), doubleValue, epsilon16);
    EXPECT_NEAR(static_cast<double>(fromDouble32), doubleValue, epsilon32);

    // Test small decimal values
    float smallFloat = 1.0e-5f;
    double smallDouble = 1.0e-10;

    Fixed16 smallFromFloat16(smallFloat);
    Fixed32 smallFromFloat32(smallFloat);
    Fixed16 smallFromDouble16(smallDouble);
    Fixed32 smallFromDouble32(smallDouble);

    // Verify small values at different precisions
    EXPECT_NEAR(static_cast<double>(smallFromFloat16), smallFloat, epsilon16);
    EXPECT_NEAR(static_cast<double>(smallFromFloat32), smallFloat, epsilon32);
    EXPECT_NEAR(static_cast<double>(smallFromDouble16), smallDouble, epsilon16);
    EXPECT_NEAR(static_cast<double>(smallFromDouble32), smallDouble, epsilon32);
}

// Tests for conversion between Fixed64 and integer types
TEST(Fixed64ConversionTest, IntegerConversion) {
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;

    // Test conversion from integer to fixed-point
    int8_t int8Value = 42;
    int16_t int16Value = 12345;
    int32_t int32Value = 1234567890;

    Fixed16 fromInt8_16(int8Value);
    Fixed32 fromInt8_32(int8Value);
    Fixed16 fromInt16_16(int16Value);
    Fixed32 fromInt16_32(int16Value);
    Fixed16 fromInt32_16(int32Value);
    Fixed32 fromInt32_32(int32Value);

    // Verify integer conversion
    EXPECT_EQ(static_cast<int>(fromInt8_16), int8Value);
    EXPECT_EQ(static_cast<int>(fromInt8_32), int8Value);
    EXPECT_EQ(static_cast<int>(fromInt16_16), int16Value);
    EXPECT_EQ(static_cast<int>(fromInt16_32), int16Value);
    EXPECT_EQ(static_cast<int>(fromInt32_16), int32Value);
    EXPECT_EQ(static_cast<int>(fromInt32_32), int32Value);

    // Test conversion from fixed-point to integer
    Fixed16 value16(123.456);
    Fixed32 value32(123.456);

    // Verify integer truncation
    EXPECT_EQ(static_cast<int>(value16), 123);
    EXPECT_EQ(static_cast<int>(value32), 123);
    EXPECT_EQ(static_cast<int8_t>(value16), 123);
    EXPECT_EQ(static_cast<int16_t>(value16), 123);
    EXPECT_EQ(static_cast<int32_t>(value16), 123);
    EXPECT_EQ(static_cast<int64_t>(value16), 123);
}

// Tests for string conversion
TEST(Fixed64ConversionTest, StringConversion) {
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;

    // Calculate epsilon for precision comparisons
    double epsilon16 = static_cast<double>(Fixed16::Epsilon());
    double epsilon32 = static_cast<double>(Fixed32::Epsilon());

    // Test ToString method
    Fixed16 value16(3.14159);
    std::string str16 = value16.ToString();
    EXPECT_FALSE(str16.empty());
    EXPECT_NE(str16.find("3.14"), std::string::npos);

    Fixed32 value32(3.14159265359);
    std::string str32 = value32.ToString();
    EXPECT_FALSE(str32.empty());
    EXPECT_NE(str32.find("3.14159"), std::string::npos);

    // Test FromString method
    Fixed16 fromStr16 = Fixed16::FromString("3.14159265359");
    EXPECT_NEAR(static_cast<double>(fromStr16), 3.14159, epsilon16);

    Fixed32 fromStr32 = Fixed32::FromString("3.14159265359");
    EXPECT_NEAR(static_cast<double>(fromStr32), 3.14159265359, epsilon32 * 20)
        << " fromStr32: " << fromStr32;

    // Test invalid string
    Fixed16 invalidStr16 = Fixed16::FromString("not_a_number");
    EXPECT_EQ(invalidStr16, Fixed16::Zero());

    // Test string with special values
    Fixed16 piStr16 = Fixed16::FromString("3.14159");
    EXPECT_NEAR(static_cast<double>(piStr16), 3.14159, epsilon16);

    Fixed16 negativeStr16 = Fixed16::FromString("-42.5");
    EXPECT_NEAR(static_cast<double>(negativeStr16), -42.5, epsilon16);
}

// Tests for conversion between different Fixed64 specializations
TEST(Fixed64ConversionTest, CrossPrecisionOperations) {
    using Fixed8 = math::fp::Fixed64<8>;
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;

    // Calculate epsilon for precision comparisons
    double epsilon16 = static_cast<double>(Fixed16::Epsilon());
    double epsilon32 = static_cast<double>(Fixed32::Epsilon());

    // Create values with different precisions
    Fixed8 value8(3.5);
    Fixed16 value16(2.25);
    Fixed32 value32(1.125);

    // Test conversion during arithmetic operations
    Fixed16 sum16_8(value16 + Fixed16(value8));
    EXPECT_NEAR(static_cast<double>(sum16_8), 3.5 + 2.25, epsilon16);

    Fixed32 sum32_16(value32 + Fixed32(value16));
    EXPECT_NEAR(static_cast<double>(sum32_16), 1.125 + 2.25, epsilon32);

    Fixed32 sum32_8(value32 + Fixed32(value8));
    EXPECT_NEAR(static_cast<double>(sum32_8), 1.125 + 3.5, epsilon32);

    // Test conversion during comparison operations
    EXPECT_TRUE(value8 > Fixed8(value16));
    EXPECT_TRUE(value16 > Fixed16(value32));
    EXPECT_TRUE(value8 > Fixed8(value32));

    // Test precision differences in equality comparison
    Fixed8 approx8(1.0);
    Fixed16 approx16(1.0 + std::pow(2.0, -10));
    Fixed32 approx32(1.0 + std::pow(2.0, -20));

    // These should be equal when converted to lower precision
    EXPECT_EQ(Fixed8(approx16), approx8);
    EXPECT_EQ(Fixed8(approx32), approx8);
    EXPECT_EQ(Fixed16(approx32), Fixed16(1.0));

    // But different when compared at higher precision
    EXPECT_NE(approx16, Fixed16(approx8));
    EXPECT_NE(approx32, Fixed32(approx8));
    EXPECT_NE(approx32, Fixed32(approx16));
}

}  // namespace math::fp::tests