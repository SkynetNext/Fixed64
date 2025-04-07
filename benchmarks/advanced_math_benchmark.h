#pragma once

#include <math/softfloat/soft_double.h>
#include <cmath>
#include <vector>
#include "benchmark_utils.h"
#include "fixed64.h"
#include "fixed64_math.h"

namespace benchmark {

// Generate test data for different function types
struct MathTestData {
    // Fixed-point data
    std::vector<math::fp::Fixed64<32>> unit_values;
    std::vector<math::fp::Fixed64<32>> positive_values;
    std::vector<math::fp::Fixed64<32>> angle_values;
    std::vector<std::pair<math::fp::Fixed64<32>, math::fp::Fixed64<32>>> atan2_pairs;
    std::vector<std::pair<math::fp::Fixed64<32>, math::fp::Fixed64<32>>> pow_pairs;

    // Float data (pre-converted to avoid conversion costs during benchmarking)
    std::vector<float> unit_values_float;
    std::vector<float> positive_values_float;
    std::vector<float> angle_values_float;
    std::vector<std::pair<float, float>> atan2_pairs_float;
    std::vector<std::pair<float, float>> pow_pairs_float;

    // Double data (pre-converted to avoid conversion costs during benchmarking)
    std::vector<double> unit_values_double;
    std::vector<double> positive_values_double;
    std::vector<double> angle_values_double;
    std::vector<std::pair<double, double>> atan2_pairs_double;
    std::vector<std::pair<double, double>> pow_pairs_double;

    // SoftDouble data (for softfloat library benchmarks)
    std::vector<::math::softfloat::float64_t> unit_values_softdouble;
    std::vector<::math::softfloat::float64_t> positive_values_softdouble;
    std::vector<::math::softfloat::float64_t> angle_values_softdouble;
    std::vector<std::pair<::math::softfloat::float64_t, ::math::softfloat::float64_t>>
        atan2_pairs_softdouble;
    std::vector<std::pair<::math::softfloat::float64_t, ::math::softfloat::float64_t>>
        pow_pairs_softdouble;
};

MathTestData generateMathTestData(int count);

// Run the advanced math functions benchmark suite
std::vector<BenchmarkResult> runAdvancedMathBenchmark(int iterations);

}  // namespace benchmark