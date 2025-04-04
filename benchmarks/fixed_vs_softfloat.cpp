#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

// Include SoftFloat headers
extern "C" {
#include "softfloat.h"
}

// Include Fixed64 headers
#include "Fixed64Math.h"

using namespace std;
using namespace std::chrono;

// Benchmark framework - modified to prevent aggressive optimization and ensure fair comparison
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

// Create complex workload that's harder to optimize away
struct TestData {
    // For fixed-point
    std::vector<math::fp::Fixed64<32>> fixed_values;
    // For SoftFloat
    std::vector<float32_t> sf_values;
    // Common random pattern for both
    std::vector<int> indices;
};

// Generate test data with consistent patterns for fair comparison
TestData generateTestData(int count) {
    TestData data;
    data.fixed_values.reserve(count);
    data.sf_values.reserve(count);
    data.indices.reserve(count);

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dist(-100.0, 100.0);  // More reasonable range
    uniform_int_distribution<> idx_dist(0, count - 1);

    // Generate values with the same pattern for both implementations
    for (int i = 0; i < count; i++) {
        double val = dist(gen);

        // Create fixed-point value
        data.fixed_values.emplace_back(val);

        // Create equivalent SoftFloat value
        union {
            float f;
            uint32_t u;
        } conv;

        conv.f = static_cast<float>(val);
        data.sf_values.push_back(float32_t{conv.u});

        // Random index for pseudo-random access pattern
        data.indices.push_back(idx_dist(gen));
    }

    return data;
}

// Verify that both implementations produce equivalent results
void verifyImplementations(const TestData& data) {
    std::cout << "Verifying implementations with sample operations..." << std::endl;

    // Sample a few random elements - fix narrowing conversion with static_cast
    vector<int> samples = {0,
                           static_cast<int>(data.fixed_values.size() / 3),
                           static_cast<int>(data.fixed_values.size() / 2),
                           static_cast<int>(data.fixed_values.size() - 1)};

    for (int i : samples) {
        int j = (i + 1) % data.fixed_values.size();

        // Use proper conversion method instead of toFloat()
        float fixed_add = static_cast<float>(data.fixed_values[i] + data.fixed_values[j]);

        float32_t sf_add = f32_add(data.sf_values[i], data.sf_values[j]);

        union {
            uint32_t u;
            float f;
        } conv;

        conv.u = sf_add.v;
        float sf_add_f = conv.f;

        cout << "Sample [" << i << "," << j << "] - Fixed: " << fixed_add
             << ", SoftFloat: " << sf_add_f << ", Diff: " << abs(fixed_add - sf_add_f) << endl;
    }

    std::cout << "Verification complete." << std::endl << std::endl;
}

