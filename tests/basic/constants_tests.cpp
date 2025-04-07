#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include "fixed64.h"

// Define missing constants if needed
#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 (M_PI / 2.0)
#define M_PI_4 (M_PI / 4.0)
#endif

#ifndef M_E
#define M_E 2.7182818284590452354
#endif

#ifndef M_LN2
#define M_LN2 0.693147180559945309417
#endif

#ifndef M_LOG2E
#define M_LOG2E 1.44269504088896340736
#endif

#ifndef M_LOG10_2
#define M_LOG10_2 0.301029995663981195214
#endif

// Helper for testing constants with a specific error tolerance
template <int P>
void TestConstant(const char* name,
                  double expected,
                  math::fp::Fixed64<P> actual,
                  double tolerance) {
    double actual_double = static_cast<double>(actual);
    double real_expected = static_cast<double>(math::fp::Fixed64<P>(expected));
    double error = std::abs(actual_double - real_expected);
    double rel_error = error / std::abs(real_expected);

    // Output detailed info for debugging
    std::cout << "Testing " << name << " with P=" << P << ":\n";
    std::cout << "  Expected: " << expected << "\n";
    std::cout << "  Actual:   " << actual_double << "\n";
    std::cout << "  Error:    " << error << " (relative: " << rel_error << ")\n";
    std::cout << "  Raw bits: 0x" << std::hex << actual.value() << std::dec << "\n";

    // Verify the error is within tolerance
    EXPECT_LT(rel_error, tolerance) << name << " with P=" << P << " exceeded error tolerance";
}

// Test all mathematical constants for P=16
TEST(ConstantsTest, TestConstantsP16) {
    constexpr int P = 16;
    // Maximum allowable relative error for P=16 (about 2^-16)
    constexpr double tolerance = static_cast<double>(math::fp::Fixed64<P>::Epsilon());

    // Test Pi and variants
    TestConstant<P>("Pi", M_PI, math::fp::Fixed64<P>::Pi(), tolerance);
    TestConstant<P>("TwoPi", 2.0 * M_PI, math::fp::Fixed64<P>::TwoPi(), tolerance);
    TestConstant<P>("HalfPi", M_PI_2, math::fp::Fixed64<P>::HalfPi(), tolerance);
    TestConstant<P>("QuarterPi", M_PI_4, math::fp::Fixed64<P>::QuarterPi(), tolerance);
    TestConstant<P>("InvPi", 1.0 / M_PI, math::fp::Fixed64<P>::InvPi(), tolerance);

    // Test E and logarithmic constants
    TestConstant<P>("E", M_E, math::fp::Fixed64<P>::E(), tolerance);
    TestConstant<P>("Ln2", M_LN2, math::fp::Fixed64<P>::Ln2(), tolerance);
    TestConstant<P>("Log2E", M_LOG2E, math::fp::Fixed64<P>::Log2E(), tolerance);
    TestConstant<P>("Log10Of2", M_LOG10_2, math::fp::Fixed64<P>::Log10Of2(), tolerance);

    // Test angle conversion constants
    TestConstant<P>("Deg2Rad", M_PI / 180.0, math::fp::Fixed64<P>::Deg2Rad(), tolerance);
    TestConstant<P>("Rad2Deg", 180.0 / M_PI, math::fp::Fixed64<P>::Rad2Deg(), tolerance);
}

