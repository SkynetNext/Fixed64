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
    // For SoftFloat - now using float64_t instead of float32_t
    std::vector<float64_t> sf_values;
    // Common random pattern for both
    std::vector<int> indices;
};

// Generate test data with consistent patterns for fair comparison
TestData generateTestData(int count) {
    TestData data;
    // Add +1 to prevent out-of-bounds access when using k+1 index
    int allocSize = count + 1;
    data.fixed_values.reserve(allocSize);
    data.sf_values.reserve(allocSize);
    data.indices.reserve(allocSize);

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dist(-100.0, 100.0);  // More reasonable range
    uniform_int_distribution<> idx_dist(0, allocSize - 1);

    // Generate values with the same pattern for both implementations
    for (int i = 0; i < allocSize; i++) {
        double val = dist(gen);

        // Create fixed-point value
        data.fixed_values.emplace_back(val);

        // Create equivalent SoftFloat value - now using float64_t
        union {
            double d;
            uint64_t u;
        } conv;

        conv.d = val;
        data.sf_values.push_back(float64_t{conv.u});

        // Random index for pseudo-random access pattern
        data.indices.push_back(idx_dist(gen));
    }

    return data;
}

// New structure for multiplication and division tests
struct MultiplyDivideTestData {
    // For fixed-point
    std::vector<std::pair<math::fp::Fixed64<32>, math::fp::Fixed64<32>>> fixed_pairs;
    // For SoftFloat - now using float64_t
    std::vector<std::pair<float64_t, float64_t>> sf_pairs;
};

// Generate paired test data specifically for multiplication and division
MultiplyDivideTestData generateMulDivTestData(int count) {
    MultiplyDivideTestData data;
    // Add +1 to prevent any potential out-of-bounds access
    int allocSize = count + 1;
    data.fixed_pairs.reserve(allocSize);
    data.sf_pairs.reserve(allocSize);

    random_device rd;
    mt19937 gen(rd());

    // Generate values far from zero for better multiplication testing
    uniform_real_distribution<> dist_pos(1.0, 1000.0);    // Positive range
    uniform_real_distribution<> dist_neg(-1000.0, -1.0);  // Negative range
    uniform_int_distribution<> sign_dist(0, 1);           // To choose positive or negative

    // Generate pairs of values
    for (int i = 0; i < allocSize; i++) {
        // Generate a and b with balanced positive/negative values
        double a = sign_dist(gen) ? dist_pos(gen) : dist_neg(gen);
        double b = sign_dist(gen) ? dist_pos(gen) : dist_neg(gen);

        // Create fixed-point pair
        math::fp::Fixed64<32> fixed_a(a);
        math::fp::Fixed64<32> fixed_b(b);
        data.fixed_pairs.emplace_back(fixed_a, fixed_b);

        // Create equivalent SoftFloat pair
        union {
            double d;
            uint64_t u;
        } conv_a, conv_b;

        conv_a.d = a;
        conv_b.d = b;

        data.sf_pairs.emplace_back(float64_t{conv_a.u}, float64_t{conv_b.u});
    }

    return data;
}

// For square root test data
struct SqrtTestData {
    // For fixed-point
    std::vector<math::fp::Fixed64<32>> fixed_values;
    // For SoftFloat
    std::vector<float64_t> sf_values;
};

// Generate special test data for square root operations (positive values only)
SqrtTestData generateSqrtTestData(int count) {
    SqrtTestData data;
    // Add +1 to prevent any potential out-of-bounds access
    int allocSize = count + 1;
    data.fixed_values.reserve(allocSize);
    data.sf_values.reserve(allocSize);

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dist(0.01, 1000.0);  // Positive values only

    // Generate positive values suitable for square root
    for (int i = 0; i < allocSize; i++) {
        double val = dist(gen);

        // Create fixed-point value
        data.fixed_values.emplace_back(val);

        // Create equivalent SoftFloat value
        union {
            double d;
            uint64_t u;
        } conv;

        conv.d = val;
        data.sf_values.push_back(float64_t{conv.u});
    }

    return data;
}

