#pragma once

#include <cmath>
#include <cstdint>
#include <iostream>
#include "FixedTrigLutTables.h"
#include "Primitives.h"

namespace math::fp {

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays
//,cppcoreguidelines-pro-bounds-pointer-arithmetic)

class FixedTrigLut {
 public:
    // Precision definitions
    static constexpr int TABLE_PRECISION = 12;  // Lookup table precision (4096 = 2^12)
    static constexpr int FRACTION_BITS = 32;    // Q31.32 format precision
    static constexpr int TABLE_SIZE = 4096;     // Lookup table size
    static constexpr int INDEX_SHIFT = FRACTION_BITS - TABLE_PRECISION;  // 32 - 12 = 20
    static constexpr int64_t INDEX_MASK = (1LL << INDEX_SHIFT) - 1;
    static constexpr int64_t HALF_FRACTION_BITS = FRACTION_BITS / 2;

    // Q31.32 format constants
    static constexpr int64_t ONE = 1LL << FRACTION_BITS;
    static constexpr int64_t PI = 13493037705LL;        // π * 2^32
    static constexpr int64_t INV_TWO_PI = 683565275LL;  // (1/2π) * 2^32
    static constexpr int64_t HALF_PI = PI >> 1;
    static constexpr int64_t TWO_PI = PI << 1;
    static constexpr int64_t ZERO = 0;

    // Lookup table array declarations
    // To properly define arrays in a header file, use 'inline' keyword (C++17 and later)
    static constexpr const int64_t* SinLut = g_FixedTrig_SinLut;
    static constexpr const int64_t* AcosLut = g_FixedTrig_AcosLut;

    /**
     * @brief Calculate sine of angle
     * @param x Angle in fixed-point radians
     * @return Fixed-point sine value
     */
    static int64_t Sin(int64_t x) noexcept {
        // 1. Normalize angle to [0, 2π)
        x %= TWO_PI;
        x = Primitives::Fixed64Mul(x, INV_TWO_PI, FRACTION_BITS);

        int sign = 1;
        if (x < 0) {
            x = -x;
            sign = -1;
        }

        // 2. Calculate lookup table index (12-bit precision)
        int index = (int)(x >> INDEX_SHIFT);
        int64_t fraction = (x & ((1LL << INDEX_SHIFT) - 1)) << TABLE_PRECISION;

        // 3. Linear interpolation (32-bit precision calculation)
        int64_t a = SinLut[index];
        int64_t b = SinLut[index + 1];

        return (a + (((b - a) * fraction) >> FRACTION_BITS)) * sign;
    }

    /**
     * @brief Calculate cosine of angle
     * @param angle Angle in fixed-point radians
     * @return Fixed-point cosine value
     */
    static int64_t Cos(int64_t angle) noexcept {
        // cos(x) = sin(x + π/2)
        return Sin(angle + HALF_PI);
    }

