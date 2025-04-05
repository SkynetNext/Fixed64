#pragma once

#include <vector>
#include "Fixed64.h"
#include "Fixed64Math.h"
#include "benchmark_utils.h"


namespace benchmark {

// Generate test data for different function types
struct MathTestData {
    // For general use (values between -1 and 1)
    std::vector<math::fp::Fixed64<32>> unit_values;

    // For positive values only (> 0)
    std::vector<math::fp::Fixed64<32>> positive_values;

    // For angles (0 to 2π)
    std::vector<math::fp::Fixed64<32>> angle_values;

    // For Atan2 (pairs of values)
    std::vector<std::pair<math::fp::Fixed64<32>, math::fp::Fixed64<32>>> atan2_pairs;
};

MathTestData generateMathTestData(int count);

// Run the advanced math functions benchmark suite
std::vector<BenchmarkResult> runAdvancedMathBenchmark(int iterations);

}  // namespace benchmark