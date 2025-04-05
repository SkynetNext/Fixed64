#include "advanced_math_benchmark.h"

namespace benchmark {

MathTestData generateMathTestData(int count) {
    using namespace std;
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

std::vector<BenchmarkResult> runAdvancedMathBenchmark(int iterations) {
    using namespace std;
    std::vector<BenchmarkResult> results;

    cout << "Generating test data for advanced math functions..." << endl;
    MathTestData data = generateMathTestData(iterations);

    cout << "Running advanced math benchmark with " << iterations << " iterations..." << endl;

    // Pow2 benchmark
    BenchmarkResult pow2Result;
    pow2Result.operation = "Pow2";

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
        iterations);

    pow2Result.times["Fixed64"] = pow2Time;
    results.push_back(pow2Result);

    // Sin benchmark
    BenchmarkResult sinResult;
    sinResult.operation = "Sin";

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
        iterations);

    sinResult.times["Fixed64"] = sinTime;
    results.push_back(sinResult);

    // Acos benchmark
    BenchmarkResult acosResult;
    acosResult.operation = "Acos";

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
        iterations);

    acosResult.times["Fixed64"] = acosTime;
    results.push_back(acosResult);

    // Exp benchmark
    BenchmarkResult expResult;
    expResult.operation = "Exp";

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
        iterations);

    expResult.times["Fixed64"] = expTime;
    results.push_back(expResult);

    // Log benchmark
    BenchmarkResult logResult;
    logResult.operation = "Log";

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
        iterations);

    logResult.times["Fixed64"] = logTime;
    results.push_back(logResult);

    // Atan2 benchmark
    BenchmarkResult atan2Result;
    atan2Result.operation = "Atan2";

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
        iterations);

    atan2Result.times["Fixed64"] = atan2Time;
    results.push_back(atan2Result);

    return results;
}

}  // namespace benchmark