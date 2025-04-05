#include <cmath>
#include <limits>

#include "Fixed64.h"
#include "Fixed64Math.h"
#include "gtest/gtest.h"

namespace math::fp::tests {

inline bool IsWithinRelativeError(double actual,
                                  double expected,
                                  double max_relative_error = 0.0001) {
    if (std::abs(expected) < 1e-10) {
        return std::abs(actual) < max_relative_error;
    }
    double relative_error = std::abs((actual - expected) / expected);
    return relative_error <= max_relative_error;
}

#define EXPECT_RELATIVE_ERROR(actual, expected, max_rel_error)                          \
    EXPECT_TRUE(IsWithinRelativeError(actual, expected, max_rel_error))                 \
        << "Relative error: " << std::abs(((actual) - (expected)) / (expected)) * 100.0 \
        << "%, Actual: " << (actual) << ", Expected: " << (expected)

// Tests for Pow2 function
TEST(Fixed64ExponentialTest, Pow2Function) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with zero exponent
    EXPECT_EQ(Fixed64Math::Pow2(Fixed::Zero()), Fixed::One());

    // Test with positive integer exponents
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Pow2(Fixed(1.0))), 2.0, 0.0001);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Pow2(Fixed(2.0))), 4.0, 0.0001);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Pow2(Fixed(3.0))), 8.0, 0.0001);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Pow2(Fixed(4.0))), 16.0, 0.0001);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Pow2(Fixed(5.0))), 32.0, 0.0001);

    // Test with negative integer exponents
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Pow2(Fixed(-1.0))), 0.5, 0.0001);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Pow2(Fixed(-2.0))), 0.25, 0.0001);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Pow2(Fixed(-3.0))), 0.125, 0.0001);

    // Test with fractional exponents
    Fixed value05(0.5);
    double dbl_value05 = static_cast<double>(value05);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Pow2(value05)), std::pow(2.0, dbl_value05), 0.0001);

    Fixed value15(1.5);
    double dbl_value15 = static_cast<double>(value15);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Pow2(value15)), std::pow(2.0, dbl_value15), 0.0001);

    Fixed valueNeg05(-0.5);
    double dbl_valueNeg05 = static_cast<double>(valueNeg05);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Pow2(valueNeg05)), std::pow(2.0, dbl_valueNeg05), 0.0001);

    // Test with very small exponents
    Fixed small_pos(0.001);
    double dbl_small_pos = static_cast<double>(small_pos);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Pow2(small_pos)), std::pow(2.0, dbl_small_pos), 0.0001);

    Fixed small_neg(-0.001);
    double dbl_small_neg = static_cast<double>(small_neg);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Pow2(small_neg)), std::pow(2.0, dbl_small_neg), 0.0001);

    // Test with large exponents (within valid range)
    int maxExponent = 63 - 16;  // For Fixed<16>
    Fixed large_exp(maxExponent - 1);
    double dbl_large_exp = static_cast<double>(large_exp);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Pow2(large_exp)), std::pow(2.0, dbl_large_exp), 0.0001);

    // Test with exponents beyond valid range
    EXPECT_EQ(Fixed64Math::Pow2(Fixed(maxExponent + 10)), Fixed::Max());
    EXPECT_EQ(Fixed64Math::Pow2(Fixed(-100)), Fixed::Zero());
}