    /**
     * @brief Calculate arc-cosine
     * @param x Input in fixed-point format, must be in range [-1, 1]
     * @return Fixed-point arc-cosine value in radians
     */
    static int64_t Acos(int64_t x) noexcept {
        // Define interval boundary constants
        constexpr int64_t THRESHOLD_0_8 = ONE * 4LL / 5LL;         // 0.8
        constexpr int64_t THRESHOLD_0_95 = ONE * 19LL / 20LL;      // 0.95
        constexpr int64_t THRESHOLD_0_99 = ONE * 99LL / 100LL;     // 0.99
        constexpr int64_t THRESHOLD_0_999 = ONE * 999LL / 1000LL;  // 0.999
        constexpr int64_t THRESHOLD_SMALL = ONE - (ONE >> 16);     // 0.999984741211

        // Boundary check: ensure input is within [-ONE, ONE] range
        if (x >= ONE)
            return 0;
        if (x <= -ONE)
            return PI;

        bool is_negative = x < 0;
        x = is_negative ? -x : x;

        // Handle extremely small angles: x > 0.999984741211, use sqrt(2(1-x)) approximation
        if (x > THRESHOLD_SMALL) {
            int64_t epsilon = ONE - x;
            int64_t sqrt_input = (epsilon << 1);
            return Sqrt(sqrt_input);
        }

        int64_t result;
        // Region 1: [0, 0.8], use 512-point uniform interpolation
        if (x < THRESHOLD_0_8) {
            int index = (x << 9) / THRESHOLD_0_8;  // x * 512 / (0.8 * ONE)
            index = std::min(index, 511);

            // Calculate interpolation
            int64_t x0 = (index * THRESHOLD_0_8) >> 9;  // index * 0.8 * ONE / 512
            int64_t dx = x - x0;
            constexpr int64_t delta = THRESHOLD_0_8 >> 9;  // 0.8 * ONE / 512
            result = AcosLut[index] + ((AcosLut[index + 1] - AcosLut[index]) * dx) / delta;
        }
        // Region 2: [0.8, 0.95], use 64-segment cubic spline interpolation
        else if (x < THRESHOLD_0_95) {
            int seg = ((x - (THRESHOLD_0_8)) * 64LL) / (ONE * 3LL / 20LL);  // (x - 0.8) / (0.15/64)
            seg = std::min(seg, 63);

            int base_idx = 513 + seg * 3;
            int64_t x0 = AcosLut[base_idx];
            int64_t y0 = AcosLut[base_idx + 1];
            int64_t dydx = AcosLut[base_idx + 2];

            int64_t dx = x - x0;
            result = y0 + ((dydx * dx) >> FRACTION_BITS);
        }
        // Region 3: [0.95, 0.99], use 512-point linear interpolation
        else if (x < THRESHOLD_0_99) {
            int base_idx = 513 + 195;              // 512 + 1 + 65*3
            int64_t rel_x = x - (THRESHOLD_0_95);  // x - 0.95
            int64_t scale = (ONE * 4LL / 100LL);   // 0.04 * ONE

            int index = (rel_x * 512LL) / scale;
            index = std::min(index, 511);

            int idx = base_idx + index;
            int64_t x1 = (THRESHOLD_0_95) + (scale * index) / 512LL;
            int64_t x2 = (THRESHOLD_0_95) + (scale * (index + 1)) / 512LL;

            int64_t alpha = ((x - x1) << FRACTION_BITS) / (x2 - x1);
            result = ((AcosLut[idx] * (ONE - alpha)) + (AcosLut[idx + 1] * alpha)) >> FRACTION_BITS;
        }
        // Region 4: [0.99, 0.999], use 512-point linear interpolation
        else if (x < THRESHOLD_0_999) {
            int base_idx = 513 + 195 + 513;
            int64_t rel_x = x - (THRESHOLD_0_99);  // x - 0.99
            int64_t scale = (ONE * 9LL / 1000LL);  // 0.009 * ONE

            int index = (rel_x * 512LL) / scale;
            index = std::min(index, 511);

            int idx = base_idx + index;
            int64_t x1 = (THRESHOLD_0_99) + (scale * index) / 512LL;
            int64_t x2 = (THRESHOLD_0_99) + (scale * (index + 1)) / 512LL;

            int64_t alpha = ((x - x1) << FRACTION_BITS) / (x2 - x1);
            result = ((AcosLut[idx] * (ONE - alpha)) + (AcosLut[idx + 1] * alpha)) >> FRACTION_BITS;
        }
        // Region 5: [0.999, 1.0), use 256-point linear interpolation
        else {
            int base_idx = 513 + 195 + 513 + 513;
            int64_t rel_x = x - (THRESHOLD_0_999);  // x - 0.999
            int64_t scale = ONE / 1000LL;           // 0.001 * ONE

            int index = (rel_x * 256LL) / scale;
            index = std::min(index, 255);

            int idx = base_idx + index;
            int64_t x1 = (THRESHOLD_0_999) + (scale * index) / 256LL;
            int64_t x2 = (THRESHOLD_0_999) + (scale * (index + 1)) / 256LL;

            int64_t alpha = ((x - x1) << FRACTION_BITS) / (x2 - x1);
            result = ((AcosLut[idx] * (ONE - alpha)) + (AcosLut[idx + 1] * alpha)) >> FRACTION_BITS;
        }

        return is_negative ? (PI - result) : result;
    }

    /**
     * @brief Calculate arc-sine
     * @param x Input in fixed-point format, must be in range [-1, 1]
     * @return Fixed-point arc-sine value in radians
     */
    static int64_t Asin(int64_t x) noexcept {
        return HALF_PI - Acos(x);
    }

 private:
    /**
     * @brief Calculate square root of fixed-point value
     * @param x Input value
     * @return Square root result
     */
    static int64_t Sqrt(int64_t x) noexcept {
        return Primitives::Fixed64SqrtFast(x, FRACTION_BITS);
    }
};

// NOLINTEND
}  // namespace math::fp
