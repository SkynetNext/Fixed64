#include <cmath>
#include <limits>
#include <sstream>
#include <string>

#include "Fixed64.h"
#include "Fixed64Math.h"
#include "gtest/gtest.h"

namespace math::fp::tests {
double epsilon8 = static_cast<double>(math::fp::Fixed64<8>::Epsilon());
double epsilon16 = static_cast<double>(math::fp::Fixed64<16>::Epsilon());
double epsilon32 = static_cast<double>(math::fp::Fixed64<32>::Epsilon());
double epsilon40 = static_cast<double>(math::fp::Fixed64<40>::Epsilon());

// Basic construction and conversion tests
TEST(Fixed64ConstructionTest, BasicConstruction) {
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;

    // Default constructor
    Fixed16 defaultValue;
    EXPECT_EQ(defaultValue.value(), 0);

    // Integer constructor
    Fixed16 intValue(42);
    EXPECT_EQ(static_cast<int>(intValue), 42);

    // Floating-point constructor
    Fixed16 floatValue(3.14159);
    EXPECT_NEAR(static_cast<double>(floatValue), 3.14159, epsilon16);

    // Copy constructor
    Fixed16 copyValue(floatValue);
    EXPECT_EQ(copyValue.value(), floatValue.value());

    // Precision conversion
    Fixed32 highPrecision(3.14159);
    Fixed16 convertedValue(highPrecision);
    EXPECT_NEAR(static_cast<double>(convertedValue), 3.14159, epsilon16);
}

// String conversion tests
TEST(Fixed64ConstructionTest, StringConversion) {
    using Fixed = math::fp::Fixed64<16>;

    // ToString
    Fixed a(3.14159);
    std::string str = a.ToString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("3.14"), std::string::npos);

    // FromString
    Fixed b = Fixed::FromString("3.14159");
    EXPECT_NEAR(static_cast<double>(b), 3.14159, epsilon16);

    // Invalid string
    Fixed c = Fixed::FromString("invalid");
    EXPECT_EQ(c, Fixed::Zero());

    // std::to_string
    std::string stdStr = std::to_string(a);
    EXPECT_FALSE(stdStr.empty());

    // Stream operators
    std::stringstream ss;
    ss << a;
    Fixed d;
    ss >> d;
    EXPECT_NEAR(static_cast<double>(d), 3.14159, epsilon16);
}

// Precision conversion tests
TEST(Fixed64ConstructionTest, PrecisionConversion) {
    using Fixed16 = math::fp::Fixed64<16>;
    using Fixed32 = math::fp::Fixed64<32>;
    using Fixed8 = math::fp::Fixed64<8>;

    // Higher to lower precision
    Fixed32 highPrecision(3.14159265359);
    Fixed16 mediumPrecision(highPrecision);
    Fixed8 lowPrecision(highPrecision);

    // Verify precision loss is as expected
    EXPECT_NEAR(static_cast<double>(highPrecision), 3.14159265359, epsilon32);
    EXPECT_NEAR(static_cast<double>(mediumPrecision), 3.14159265359, epsilon16);
    EXPECT_NEAR(static_cast<double>(lowPrecision), 3.14159265359, epsilon8);

    // Lower to higher precision
    Fixed8 original(3.14);
    Fixed16 higher1(original);
    Fixed32 higher2(original);

    // Verify no additional precision is gained
    EXPECT_NEAR(static_cast<double>(original), static_cast<double>(higher1), epsilon16);
    EXPECT_NEAR(static_cast<double>(original), static_cast<double>(higher2), epsilon32);
}

// Edge cases for construction
TEST(Fixed64ConstructionTest, ConstructionEdgeCases) {
    using Fixed16 = math::fp::Fixed64<16>;

    // Construction from extreme values
    Fixed16 maxInt(std::numeric_limits<int>::max());
    Fixed16 minInt(std::numeric_limits<int>::min());

    EXPECT_EQ(static_cast<int>(maxInt), std::numeric_limits<int>::max());
    EXPECT_EQ(static_cast<int>(minInt), std::numeric_limits<int>::min());

    // Construction from very small floating point values
    Fixed16 tiny(1e-10);
    EXPECT_NEAR(static_cast<double>(tiny), 0.0, epsilon16);

    // Construction from very large floating point values
    Fixed16 huge(1e10);
    EXPECT_NEAR(static_cast<double>(huge), 1e10, epsilon16);
}

}  // namespace math::fp::tests