// Test all mathematical constants for P=32
TEST(ConstantsTest, TestConstantsP32) {
    constexpr int P = 32;
    // Maximum allowable relative error for P=32 (about 2^-32)
    constexpr double tolerance = static_cast<double>(math::fp::Fixed64<P>::Epsilon());

    // Test Pi and variants
    TestConstant<P>("Pi", M_PI, math::fp::Fixed64<P>::Pi(), tolerance);
    TestConstant<P>("TwoPi", 2.0 * M_PI, math::fp::Fixed64<P>::TwoPi(), tolerance);
    TestConstant<P>("HalfPi", M_PI_2, math::fp::Fixed64<P>::HalfPi(), tolerance);
    TestConstant<P>("QuarterPi", M_PI_4, math::fp::Fixed64<P>::QuarterPi(), tolerance);
    TestConstant<P>("InvPi", 1.0 / M_PI, math::fp::Fixed64<P>::InvPi(), tolerance);

    // Test E and logarithmic constants
    TestConstant<P>("E", M_E, math::fp::Fixed64<P>::E(), tolerance);
    TestConstant<P>("Ln2", M_LN2, math::fp::Fixed64<P>::Ln2(), tolerance);
    TestConstant<P>("Log2E", M_LOG2E, math::fp::Fixed64<P>::Log2E(), tolerance);
    TestConstant<P>("Log10Of2", M_LOG10_2, math::fp::Fixed64<P>::Log10Of2(), tolerance);

    // Test angle conversion constants
    TestConstant<P>("Deg2Rad", M_PI / 180.0, math::fp::Fixed64<P>::Deg2Rad(), tolerance);
    TestConstant<P>("Rad2Deg", 180.0 / M_PI, math::fp::Fixed64<P>::Rad2Deg(), tolerance);
}

// Test all mathematical constants for P=40 (maximum precision)
TEST(ConstantsTest, TestConstantsP40) {
    constexpr int P = 40;
    // Maximum allowable relative error for P=40 (about 2^-40)
    constexpr double tolerance = static_cast<double>(math::fp::Fixed64<P>::Epsilon());

    // Test Pi and variants
    TestConstant<P>("Pi", M_PI, math::fp::Fixed64<P>::Pi(), tolerance);
    TestConstant<P>("TwoPi", 2.0 * M_PI, math::fp::Fixed64<P>::TwoPi(), tolerance);
    TestConstant<P>("HalfPi", M_PI_2, math::fp::Fixed64<P>::HalfPi(), tolerance);
    TestConstant<P>("QuarterPi", M_PI_4, math::fp::Fixed64<P>::QuarterPi(), tolerance);
    TestConstant<P>("InvPi", 1.0 / M_PI, math::fp::Fixed64<P>::InvPi(), tolerance);

    // Test E and logarithmic constants
    TestConstant<P>("E", M_E, math::fp::Fixed64<P>::E(), tolerance);
    TestConstant<P>("Ln2", M_LN2, math::fp::Fixed64<P>::Ln2(), tolerance);
    TestConstant<P>("Log2E", M_LOG2E, math::fp::Fixed64<P>::Log2E(), tolerance);
    TestConstant<P>("Log10Of2", M_LOG10_2, math::fp::Fixed64<P>::Log10Of2(), tolerance);

    // Test angle conversion constants
    TestConstant<P>("Deg2Rad", M_PI / 180.0, math::fp::Fixed64<P>::Deg2Rad(), tolerance);
    TestConstant<P>("Rad2Deg", 180.0 / M_PI, math::fp::Fixed64<P>::Rad2Deg(), tolerance);
}

// Direct raw bits verification test
TEST(ConstantsTest, RawBitsVerification) {
    // Verify that Pi with P=32 has the expected bit pattern
    math::fp::Fixed64<32> pi32 = math::fp::Fixed64<32>::Pi();

    // Pi is approximately 3.14159... which as a fixed point with P=32 should be:
    // 3.14159... * 2^32 ≈ 13493037705
    EXPECT_EQ(pi32.value(), 13493037705LL);

    // Verify TwoPi with P=16
    math::fp::Fixed64<16> two_pi16 = math::fp::Fixed64<16>::TwoPi();

    // 2*Pi is approximately 6.28318... which as a fixed point with P=16 should be:
    // 6.28318... * 2^16 ≈ 411775
    EXPECT_EQ(two_pi16.value(), 411775LL);

    // Verify E with P=32
    math::fp::Fixed64<32> e32 = math::fp::Fixed64<32>::E();

    // e is approximately 2.71828... which as a fixed point with P=32 should be:
    // 2.71828... * 2^32 ≈ 11674931726
    EXPECT_EQ(e32.value(), 11674931555LL);
}