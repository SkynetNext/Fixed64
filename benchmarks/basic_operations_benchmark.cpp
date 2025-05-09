#include "basic_operations_benchmark.h"

namespace benchmark {

// Generate test data with consistent patterns for fair comparison
TestData generateTestData(int count) {
    TestData data;
    // Add +1 to prevent out-of-bounds access when using k+1 index
    int allocSize = count + 1;
    data.fixed_values.reserve(allocSize);

    data.sf_values.reserve(allocSize);

    data.int64_values.reserve(allocSize);
    data.double_values.reserve(allocSize);

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dist(-100.0, 100.0);  // More reasonable range
    uniform_int_distribution<> idx_dist(0, allocSize - 1);

    // Generate values with the same pattern for all implementations
    for (int i = 0; i < allocSize; i++) {
        double val = dist(gen);

        // Create fixed-point value
        data.fixed_values.emplace_back(val);

        // Create equivalent SoftDouble value (simpler with soft_double)
        data.sf_values.emplace_back(val);

        // Create float and double values for standard library operations
        data.double_values.push_back(val);
        data.int64_values.push_back(static_cast<int64_t>(val) + 1);
    }

    return data;
}

// Generate specialized test data for multiply/divide
MultiplyDivideTestData generateMulDivTestData(int count) {
    MultiplyDivideTestData data;
    // Add +1 to prevent out-of-bounds access
    int allocSize = count + 1;
    data.fixed_pairs.reserve(allocSize);
    data.sf_pairs.reserve(allocSize);
    data.int64_pairs.reserve(allocSize);
    data.double_pairs.reserve(allocSize);

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dist(-1000.0, 1000.0);

    for (int i = 0; i < allocSize; i++) {
        double a = dist(gen);
        double b = dist(gen);

        // Avoid division by zero
        if (std::abs(b) < 0.001) {
            b = (b >= 0) ? 1.0 : -1.0;
        }

        // Create fixed-point pair
        data.fixed_pairs.emplace_back(math::fp::Fixed64<32>(a), math::fp::Fixed64<32>(b));

        // Create equivalent SoftDouble pair
        data.sf_pairs.emplace_back(::math::softfloat::float64_t(a),
                                   ::math::softfloat::float64_t(b));

        // Create int64_t pairs with rounding
        int64_t a_int = std::llround(a);
        int64_t b_int = std::llround(b);

        // If rounded to zero, move away from zero in the original value's direction
        if (a_int == 0) {
            a_int = (a >= 0) ? 1 : -1;
        }

        if (b_int == 0) {
            b_int = (b >= 0) ? 1 : -1;
        }

        data.int64_pairs.emplace_back(a_int, b_int);
        data.double_pairs.emplace_back(a, b);
    }

    return data;
}

// Generate special test data for square root operations (positive values only)
SqrtTestData generateSqrtTestData(int count) {
    SqrtTestData data;
    // Add +1 to prevent any potential out-of-bounds access
    int allocSize = count + 1;
    data.fixed_values.reserve(allocSize);

    data.sf_values.reserve(allocSize);
    data.double_values.reserve(allocSize);

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dist(0.01, 1000.0);  // Positive values only

    // Generate positive values suitable for square root
    for (int i = 0; i < allocSize; i++) {
        double val = dist(gen);

        // Create fixed-point value
        data.fixed_values.emplace_back(val);

        // Create equivalent SoftDouble value
        data.sf_values.emplace_back(val);

        // Create float and double values for standard library operations
        data.double_values.push_back(val);
    }

    return data;
}

vector<BenchmarkResult> runBasicOperationsBenchmark(int iterations) {
    vector<BenchmarkResult> results;

    cout << "Generating test data..." << endl;
    TestData data = generateTestData(iterations);
    MultiplyDivideTestData mulDivData = generateMulDivTestData(iterations);
    SqrtTestData sqrtData = generateSqrtTestData(iterations);

    cout << "Running basic operations benchmark with " << iterations << " iterations..." << endl;

    // Addition benchmark
    BenchmarkResult addResult;
    addResult.operation = "Addition";

    double fixedAddTime = runBenchmark(
        "Fixed Addition",
        [&](int n) -> double {
            math::fp::Fixed64<32> sum(0);
            for (int k = 0; k < n; k++) {
                sum += data.fixed_values[k];
            }
            return static_cast<double>(sum);
        },
        iterations);

    double softAddTime = runBenchmark(
        "SoftDouble Addition",
        [&](int n) -> double {
            ::math::softfloat::float64_t sum(0);
            for (int k = 0; k < n; k++) {
                // Use operator+ instead of f64_add
                sum = sum + data.sf_values[k];
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Addition benchmark for float
    double int64AddTime = runBenchmark(
        "Int64 Addition",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                sum += data.int64_values[k];
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Addition benchmark for double
    double doubleAddTime = runBenchmark(
        "Double Addition",
        [&](int n) -> double {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                sum += data.double_values[k];
            }
            return sum;
        },
        iterations);

    addResult.times["Fixed64"] = fixedAddTime;

    addResult.times["SoftDouble"] = softAddTime;

    addResult.times["int64"] = int64AddTime;
    addResult.times["double"] = doubleAddTime;
    results.push_back(addResult);

    // Subtraction benchmark
    BenchmarkResult subResult;
    subResult.operation = "Subtraction";

    double fixedSubTime = runBenchmark(
        "Fixed Subtraction",
        [&](int n) -> double {
            math::fp::Fixed64<32> sum(0);
            for (int k = 0; k < n; k++) {
                sum -= data.fixed_values[k];
            }
            return static_cast<double>(sum);
        },
        iterations);

    double softSubTime = runBenchmark(
        "SoftDouble Subtraction",
        [&](int n) -> double {
            ::math::softfloat::float64_t sum(0);
            for (int k = 0; k < n; k++) {
                // Use operator- instead of f64_sub
                sum = sum - data.sf_values[k];
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Subtraction benchmark for float
    double int64SubTime = runBenchmark(
        "Int64 Subtraction",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                sum -= data.int64_values[k];
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Subtraction benchmark for double
    double doubleSubTime = runBenchmark(
        "Double Subtraction",
        [&](int n) -> double {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                sum -= data.double_values[k];
            }
            return sum;
        },
        iterations);

    subResult.times["Fixed64"] = fixedSubTime;

    subResult.times["SoftDouble"] = softSubTime;

    subResult.times["int64"] = int64SubTime;
    subResult.times["double"] = doubleSubTime;
    results.push_back(subResult);

    // Multiplication benchmark
    BenchmarkResult mulResult;
    mulResult.operation = "Multiplication";

    double fixedMulTime = runBenchmark(
        "Fixed Multiplication",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = mulDivData.fixed_pairs[k].first * mulDivData.fixed_pairs[k].second;
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        iterations);

    double softMulTime = runBenchmark(
        "SoftDouble Multiplication",
        [&](int n) -> double {
            ::math::softfloat::float64_t sum(0);
            for (int k = 0; k < n; k++) {
                // Use operator* instead of f64_mul
                auto result = mulDivData.sf_pairs[k].first * mulDivData.sf_pairs[k].second;
                sum = sum + result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Multiplication benchmark for float
    double int64MulTime = runBenchmark(
        "Int64 Multiplication",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                int64_t result = mulDivData.int64_pairs[k].first * mulDivData.int64_pairs[k].second;
                sum += result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Multiplication benchmark for double
    double doubleMulTime = runBenchmark(
        "Double Multiplication",
        [&](int n) -> double {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                double result =
                    mulDivData.double_pairs[k].first * mulDivData.double_pairs[k].second;
                sum += result;
            }
            return sum;
        },
        iterations);

    mulResult.times["Fixed64"] = fixedMulTime;

    mulResult.times["SoftDouble"] = softMulTime;

    mulResult.times["int64"] = int64MulTime;
    mulResult.times["double"] = doubleMulTime;
    results.push_back(mulResult);

    // Division benchmark
    BenchmarkResult divResult;
    divResult.operation = "Division";

    double fixedDivTime = runBenchmark(
        "Fixed Division",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = mulDivData.fixed_pairs[k].first / mulDivData.fixed_pairs[k].second;
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        iterations);

    double softDivTime = runBenchmark(
        "SoftDouble Division",
        [&](int n) -> double {
            ::math::softfloat::float64_t sum(0);
            for (int k = 0; k < n; k++) {
                // Use operator/ instead of f64_div
                auto result = mulDivData.sf_pairs[k].first / mulDivData.sf_pairs[k].second;
                sum = sum + result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Division benchmark for float
    double int64DivTime = runBenchmark(
        "Int64 Division",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                int64_t result = mulDivData.int64_pairs[k].first / mulDivData.int64_pairs[k].second;
                sum += result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Division benchmark for double
    double doubleDivTime = runBenchmark(
        "Double Division",
        [&](int n) -> double {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                double result =
                    mulDivData.double_pairs[k].first / mulDivData.double_pairs[k].second;
                sum += result;
            }
            return sum;
        },
        iterations);

    divResult.times["Fixed64"] = fixedDivTime;

    divResult.times["SoftDouble"] = softDivTime;

    divResult.times["int64"] = int64DivTime;
    divResult.times["double"] = doubleDivTime;
    results.push_back(divResult);

    // Square Root benchmark
    BenchmarkResult sqrtResult;
    sqrtResult.operation = "Square Root";

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
        iterations);

    double softSqrtTime = runBenchmark(
        "SoftDouble Square Root",
        [&](int n) -> double {
            ::math::softfloat::float64_t sum(0);
            for (int k = 0; k < n; k++) {
                // Use ::math::softfloat::sqrt instead of f64_sqrt
                auto result = ::math::softfloat::sqrt(sqrtData.sf_values[k]);
                sum = sum + result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Square Root benchmark for double
    double doubleSqrtTime = runBenchmark(
        "Double Square Root",
        [&](int n) -> double {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                int64_t result = static_cast<int64_t>(std::sqrt(sqrtData.double_values[k]));
                sum += result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    sqrtResult.times["Fixed64"] = fixedSqrtTime;
    sqrtResult.times["SoftDouble"] = softSqrtTime;
    sqrtResult.times["double"] = doubleSqrtTime;
    results.push_back(sqrtResult);

    return results;
}

}  // namespace benchmark