// Tests for Log function (natural logarithm)
TEST(Fixed64ExponentialTest, LogFunction) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with e^x values
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Log(Fixed(1.0))), 0.0, 0.0001);

    Fixed e_value(std::exp(1.0));
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Log(e_value)), 1.0, 0.0001);

    Fixed e_squared(std::exp(2.0));
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Log(e_squared)), 2.0, 0.0001);

    // Test with other values
    Fixed val2(2.0);
    double dbl_val2 = static_cast<double>(val2);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Log(val2)), std::log(dbl_val2), 0.0001);

    Fixed val10(10.0);
    double dbl_val10 = static_cast<double>(val10);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Log(val10)), std::log(dbl_val10), 0.0001);

    Fixed val05(0.5);
    double dbl_val05 = static_cast<double>(val05);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Log(val05)), std::log(dbl_val05), 0.0001);

    // Test with very small positive values
    Fixed small_val(0.001);
    double dbl_small_val = static_cast<double>(small_val);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Log(small_val)), std::log(dbl_small_val), 0.0001);

    // Test with large values
    Fixed large_val(1000.0);
    double dbl_large_val = static_cast<double>(large_val);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Log(large_val)), std::log(dbl_large_val), 0.0001);

    // Test with invalid inputs (should return minimum value)
    EXPECT_EQ(Fixed64Math::Log(Fixed::Zero()), Fixed::Min());
    EXPECT_EQ(Fixed64Math::Log(Fixed(-1.0)), Fixed::Min());

    // Test logarithm base 2 calculation using Log and ln(2)
    Fixed val2_forBase2(2.0);
    double dbl_val2_forBase2 = static_cast<double>(val2_forBase2);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Log(val2_forBase2) / Fixed64<16>::Ln2()),
                          std::log(dbl_val2_forBase2) / std::log(2.0),
                          0.0001);

    Fixed val4(4.0);
    double dbl_val4 = static_cast<double>(val4);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Log(val4) / Fixed64<16>::Ln2()),
                          std::log(dbl_val4) / std::log(2.0),
                          0.0001);

    Fixed val8(8.0);
    double dbl_val8 = static_cast<double>(val8);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Log(val8) / Fixed64<16>::Ln2()),
                          std::log(dbl_val8) / std::log(2.0),
                          0.0001);

    // Test logarithm base 10 calculation using Log and ln(10)
    Fixed fixed_10(10.0);
    double dbl_fixed_10 = static_cast<double>(fixed_10);
    const auto ln10 = Fixed64Math::Log(fixed_10);

    Fixed val10_forBase10(10.0);
    double dbl_val10_forBase10 = static_cast<double>(val10_forBase10);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Log(val10_forBase10) / ln10),
                          std::log(dbl_val10_forBase10) / std::log(dbl_fixed_10),
                          0.0001);

    Fixed val100(100.0);
    double dbl_val100 = static_cast<double>(val100);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Log(val100) / ln10),
                          std::log(dbl_val100) / std::log(dbl_fixed_10),
                          0.0001);

    Fixed val1000(1000.0);
    double dbl_val1000 = static_cast<double>(val1000);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Log(val1000) / ln10),
                          std::log(dbl_val1000) / std::log(dbl_fixed_10),
                          0.0001);
}

// Tests for Exp function
TEST(Fixed64ExponentialTest, ExpFunction) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with zero
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Exp(Fixed(0.0))), 1.0, 0.0001);

    // Test with positive values
    Fixed val1(1.0);
    double dbl_val1 = static_cast<double>(val1);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Exp(val1)), std::exp(dbl_val1), 0.0001);

    Fixed val2(2.0);
    double dbl_val2 = static_cast<double>(val2);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Exp(val2)), std::exp(dbl_val2), 0.0001);

    Fixed val05(0.5);
    double dbl_val05 = static_cast<double>(val05);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Exp(val05)), std::exp(dbl_val05), 0.0001);

    // Test with negative values
    Fixed neg_val1(-1.0);
    double dbl_neg_val1 = static_cast<double>(neg_val1);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Exp(neg_val1)), std::exp(dbl_neg_val1), 0.0001);

    Fixed neg_val2(-2.0);
    double dbl_neg_val2 = static_cast<double>(neg_val2);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Exp(neg_val2)), std::exp(dbl_neg_val2), 0.0001);

    Fixed neg_val05(-0.5);
    double dbl_neg_val05 = static_cast<double>(neg_val05);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Exp(neg_val05)), std::exp(dbl_neg_val05), 0.0001);

    // Test with very small values
    Fixed small_val(0.001);
    double dbl_small_val = static_cast<double>(small_val);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Exp(small_val)), std::exp(dbl_small_val), 0.0001);

    Fixed neg_small_val(-0.001);
    double dbl_neg_small_val = static_cast<double>(neg_small_val);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Exp(neg_small_val)), std::exp(dbl_neg_small_val), 0.0001);

    // Test with large values (should handle overflow)
    EXPECT_EQ(Fixed64Math::Exp(Fixed(100.0)), Fixed::Max());
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Exp(Fixed(-100.0))), 0.0, 0.0001);

    // Test the relationship between Exp and Log
    for (double x = 0.1; x < 5.0; x += 0.5) {
        Fixed value(x);
        double dbl_value = static_cast<double>(value);
        Fixed logValue = Fixed64Math::Log(value);
        Fixed expLogValue = Fixed64Math::Exp(logValue);
        EXPECT_RELATIVE_ERROR(static_cast<double>(expLogValue), dbl_value, 0.00033);
    }
}