// Verify that both implementations produce equivalent results for all operations
void verifyImplementations(const TestData& data,
                           const MultiplyDivideTestData& mulDivData,
                           const SqrtTestData& sqrtData) {
    std::cout << "Verifying implementations with sample operations..." << std::endl;

    // Sample a few random elements for addition/subtraction
    vector<int> samples = {0,
                           static_cast<int>(data.fixed_values.size() / 3),
                           static_cast<int>(data.fixed_values.size() / 2),
                           static_cast<int>(data.fixed_values.size() - 1)};

    std::cout << "\n=== Addition/Subtraction Verification ===" << std::endl;
    for (int i : samples) {
        int j = (i + 1) % data.fixed_values.size();

        // Addition verification
        double fixed_add = static_cast<double>(data.fixed_values[i] + data.fixed_values[j]);

        float64_t sf_add = f64_add(data.sf_values[i], data.sf_values[j]);

        union {
            uint64_t u;
            double d;
        } add_conv;

        add_conv.u = sf_add.v;
        double sf_add_d = add_conv.d;

        cout << "Addition [" << i << "," << j << "] - Fixed: " << fixed_add
             << ", SoftFloat: " << sf_add_d << ", Diff: " << abs(fixed_add - sf_add_d) << endl;

        // Subtraction verification
        double fixed_sub = static_cast<double>(data.fixed_values[i] - data.fixed_values[j]);

        float64_t sf_sub = f64_sub(data.sf_values[i], data.sf_values[j]);

        union {
            uint64_t u;
            double d;
        } sub_conv;

        sub_conv.u = sf_sub.v;
        double sf_sub_d = sub_conv.d;

        cout << "Subtraction [" << i << "," << j << "] - Fixed: " << fixed_sub
             << ", SoftFloat: " << sf_sub_d << ", Diff: " << abs(fixed_sub - sf_sub_d) << endl;
    }

    // Sample multiplication and division from the specialized test data
    std::cout << "\n=== Multiplication/Division Verification ===" << std::endl;
    samples = {0,
               static_cast<int>(mulDivData.fixed_pairs.size() / 3),
               static_cast<int>(mulDivData.fixed_pairs.size() / 2),
               static_cast<int>(mulDivData.fixed_pairs.size() - 1)};

    for (int i : samples) {
        // Get the test values
        auto fixed_a = mulDivData.fixed_pairs[i].first;
        auto fixed_b = mulDivData.fixed_pairs[i].second;
        auto sf_a = mulDivData.sf_pairs[i].first;
        auto sf_b = mulDivData.sf_pairs[i].second;

        // Convert to double for display
        union {
            uint64_t u;
            double d;
        } a_conv, b_conv;

        a_conv.u = sf_a.v;
        b_conv.u = sf_b.v;

        // Display test values
        cout << "Test values [" << i << "]: a=" << static_cast<double>(fixed_a) << " (" << a_conv.d
             << "), b=" << static_cast<double>(fixed_b) << " (" << b_conv.d << ")" << endl;

        // Multiplication verification
        double fixed_mul = static_cast<double>(fixed_a * fixed_b);

        float64_t sf_mul = f64_mul(sf_a, sf_b);

        union {
            uint64_t u;
            double d;
        } mul_conv;

        mul_conv.u = sf_mul.v;
        double sf_mul_d = mul_conv.d;

        cout << "Multiplication - Fixed: " << fixed_mul << ", SoftFloat: " << sf_mul_d
             << ", Diff: " << abs(fixed_mul - sf_mul_d)
             << ", Rel Diff: " << (abs(fixed_mul - sf_mul_d) / (abs(sf_mul_d) + 1e-10)) << endl;

        // Division verification
        double fixed_div = static_cast<double>(fixed_a / fixed_b);

        float64_t sf_div = f64_div(sf_a, sf_b);

        union {
            uint64_t u;
            double d;
        } div_conv;

        div_conv.u = sf_div.v;
        double sf_div_d = div_conv.d;

        cout << "Division - Fixed: " << fixed_div << ", SoftFloat: " << sf_div_d
             << ", Diff: " << abs(fixed_div - sf_div_d)
             << ", Rel Diff: " << (abs(fixed_div - sf_div_d) / (abs(sf_div_d) + 1e-10)) << endl;
    }

    // Add square root verification
    std::cout << "\n=== Square Root Verification ===" << std::endl;
    vector<int> sqrtSamples = {0,
                               static_cast<int>(sqrtData.fixed_values.size() / 3),
                               static_cast<int>(sqrtData.fixed_values.size() / 2),
                               static_cast<int>(sqrtData.fixed_values.size() - 1)};

    for (int i : sqrtSamples) {
        double fixed_val = static_cast<double>(sqrtData.fixed_values[i]);

        union {
            uint64_t u;
            double d;
        } sf_conv;

        sf_conv.u = sqrtData.sf_values[i].v;
        double sf_val = sf_conv.d;

        // Calculate square root with fixed-point - using Fixed64Math::Sqrt
        double fixed_sqrt =
            static_cast<double>(math::fp::Fixed64Math::Sqrt(sqrtData.fixed_values[i]));

        // Calculate square root with SoftFloat
        float64_t sf_sqrt = f64_sqrt(sqrtData.sf_values[i]);

        union {
            uint64_t u;
            double d;
        } sqrt_conv;

        sqrt_conv.u = sf_sqrt.v;
        double sf_sqrt_d = sqrt_conv.d;

        cout << "Value [" << i << "]: " << fixed_val << " (" << sf_val << ")" << endl;
        cout << "Square Root - Fixed: " << fixed_sqrt << ", SoftFloat: " << sf_sqrt_d
             << ", Diff: " << abs(fixed_sqrt - sf_sqrt_d)
             << ", Rel Diff: " << (abs(fixed_sqrt - sf_sqrt_d) / (abs(sf_sqrt_d) + 1e-10)) << endl;
    }

    std::cout << "\nVerification complete." << std::endl << std::endl;
}

