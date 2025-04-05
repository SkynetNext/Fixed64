#include "basic_operations_benchmark.h"

namespace benchmark {

// Generate test data with consistent patterns for fair comparison
TestData generateTestData(int count) {
    TestData data;
    // Add +1 to prevent out-of-bounds access when using k+1 index
    int allocSize = count + 1;
    data.fixed_values.reserve(allocSize);
    data.sf_values.reserve(allocSize);
    data.float_values.reserve(allocSize);
    data.double_values.reserve(allocSize);
    data.indices.reserve(allocSize);

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dist(-100.0, 100.0);  // More reasonable range
    uniform_int_distribution<> idx_dist(0, allocSize - 1);

    // Generate values with the same pattern for all implementations
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

        // Create float and double values for standard library operations
        data.float_values.push_back(static_cast<float>(val));
        data.double_values.push_back(val);

        // Generate random index for access pattern
        data.indices.push_back(idx_dist(gen));
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
    data.float_pairs.reserve(allocSize);
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

        // Create equivalent SoftFloat pair
        union {
            double d;
            uint64_t u;
        } conv_a, conv_b;

        conv_a.d = a;
        conv_b.d = b;

        data.sf_pairs.emplace_back(float64_t{conv_a.u}, float64_t{conv_b.u});

        // Create float and double pairs for standard library operations
        data.float_pairs.emplace_back(static_cast<float>(a), static_cast<float>(b));
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
    data.float_values.reserve(allocSize);
    data.double_values.reserve(allocSize);

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

        // Create float and double values for standard library operations
        data.float_values.push_back(static_cast<float>(val));
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
        "SoftFloat Addition",
        [&](int n) -> double {
            float64_t sum(0);
            for (int k = 0; k < n; k++) {
                sum = f64_add(sum, data.sf_values[k]);
            }
            return static_cast<double>(sum.v);
        },
        iterations);

    // Addition benchmark for float
    double floatAddTime = runBenchmark(
        "Float Addition",
        [&](int n) -> double {
            float sum = 0;
            for (int k = 0; k < n; k++) {
                sum += data.float_values[k];
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
    addResult.times["SoftFloat"] = softAddTime;
    addResult.times["float"] = floatAddTime;
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
        "SoftFloat Subtraction",
        [&](int n) -> double {
            float64_t sum(0);
            for (int k = 0; k < n; k++) {
                sum = f64_sub(sum, data.sf_values[k]);
            }
            return static_cast<double>(sum.v);
        },
        iterations);

    // Subtraction benchmark for float
    double floatSubTime = runBenchmark(
        "Float Subtraction",
        [&](int n) -> double {
            float sum = 0;
            for (int k = 0; k < n; k++) {
                sum += data.float_values[k];
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
    subResult.times["SoftFloat"] = softSubTime;
    subResult.times["float"] = floatSubTime;
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
        "SoftFloat Multiplication",
        [&](int n) -> double {
            uint64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = f64_mul(mulDivData.sf_pairs[k].first, mulDivData.sf_pairs[k].second);
                sum += result.v;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Multiplication benchmark for float
    double floatMulTime = runBenchmark(
        "Float Multiplication",
        [&](int n) -> double {
            float sum = 0;
            for (int k = 0; k < n; k++) {
                float result = mulDivData.float_pairs[k].first * mulDivData.float_pairs[k].second;
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
    mulResult.times["SoftFloat"] = softMulTime;
    mulResult.times["float"] = floatMulTime;
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
        "SoftFloat Division",
        [&](int n) -> double {
            uint64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = f64_div(mulDivData.sf_pairs[k].first, mulDivData.sf_pairs[k].second);
                sum += result.v;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Division benchmark for float
    double floatDivTime = runBenchmark(
        "Float Division",
        [&](int n) -> double {
            float sum = 0;
            for (int k = 0; k < n; k++) {
                float result = mulDivData.float_pairs[k].first / mulDivData.float_pairs[k].second;
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
    divResult.times["SoftFloat"] = softDivTime;
    divResult.times["float"] = floatDivTime;
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
        "SoftFloat Square Root",
        [&](int n) -> double {
            uint64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = f64_sqrt(sqrtData.sf_values[k]);
                sum += result.v;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Square Root benchmark for float
    double floatSqrtTime = runBenchmark(
        "Float Square Root",
        [&](int n) -> double {
            float sum = 0;
            for (int k = 0; k < n; k++) {
                float result = std::sqrtf(sqrtData.float_values[k]);
                sum += result;
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
                double result = std::sqrt(sqrtData.double_values[k]);
                sum += result;
            }
            return sum;
        },
        iterations);

    sqrtResult.times["Fixed64"] = fixedSqrtTime;
    sqrtResult.times["SoftFloat"] = softSqrtTime;
    sqrtResult.times["float"] = floatSqrtTime;
    sqrtResult.times["double"] = doubleSqrtTime;
    results.push_back(sqrtResult);

    return results;
}

}  // namespace benchmark