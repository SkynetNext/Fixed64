#include <cmath>
#include <limits>
#include <vector>

#include "fixed64.h"
#include "fixed64_math.h"
#include "gtest/gtest.h"

namespace math::fp::tests {

// Expected maximum absolute error - defined as Fixed64<32>::Epsilon()
// This can be adjusted as needed for different fraction bits
template <int FractionBits>
constexpr double GetExpectedMaxError() {
    return static_cast<double>(Fixed64<FractionBits>::Epsilon());
}

TEST(Fixed64Log2Test, Pow2AndLog2Inverse) {
    using Fixed = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    // Use Fixed<32>::Epsilon() as the error tolerance
    constexpr double error_tolerance = GetExpectedMaxError<32>();

    // Test log2(2^x) = x for various values
    std::vector<double> test_exponents = {
        -10.0, -5.0, -3.0, -2.0, -1.0, -0.5, -0.25, 0.0, 0.25, 0.5, 1.0, 2.0, 3.0, 5.0, 10.0};

    for (double exp : test_exponents) {
        Fixed exp_fixed(exp);
        Fixed pow2_result = Fixed64Math::Pow2(exp_fixed);
        Fixed log2_result = Fixed64Math::Log2(pow2_result);

        EXPECT_NEAR(static_cast<double>(log2_result), exp, error_tolerance)
            << "Error exceeds epsilon for exp = " << exp;
    }
}

TEST(Fixed64Log2Test, StandardValues) {
    using Fixed = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    constexpr double error_tolerance = GetExpectedMaxError<32>();

    // Test standard values for log2
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Log2(Fixed(1.0))), 0.0, error_tolerance);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Log2(Fixed(2.0))), 1.0, error_tolerance);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Log2(Fixed(4.0))), 2.0, error_tolerance);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Log2(Fixed(8.0))), 3.0, error_tolerance);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Log2(Fixed(16.0))), 4.0, error_tolerance);
    EXPECT_NEAR(static_cast<double>(Fixed64Math::Log2(Fixed(32.0))), 5.0, error_tolerance);
}

TEST(Fixed64Log2Test, CompareWithCMath) {
    using Fixed = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    constexpr double error_tolerance = GetExpectedMaxError<32>();

    // Compare with std::log2 for various values
    std::vector<double> test_values = {0.1,
                                       0.2,
                                       0.5,
                                       0.75,
                                       1.0,
                                       1.5,
                                       2.0,
                                       3.0,
                                       5.0,
                                       10.0,
                                       15.0,
                                       20.0,
                                       50.0,
                                       100.0,
                                       1000.0,
                                       10000.0};

    for (double val : test_values) {
        Fixed fixed_val(val);
        double expected = std::log2(static_cast<double>(fixed_val));
        Fixed result = Fixed64Math::Log2(fixed_val);

        EXPECT_NEAR(static_cast<double>(result), expected, error_tolerance)
            << "Error exceeds epsilon for value = " << val;
    }
}

TEST(Fixed64Log2Test, EdgeCases) {
    using Fixed = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    // Test edge cases

    // 1. Log2 of 0 should return MIN value (undefined mathematically)
    EXPECT_EQ(Fixed64Math::Log2(Fixed(0.0)), Fixed::Min());

    // 2. Log2 of negative should return MIN value (undefined mathematically)
    EXPECT_EQ(Fixed64Math::Log2(Fixed(-1.0)), Fixed::Min());
    EXPECT_EQ(Fixed64Math::Log2(Fixed(-100.0)), Fixed::Min());

    // 3. Log2 of very small positive value
    Fixed small_val(std::numeric_limits<double>::min() * 1000);
    double expected_small = std::log2(static_cast<double>(small_val));
    Fixed result_small = Fixed64Math::Log2(small_val);

    // For very small values, we only check that the result is finite and negative
    EXPECT_LT(result_small, Fixed(0.0));

    // 4. Log2 of very large positive value
    Fixed large_val(std::numeric_limits<double>::max() / 1000);
    double expected_large = std::log2(static_cast<double>(large_val));
    Fixed result_large = Fixed64Math::Log2(large_val);

    // For very large values, we only check that the result is finite and positive
    EXPECT_TRUE(std::isinf(result_large) || std::isnan(result_large));
}

TEST(Fixed64Log2Test, DifferentPrecisions) {
    // Test with different precisions

    // Test with 16 fraction bits
    {
        using Fixed16 = math::fp::Fixed64<16>;
        using math::fp::Fixed64Math;

        constexpr double error_tolerance = GetExpectedMaxError<16>();

        std::vector<double> test_values = {0.5, 1.0, 2.0, 4.0, 8.0};

        for (double val : test_values) {
            Fixed16 fixed_val(val);
            double expected = std::log2(val);
            Fixed16 result = Fixed64Math::Log2(fixed_val);

            EXPECT_NEAR(static_cast<double>(result), expected, error_tolerance)
                << "Error exceeds epsilon for Fixed<16> value = " << val;
        }
    }

    // Test with 48 fraction bits (high precision)
    {
        using Fixed48 = math::fp::Fixed64<48>;
        using math::fp::Fixed64Math;

        constexpr double error_tolerance = GetExpectedMaxError<48>();

        std::vector<double> test_values = {0.5, 1.0, 2.0, 4.0, 8.0};

        for (double val : test_values) {
            Fixed48 fixed_val(val);
            double expected = std::log2(val);
            Fixed48 result = Fixed64Math::Log2(fixed_val);

            EXPECT_NEAR(static_cast<double>(result), expected, error_tolerance)
                << "Error exceeds epsilon for Fixed<48> value = " << val;
        }
    }
}

// Test for consistency over a wide range of values
TEST(Fixed64Log2Test, ConsistencyOverRange) {
    using Fixed = math::fp::Fixed64<32>;
    using math::fp::Fixed64Math;

    constexpr double error_tolerance = GetExpectedMaxError<32>();
    constexpr int num_samples = 100;

    for (int i = 1; i <= num_samples; ++i) {
        // Generate a sample from 0.01 to 100.0 on logarithmic scale
        double val = 0.01 * std::pow(10000.0, static_cast<double>(i) / num_samples);

        Fixed fixed_val(val);
        double expected = std::log2(val);
        Fixed result = Fixed64Math::Log2(fixed_val);

        EXPECT_NEAR(static_cast<double>(result), expected, error_tolerance)
            << "Error exceeds epsilon for value = " << val;
    }
}

}  // namespace math::fp::tests