int main() {
    std::cout << "==== Build Information ====" << std::endl;
    std::cout << "Size of void*: " << sizeof(void*) << " bytes" << std::endl;
    std::cout << "Size of size_t: " << sizeof(size_t) << " bytes" << std::endl;
    std::cout << "Size of int64_t: " << sizeof(int64_t) << " bytes" << std::endl;

    // Increased iterations for more accurate timing
    const int ITERATIONS = 10000000;  // 10M iterations instead of 1M

    cout << "Generating test data..." << endl;
    TestData data = generateTestData(ITERATIONS);

    // Generate special test data for multiply/divide operations
    MultiplyDivideTestData mulDivData = generateMulDivTestData(ITERATIONS);

    // Generate special test data for square root operations
    SqrtTestData sqrtData = generateSqrtTestData(ITERATIONS);

    cout << "Running benchmarks with iterations: " << ITERATIONS << endl;
    cout << "------------------------------------------------------------" << endl;
    cout << left << setw(20) << "Operation" << setw(15) << "Fixed64 (ms)" << setw(15)
         << "SoftFloat (ms)" << setw(15) << "Speedup" << endl;
    cout << "------------------------------------------------------------" << endl;

    // Verify implementations first - pass all data structures
    verifyImplementations(data, mulDivData, sqrtData);

    // Addition benchmark - direct indexing without modulo
    double fixedAddTime = runBenchmark(
        "Fixed Addition",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                // Direct array access without modulo
                auto result = data.fixed_values[k] + data.fixed_values[(k + 1)];
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    double softAddTime = runBenchmark(
        "SoftFloat Addition",
        [&](int n) -> double {
            uint64_t sum = 0;
            for (int k = 0; k < n; k++) {
                // Direct array access without modulo
                auto result = f64_add(data.sf_values[k], data.sf_values[(k + 1)]);
                sum += result.v;
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    double speedup = (fixedAddTime > 0) ? (softAddTime / fixedAddTime) : 999.999;
    cout << left << setw(20) << "Addition" << setw(15) << fixed << setprecision(3) << fixedAddTime
         << setw(15) << softAddTime << setw(15) << speedup << endl;

    // Subtraction benchmark - direct indexing without modulo
    double fixedSubTime = runBenchmark(
        "Fixed Subtraction",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                // Direct array access without modulo
                auto result = data.fixed_values[k] - data.fixed_values[(k + 1)];
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    double softSubTime = runBenchmark(
        "SoftFloat Subtraction",
        [&](int n) -> double {
            uint64_t sum = 0;
            for (int k = 0; k < n; k++) {
                // Direct array access without modulo
                auto result = f64_sub(data.sf_values[k], data.sf_values[(k + 1)]);
                sum += result.v;
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    speedup = (fixedSubTime > 0) ? (softSubTime / fixedSubTime) : 999.999;
    cout << left << setw(20) << "Subtraction" << setw(15) << fixed << setprecision(3)
         << fixedSubTime << setw(15) << softSubTime << setw(15) << speedup << endl;

    // Multiplication benchmark - direct indexing without modulo
    double fixedMulTime = runBenchmark(
        "Fixed Multiplication",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                // Direct array access without modulo
                auto result = mulDivData.fixed_pairs[k].first * mulDivData.fixed_pairs[k].second;
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    double softMulTime = runBenchmark(
        "SoftFloat Multiplication",
        [&](int n) -> double {
            uint64_t sum = 0;
            for (int k = 0; k < n; k++) {
                // Direct array access without modulo
                auto result = f64_mul(mulDivData.sf_pairs[k].first, mulDivData.sf_pairs[k].second);
                sum += result.v;
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    speedup = (fixedMulTime > 0) ? (softMulTime / fixedMulTime) : 999.999;
    cout << left << setw(20) << "Multiplication" << setw(15) << fixed << setprecision(3)
         << fixedMulTime << setw(15) << softMulTime << setw(15) << speedup << endl;

    // Division benchmark - direct indexing without modulo
    double fixedDivTime = runBenchmark(
        "Fixed Division",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                // Direct array access without modulo
                auto result = mulDivData.fixed_pairs[k].first / mulDivData.fixed_pairs[k].second;
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    double softDivTime = runBenchmark(
        "SoftFloat Division",
        [&](int n) -> double {
            uint64_t sum = 0;
            for (int k = 0; k < n; k++) {
                // Direct array access without modulo
                auto result = f64_div(mulDivData.sf_pairs[k].first, mulDivData.sf_pairs[k].second);
                sum += result.v;
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    speedup = (fixedDivTime > 0) ? (softDivTime / fixedDivTime) : 999.999;
    cout << left << setw(20) << "Division" << setw(15) << fixed << setprecision(3) << fixedDivTime
         << setw(15) << softDivTime << setw(15) << speedup << endl;

    // Add Square Root benchmark
    double fixedSqrtTime = runBenchmark(
        "Fixed Square Root",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = math::fp::Fixed64Math::Sqrt(sqrtData.fixed_values[k]);
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    double softSqrtTime = runBenchmark(
        "SoftFloat Square Root",
        [&](int n) -> double {
            uint64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = f64_sqrt(sqrtData.sf_values[k]);
                sum += result.v;
            }
            return static_cast<double>(sum);
        },
        ITERATIONS);

    speedup = (fixedSqrtTime > 0) ? (softSqrtTime / fixedSqrtTime) : 999.999;
    cout << left << setw(20) << "Square Root" << setw(15) << fixed << setprecision(3)
         << fixedSqrtTime << setw(15) << softSqrtTime << setw(15) << speedup << endl;

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
    if (fixedSqrtTime > 0) {
        avgSpeedup += softSqrtTime / fixedSqrtTime;
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