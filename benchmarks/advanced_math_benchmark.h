#pragma once

#include <cmath>
#include <vector>
#include "Fixed64.h"
#include "Fixed64Math.h"
#include "benchmark_utils.h"


namespace benchmark {

// Generate test data for different function types
struct MathTestData {
    // Fixed-point data
    std::vector<math::fp::Fixed64<32>> unit_values;
    std::vector<math::fp::Fixed64<32>> positive_values;
    std::vector<math::fp::Fixed64<32>> angle_values;
    std::vector<std::pair<math::fp::Fixed64<32>, math::fp::Fixed64<32>>> atan2_pairs;

    // Float data (pre-converted to avoid conversion costs during benchmarking)
    std::vector<float> unit_values_float;
    std::vector<float> positive_values_float;
    std::vector<float> angle_values_float;
    std::vector<std::pair<float, float>> atan2_pairs_float;

    // Double data (pre-converted to avoid conversion costs during benchmarking)
    std::vector<double> unit_values_double;
    std::vector<double> positive_values_double;
    std::vector<double> angle_values_double;
    std::vector<std::pair<double, double>> atan2_pairs_double;
};

MathTestData generateMathTestData(int count);

// Run the advanced math functions benchmark suite
std::vector<BenchmarkResult> runAdvancedMathBenchmark(int iterations);

}  // namespace benchmark