int main() {
    const int ITERATIONS = 1000000;

    cout << "Generating test data..." << endl;
    TestData data = generateTestData(ITERATIONS);

    cout << "Running benchmarks with iterations: " << ITERATIONS << endl;
    cout << "------------------------------------------------------------" << endl;
    cout << left << setw(20) << "Operation" << setw(15) << "Fixed64 (ms)" << setw(15)
         << "SoftFloat (ms)" << setw(15) << "Speedup" << endl;
    cout << "------------------------------------------------------------" << endl;

    // Verify implementations first
    verifyImplementations(data);

    // Addition benchmark - avoid unnecessary conversions
    double fixedAddTime = runBenchmark(
        "Fixed Addition",
        [&](int n) -> double {
            // Use integers instead of floats to avoid conversion
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                int i = k % data.indices.size();
                int j = data.indices[i];
                auto result = data.fixed_values[i] + data.fixed_values[j];
                sum += result.value();  // Use value() instead of toFloat()
            }
            return static_cast<double>(sum);  // Only convert once at the end
        },
        ITERATIONS);

    double softAddTime = runBenchmark(
        "SoftFloat Addition",
        [&](int n) -> double {
            uint32_t sum = 0;  // Use integer accumulation
            for (int k = 0; k < n; k++) {
                int i = k % data.indices.size();
                int j = data.indices[i];
                auto result = f32_add(data.sf_values[i], data.sf_values[j]);
                sum += result.v;  // Directly use the bit pattern
            }
            return static_cast<double>(sum);  // Only convert once
        },
        ITERATIONS);

    double speedup = (fixedAddTime > 0) ? (softAddTime / fixedAddTime) : 999.999;
    cout << left << setw(20) << "Addition" << setw(15) << fixed << setprecision(3) << fixedAddTime
         << setw(15) << softAddTime << setw(15) << speedup << endl;

    // Subtraction benchmark
    double fixedSubTime = runBenchmark(
        "Fixed Subtraction",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                int i = k % data.indices.size();
                int j = data.indices[i];
                auto result = data.fixed_values[i] - data.fixed_values[j];
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    double softSubTime = runBenchmark(
        "SoftFloat Subtraction",
        [&](int n) -> double {
            uint32_t sum = 0;
            for (int k = 0; k < n; k++) {
                int i = k % data.indices.size();
                int j = data.indices[i];
                auto result = f32_sub(data.sf_values[i], data.sf_values[j]);
                sum += result.v;
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    speedup = (fixedSubTime > 0) ? (softSubTime / fixedSubTime) : 999.999;
    cout << left << setw(20) << "Subtraction" << setw(15) << fixed << setprecision(3)
         << fixedSubTime << setw(15) << softSubTime << setw(15) << speedup << endl;

    // Multiplication benchmark
    double fixedMulTime = runBenchmark(
        "Fixed Multiplication",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                int i = k % data.indices.size();
                int j = data.indices[i];
                auto result = data.fixed_values[i] * data.fixed_values[j];
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    double softMulTime = runBenchmark(
        "SoftFloat Multiplication",
        [&](int n) -> double {
            uint32_t sum = 0;
            for (int k = 0; k < n; k++) {
                int i = k % data.indices.size();
                int j = data.indices[i];
                auto result = f32_mul(data.sf_values[i], data.sf_values[j]);
                sum += result.v;
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    speedup = (fixedMulTime > 0) ? (softMulTime / fixedMulTime) : 999.999;
    cout << left << setw(20) << "Multiplication" << setw(15) << fixed << setprecision(3)
         << fixedMulTime << setw(15) << softMulTime << setw(15) << speedup << endl;

    // Division benchmark
    double fixedDivTime = runBenchmark(
        "Fixed Division",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                int i = k % data.indices.size();
                int j = data.indices[i];
                // Skip division by zero or very small values
                if (data.fixed_values[j].value() != 0) {
                    auto result = data.fixed_values[i] / data.fixed_values[j];
                    sum += result.value();
                } else {
                    sum += 1;  // Add something to avoid optimization
                }
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    double softDivTime = runBenchmark(
        "SoftFloat Division",
        [&](int n) -> double {
            uint32_t sum = 0;
            for (int k = 0; k < n; k++) {
                int i = k % data.indices.size();
                int j = data.indices[i];

                // Skip division by zero (check if exponent and mantissa are all zero)
                if ((data.sf_values[j].v & 0x7FFFFFFF) != 0) {
                    auto result = f32_div(data.sf_values[i], data.sf_values[j]);
                    sum += result.v;
                } else {
                    sum += 1;  // Add something to avoid optimization
                }
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    speedup = (fixedDivTime > 0) ? (softDivTime / fixedDivTime) : 999.999;
    cout << left << setw(20) << "Division" << setw(15) << fixed << setprecision(3) << fixedDivTime
         << setw(15) << softDivTime << setw(15) << speedup << endl;

    // Add a summary section
    cout << "\nSummary:" << endl;
    cout << "------------------------------------------------------------" << endl;
    double avgSpeedup = 0.0;
    int opCount = 0;

    if (fixedAddTime > 0) {
        avgSpeedup += softAddTime / fixedAddTime;
        opCount++;
    }
    if (fixedSubTime > 0) {
        avgSpeedup += softSubTime / fixedSubTime;
        opCount++;
    }
    if (fixedMulTime > 0) {
        avgSpeedup += softMulTime / fixedMulTime;
        opCount++;
    }
    if (fixedDivTime > 0) {
        avgSpeedup += softDivTime / fixedDivTime;
        opCount++;
    }

    if (opCount > 0) {
        avgSpeedup /= opCount;
        cout << "Average speedup: " << fixed << setprecision(2) << avgSpeedup << "x" << endl;
        cout << "Fixed64 is approximately " << fixed << setprecision(2) << avgSpeedup
             << " times faster than SoftFloat" << endl;
    } else {
        cout << "Could not calculate average speedup (division by zero)" << endl;
    }

    return 0;
}