// Tests for Pow function
TEST(Fixed64ExponentialTest, PowFunction) {
    using Fixed = math::fp::Fixed64<16>;
    using math::fp::Fixed64Math;

    // Test with base = 0
    EXPECT_EQ(Fixed64Math::Pow(Fixed(0.0), Fixed(5.0)), Fixed(0.0));

    // Test with exponent = 0
    EXPECT_EQ(Fixed64Math::Pow(Fixed(5.0), Fixed(0.0)), Fixed(1.0));

    // Test with exponent = 1
    EXPECT_EQ(Fixed64Math::Pow(Fixed(5.0), Fixed(1.0)), Fixed(5.0));

    // Test with integer exponents
    Fixed base2(2.0);
    double dbl_base2 = static_cast<double>(base2);
    Fixed exp3(3.0);
    double dbl_exp3 = static_cast<double>(exp3);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Pow(base2, exp3)), std::pow(dbl_base2, dbl_exp3), 0.0001);

    Fixed base3(3.0);
    double dbl_base3 = static_cast<double>(base3);
    Fixed exp2(2.0);
    double dbl_exp2 = static_cast<double>(exp2);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Pow(base3, exp2)), std::pow(dbl_base3, dbl_exp2), 0.0001);

    Fixed base4(4.0);
    double dbl_base4 = static_cast<double>(base4);
    Fixed exp05(0.5);
    double dbl_exp05 = static_cast<double>(exp05);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Pow(base4, exp05)),
                          std::pow(dbl_base4, dbl_exp05),
                          0.0001);

    // Test with negative base and integer exponent
    Fixed neg_base2(-2.0);
    double dbl_neg_base2 = static_cast<double>(neg_base2);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Pow(neg_base2, exp2)),
                          std::pow(dbl_neg_base2, dbl_exp2),
                          0.0001);

    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Pow(neg_base2, exp3)),
                          std::pow(dbl_neg_base2, dbl_exp3),
                          0.0001);

    // Test with negative base and non-integer exponent (should return 0)
    EXPECT_EQ(Fixed64Math::Pow(Fixed(-2.0), Fixed(0.5)), Fixed(0.0));

    // Test with negative exponents
    Fixed neg_exp1(-1.0);
    double dbl_neg_exp1 = static_cast<double>(neg_exp1);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Pow(base2, neg_exp1)),
                          std::pow(dbl_base2, dbl_neg_exp1),
                          0.0001);

    Fixed neg_exp2(-2.0);
    double dbl_neg_exp2 = static_cast<double>(neg_exp2);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Pow(base2, neg_exp2)),
                          std::pow(dbl_base2, dbl_neg_exp2),
                          0.0001);

    // Test with fractional exponents
    Fixed base9(9.0);
    double dbl_base9 = static_cast<double>(base9);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Pow(base9, exp05)),
                          std::pow(dbl_base9, dbl_exp05),
                          0.0001);

    Fixed base27(27.0);
    double dbl_base27 = static_cast<double>(base27);
    Fixed exp_third(1.0 / 3.0);
    double dbl_exp_third = static_cast<double>(exp_third);
    EXPECT_RELATIVE_ERROR(static_cast<double>(Fixed64Math::Pow(base27, exp_third)),
                          std::pow(dbl_base27, dbl_exp_third),
                          0.0001);

    // Test with unsigned integer exponent overload
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Pow(base2, 3u)), std::pow(dbl_base2, 3.0), 0.0001);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Pow(base3, 2u)), std::pow(dbl_base3, 2.0), 0.0001);

    // Test with signed integer exponent overload
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Pow(base2, 3)), std::pow(dbl_base2, 3.0), 0.0001);
    EXPECT_RELATIVE_ERROR(
        static_cast<double>(Fixed64Math::Pow(base2, -2)), std::pow(dbl_base2, -2.0), 0.0001);
}

}  // namespace math::fp::tests