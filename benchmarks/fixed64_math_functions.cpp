#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

// Include Fixed64 headers
#include "Fixed64Math.h"

using namespace std;
using namespace std::chrono;

// Benchmark framework
template <typename Func>
double runBenchmark(const string& name, Func func, int iterations) {
    // Warmup run
    func(std::min(10000, iterations));

    // Force alignment to prevent cache effects
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // Perform multiple timing runs and take the average
    const int TIMING_RUNS = 3;
    double totalTime = 0.0;

    for (int run = 0; run < TIMING_RUNS; run++) {
        auto start = high_resolution_clock::now();
        // Call the function and get a result that must be used
        auto result = func(iterations);
        auto end = high_resolution_clock::now();

        // Store the time
        double time = duration_cast<microseconds>(end - start).count() / 1000.0;
        totalTime += time;

        // Make sure the result is used to prevent optimization
        if (run == 0) {
            std::cout << "  " << name << " - Run " << run + 1 << " result: " << std::scientific
                      << std::setprecision(6) << result << std::endl;
        }
    }

    return totalTime / TIMING_RUNS;
}

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

MathTestData generateMathTestData(int count) {
    MathTestData data;
    int allocSize = count + 1;

    data.unit_values.reserve(allocSize);
    data.positive_values.reserve(allocSize);
    data.angle_values.reserve(allocSize);
    data.atan2_pairs.reserve(allocSize);

    random_device rd;
    mt19937 gen(rd());

    // Different distributions for different function needs
    uniform_real_distribution<> unit_dist(-1.0, 1.0);          // For [-1, 1] range
    uniform_real_distribution<> positive_dist(0.001, 10.0);    // For positive values
    uniform_real_distribution<> angle_dist(0.0, 6.283185307);  // For [0, 2π] range
    uniform_real_distribution<> atan2_dist(-10.0, 10.0);       // For atan2 inputs

    for (int i = 0; i < allocSize; i++) {
        // Generate values in the unit range [-1, 1]
        data.unit_values.emplace_back(unit_dist(gen));

        // Generate positive values (for log, sqrt, etc.)
        data.positive_values.emplace_back(positive_dist(gen));

        // Generate angle values (for trig functions)
        data.angle_values.emplace_back(angle_dist(gen));

        // Generate pairs for Atan2
        data.atan2_pairs.emplace_back(math::fp::Fixed64<32>(atan2_dist(gen)),
                                      math::fp::Fixed64<32>(atan2_dist(gen)));
    }

    return data;
}

int main() {
    std::cout << "==== Fixed64Math Advanced Function Benchmarks ====" << std::endl;

    const int ITERATIONS = 10000000;  // 10M iterations as requested

    cout << "Generating test data..." << endl;
    MathTestData data = generateMathTestData(ITERATIONS);

    cout << "Running benchmarks with iterations: " << ITERATIONS << endl;
    cout << "------------------------------------------------------------" << endl;
    cout << left << setw(20) << "Function" << setw(15) << "Time (ms)" << endl;
    cout << "------------------------------------------------------------" << endl;

    // Benchmark Pow2 function
    double pow2Time = runBenchmark(
        "Pow2",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = math::fp::Fixed64Math::Pow2(data.unit_values[k]);
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    cout << left << setw(20) << "Pow2" << setw(15) << fixed << setprecision(3) << pow2Time << endl;

    // Benchmark Sin function
    double sinTime = runBenchmark(
        "Sin",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = math::fp::Fixed64Math::Sin(data.angle_values[k]);
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    cout << left << setw(20) << "Sin" << setw(15) << fixed << setprecision(3) << sinTime << endl;

    // Benchmark Acos function
    double acosTime = runBenchmark(
        "Acos",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = math::fp::Fixed64Math::Acos(data.unit_values[k]);
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    cout << left << setw(20) << "Acos" << setw(15) << fixed << setprecision(3) << acosTime << endl;

    // Benchmark Exp function
    double expTime = runBenchmark(
        "Exp",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = math::fp::Fixed64Math::Exp(data.unit_values[k]);
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    cout << left << setw(20) << "Exp" << setw(15) << fixed << setprecision(3) << expTime << endl;

    // Benchmark Log function
    double logTime = runBenchmark(
        "Log",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = math::fp::Fixed64Math::Log(data.positive_values[k]);
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    cout << left << setw(20) << "Log" << setw(15) << fixed << setprecision(3) << logTime << endl;

    // Benchmark Atan2 function
    double atan2Time = runBenchmark(
        "Atan2",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = math::fp::Fixed64Math::Atan2(data.atan2_pairs[k].first,
                                                           data.atan2_pairs[k].second);
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    cout << left << setw(20) << "Atan2" << setw(15) << fixed << setprecision(3) << atan2Time
         << endl;

    // Summary
    cout << "\nSummary:" << endl;
    cout << "------------------------------------------------------------" << endl;
    cout << "Advanced math functions are typically implemented using iterative algorithms" << endl;
    cout << "or lookup tables in fixed-point, which affects their performance compared" << endl;
    cout << "to hardware floating-point implementations." << endl;

    return 0;
}