#pragma once

#include <cmath>  // For std::sqrt
#include "Fixed64.h"
#include "Fixed64Math.h"
#include "benchmark_utils.h"

extern "C" {
#include "softfloat.h"
}

using namespace std;

namespace benchmark {

// Test data structures for basic operations
struct TestData {
    // For fixed-point
    std::vector<math::fp::Fixed64<32>> fixed_values;
    // For SoftFloat
    std::vector<float64_t> sf_values;
    // For float standard operations
    std::vector<float> float_values;
    // For double standard operations
    std::vector<double> double_values;
    // Additional indices for random access patterns
    std::vector<int> indices;
};

struct MultiplyDivideTestData {
    // For fixed-point
    std::vector<std::pair<math::fp::Fixed64<32>, math::fp::Fixed64<32>>> fixed_pairs;
    // For SoftFloat
    std::vector<std::pair<float64_t, float64_t>> sf_pairs;
    // For float standard operations
    std::vector<std::pair<float, float>> float_pairs;
    // For double standard operations
    std::vector<std::pair<double, double>> double_pairs;
};

struct SqrtTestData {
    // For fixed-point
    std::vector<math::fp::Fixed64<32>> fixed_values;
    // For SoftFloat
    std::vector<float64_t> sf_values;
    // For float standard operations
    std::vector<float> float_values;
    // For double standard operations
    std::vector<double> double_values;
};

// Generate test data
TestData generateTestData(int count);
MultiplyDivideTestData generateMulDivTestData(int count);
SqrtTestData generateSqrtTestData(int count);

// Run the basic operations benchmark suite
vector<BenchmarkResult> runBasicOperationsBenchmark(int iterations);

}  // namespace benchmark