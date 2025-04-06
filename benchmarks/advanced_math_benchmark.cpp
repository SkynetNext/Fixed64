#include "advanced_math_benchmark.h"

namespace benchmark {

#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 (M_PI / 2.0)
#endif

MathTestData generateMathTestData(int count) {
    using namespace std;
    MathTestData data;
    int allocSize = count + 1;

    // Pre-allocate space for all data structures
    data.unit_values.reserve(allocSize);
    data.positive_values.reserve(allocSize);
    data.angle_values.reserve(allocSize);
    data.atan2_pairs.reserve(allocSize);
    data.pow_pairs.reserve(allocSize);

    data.unit_values_float.reserve(allocSize);
    data.positive_values_float.reserve(allocSize);
    data.angle_values_float.reserve(allocSize);
    data.atan2_pairs_float.reserve(allocSize);
    data.pow_pairs_float.reserve(allocSize);

    data.unit_values_double.reserve(allocSize);
    data.positive_values_double.reserve(allocSize);
    data.angle_values_double.reserve(allocSize);
    data.atan2_pairs_double.reserve(allocSize);
    data.pow_pairs_double.reserve(allocSize);

    // Add soft_double data structures
    data.unit_values_softdouble.reserve(allocSize);
    data.positive_values_softdouble.reserve(allocSize);
    data.angle_values_softdouble.reserve(allocSize);
    data.atan2_pairs_softdouble.reserve(allocSize);
    data.pow_pairs_softdouble.reserve(allocSize);

    random_device rd;
    mt19937 gen(rd());

    // Different distributions for different function needs
    uniform_real_distribution<> unit_dist(-1.0, 1.0);          // For [-1, 1] range
    uniform_real_distribution<> positive_dist(0.001, 10.0);    // For positive values
    uniform_real_distribution<> angle_dist(0.0, 6.283185307);  // For [0, 2π] range
    uniform_real_distribution<> atan2_dist(-10.0, 10.0);       // For atan2 inputs

    for (int i = 0; i < allocSize; i++) {
        // Generate a single random value and convert to different types to ensure we're comparing
        // the same values
        double unit_val = unit_dist(gen);
        double positive_val = positive_dist(gen);
        double angle_val = angle_dist(gen);
        double y_val = atan2_dist(gen);
        double x_val = atan2_dist(gen);

        // Fixed-point data
        data.unit_values.emplace_back(unit_val);
        data.positive_values.emplace_back(positive_val);
        data.angle_values.emplace_back(angle_val);
        data.atan2_pairs.emplace_back(math::fp::Fixed64<32>(y_val), math::fp::Fixed64<32>(x_val));

        // Float data
        data.unit_values_float.push_back(static_cast<float>(unit_val));
        data.positive_values_float.push_back(static_cast<float>(positive_val));
        data.angle_values_float.push_back(static_cast<float>(angle_val));
        data.atan2_pairs_float.emplace_back(static_cast<float>(y_val), static_cast<float>(x_val));

        // Double data
        data.unit_values_double.push_back(unit_val);
        data.positive_values_double.push_back(positive_val);
        data.angle_values_double.push_back(angle_val);
        data.atan2_pairs_double.emplace_back(y_val, x_val);

        // SoftDouble data
        data.unit_values_softdouble.push_back(::math::softfloat::float64_t(unit_val));
        data.positive_values_softdouble.push_back(::math::softfloat::float64_t(positive_val));
        data.angle_values_softdouble.push_back(::math::softfloat::float64_t(angle_val));
        data.atan2_pairs_softdouble.emplace_back(::math::softfloat::float64_t(y_val),
                                                 ::math::softfloat::float64_t(x_val));
    }

    // Generate test data pairs (base, exponent) for Pow function
    uniform_real_distribution<> base_dist(0.1, 4.0);  // Positive bases to avoid complex results
    uniform_real_distribution<> exp_dist(-2.0, 2.0);  // Mix of negative and positive exponents

    for (int i = 0; i < count; ++i) {
        // Generate random base and exponent values
        double base_val = base_dist(gen);
        double exp_val = exp_dist(gen);

        // Create Fixed64 pair
        auto base_fixed = math::fp::Fixed64<32>(base_val);
        auto exp_fixed = math::fp::Fixed64<32>(exp_val);
        data.pow_pairs.emplace_back(base_fixed, exp_fixed);

        // Create equivalent float and double pairs
        data.pow_pairs_float.emplace_back(static_cast<float>(base_val),
                                          static_cast<float>(exp_val));
        data.pow_pairs_double.emplace_back(base_val, exp_val);

        // Create SoftDouble pair
        data.pow_pairs_softdouble.emplace_back(::math::softfloat::float64_t(base_val),
                                               ::math::softfloat::float64_t(exp_val));
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
        "Pow2 (Fixed64)",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = math::fp::Fixed64Math::Pow2(data.unit_values[k]);
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        iterations);

    double pow2SoftDoubleTime = runBenchmark(
        "Pow2 (SoftDouble)",
        [&](int n) -> double {
            ::math::softfloat::float64_t sum(0);
            for (int k = 0; k < n; k++) {
                // If exp2 isn't directly available, use pow(2,x) instead
                auto result = ::math::softfloat::pow(::math::softfloat::float64_t(2.0),
                                                     data.unit_values_softdouble[k]);
                sum = sum + result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Add float benchmark
    double pow2FloatTime = runBenchmark(
        "Pow2 (float)",
        [&](int n) -> double {
            float sum = 0;
            for (int k = 0; k < n; k++) {
                float value = static_cast<float>(data.unit_values_float[k]);
                float result = std::exp2f(value);
                sum += result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Add double benchmark
    double pow2DoubleTime = runBenchmark(
        "Pow2 (double)",
        [&](int n) -> double {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                double value = data.unit_values_double[k];
                double result = std::exp2(value);
                sum += result;
            }
            return sum;
        },
        iterations);

    pow2Result.times["Fixed64"] = pow2Time;
    pow2Result.times["SoftDouble"] = pow2SoftDoubleTime;
    pow2Result.times["float"] = pow2FloatTime;
    pow2Result.times["double"] = pow2DoubleTime;
    results.push_back(pow2Result);

    // Sin benchmark
    BenchmarkResult sinResult;
    sinResult.operation = "Sin";

    double sinTime = runBenchmark(
        "Sin (Fixed64)",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = math::fp::Fixed64Math::Sin(data.angle_values[k]);
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        iterations);

    double sinSoftDoubleTime = runBenchmark(
        "Sin (SoftDouble)",
        [&](int n) -> double {
            ::math::softfloat::float64_t sum(0);
            for (int k = 0; k < n; k++) {
                auto result = ::math::softfloat::sin(data.angle_values_softdouble[k]);
                sum = sum + result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Add float benchmark using pre-converted values
    double sinFloatTime = runBenchmark(
        "Sin (float)",
        [&](int n) -> double {
            float sum = 0;
            for (int k = 0; k < n; k++) {
                float result = std::sinf(data.angle_values_float[k]);
                sum += result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Add double benchmark using pre-converted values
    double sinDoubleTime = runBenchmark(
        "Sin (double)",
        [&](int n) -> double {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                double result = std::sin(data.angle_values_double[k]);
                sum += result;
            }
            return sum;
        },
        iterations);

    sinResult.times["Fixed64"] = sinTime;
    sinResult.times["SoftDouble"] = sinSoftDoubleTime;
    sinResult.times["float"] = sinFloatTime;
    sinResult.times["double"] = sinDoubleTime;
    results.push_back(sinResult);

    // Acos benchmark
    BenchmarkResult acosResult;
    acosResult.operation = "Acos";

    double acosTime = runBenchmark(
        "Acos (Fixed64)",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = math::fp::Fixed64Math::Acos(data.unit_values[k]);
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        iterations);

    double acosSoftDoubleTime = runBenchmark(
        "Acos (SoftDouble)",
        [&](int n) -> double {
            ::math::softfloat::float64_t sum(0);
            for (int k = 0; k < n; k++) {
                auto result = ::math::softfloat::acos(data.unit_values_softdouble[k]);
                sum = sum + result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Add float benchmark
    double acosFloatTime = runBenchmark(
        "Acos (float)",
        [&](int n) -> double {
            float sum = 0;
            for (int k = 0; k < n; k++) {
                float value = static_cast<float>(data.unit_values_float[k]);
                float result = std::acosf(value);
                sum += result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Add double benchmark
    double acosDoubleTime = runBenchmark(
        "Acos (double)",
        [&](int n) -> double {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                double value = data.unit_values_double[k];
                double result = std::acos(value);
                sum += result;
            }
            return sum;
        },
        iterations);

    acosResult.times["Fixed64"] = acosTime;
    acosResult.times["SoftDouble"] = acosSoftDoubleTime;
    acosResult.times["float"] = acosFloatTime;
    acosResult.times["double"] = acosDoubleTime;
    results.push_back(acosResult);

    // Exp benchmark
    BenchmarkResult expResult;
    expResult.operation = "Exp";

    double expTime = runBenchmark(
        "Exp (Fixed64)",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = math::fp::Fixed64Math::Exp(data.unit_values[k]);
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        iterations);

    double expSoftDoubleTime = runBenchmark(
        "Exp (SoftDouble)",
        [&](int n) -> double {
            ::math::softfloat::float64_t sum(0);
            for (int k = 0; k < n; k++) {
                auto result = ::math::softfloat::exp(data.unit_values_softdouble[k]);
                sum = sum + result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Add float benchmark
    double expFloatTime = runBenchmark(
        "Exp (float)",
        [&](int n) -> double {
            float sum = 0;
            for (int k = 0; k < n; k++) {
                float result = std::expf(data.unit_values_float[k]);
                sum += result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Add double benchmark
    double expDoubleTime = runBenchmark(
        "Exp (double)",
        [&](int n) -> double {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                double result = std::exp(data.unit_values_double[k]);
                sum += result;
            }
            return sum;
        },
        iterations);

    expResult.times["Fixed64"] = expTime;
    expResult.times["SoftDouble"] = expSoftDoubleTime;
    expResult.times["float"] = expFloatTime;
    expResult.times["double"] = expDoubleTime;
    results.push_back(expResult);

    // Log benchmark
    BenchmarkResult logResult;
    logResult.operation = "Log";

    double logTime = runBenchmark(
        "Log (Fixed64)",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = math::fp::Fixed64Math::Log(data.positive_values[k]);
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        iterations);

    double logSoftDoubleTime = runBenchmark(
        "Log (SoftDouble)",
        [&](int n) -> double {
            ::math::softfloat::float64_t sum(0);
            for (int k = 0; k < n; k++) {
                auto result = ::math::softfloat::log(data.positive_values_softdouble[k]);
                sum = sum + result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Add float benchmark
    double logFloatTime = runBenchmark(
        "Log (float)",
        [&](int n) -> double {
            float sum = 0;
            for (int k = 0; k < n; k++) {
                float result = std::logf(data.positive_values_float[k]);
                sum += result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Add double benchmark
    double logDoubleTime = runBenchmark(
        "Log (double)",
        [&](int n) -> double {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                double result = std::log(data.positive_values_double[k]);
                sum += result;
            }
            return sum;
        },
        iterations);

    logResult.times["Fixed64"] = logTime;
    logResult.times["SoftDouble"] = logSoftDoubleTime;
    logResult.times["float"] = logFloatTime;
    logResult.times["double"] = logDoubleTime;
    results.push_back(logResult);

    // Atan benchmark
    BenchmarkResult atanResult;
    atanResult.operation = "Atan";

    double atanTime = runBenchmark(
        "Atan (Fixed64)",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result = math::fp::Fixed64Math::Atan(data.unit_values[k]);
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        iterations);

    double atanSoftDoubleTime = runBenchmark(
        "Atan (SoftDouble)",
        [&](int n) -> double {
            ::math::softfloat::float64_t sum(0);
            for (int k = 0; k < n; k++) {
                auto result = ::math::softfloat::atan(data.unit_values_softdouble[k]);
                sum = sum + result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Add float benchmark
    double atanFloatTime = runBenchmark(
        "Atan (float)",
        [&](int n) -> double {
            float sum = 0;
            for (int k = 0; k < n; k++) {
                float result = std::atanf(data.unit_values_float[k]);
                sum += result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Add double benchmark
    double atanDoubleTime = runBenchmark(
        "Atan (double)",
        [&](int n) -> double {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                double result = std::atan(data.unit_values_double[k]);
                sum += result;
            }
            return sum;
        },
        iterations);

    atanResult.times["Fixed64"] = atanTime;
    atanResult.times["SoftDouble"] = atanSoftDoubleTime;
    atanResult.times["float"] = atanFloatTime;
    atanResult.times["double"] = atanDoubleTime;
    results.push_back(atanResult);

    // Atan2 benchmark
    BenchmarkResult atan2Result;
    atan2Result.operation = "Atan2";

    double atan2Time = runBenchmark(
        "Atan2 (Fixed64)",
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

    double atan2SoftDoubleTime = runBenchmark(
        "Atan2 (SoftDouble)",
        [&](int n) -> double {
            ::math::softfloat::float64_t sum(0);
            for (int k = 0; k < n; k++) {
                // Implementation for when atan2 isn't directly available
                auto y = data.atan2_pairs_softdouble[k].first;
                auto x = data.atan2_pairs_softdouble[k].second;

                // Manual implementation of atan2 using available functions
                ::math::softfloat::float64_t result;

                if (x > ::math::softfloat::float64_t(0)) {
                    result = ::math::softfloat::atan(y / x);
                } else if (x < ::math::softfloat::float64_t(0)) {
                    if (y >= ::math::softfloat::float64_t(0)) {
                        result =
                            ::math::softfloat::atan(y / x) + ::math::softfloat::float64_t(M_PI);
                    } else {
                        result =
                            ::math::softfloat::atan(y / x) - ::math::softfloat::float64_t(M_PI);
                    }
                } else {  // x == 0
                    if (y > ::math::softfloat::float64_t(0)) {
                        result = ::math::softfloat::float64_t(M_PI / 2);
                    } else if (y < ::math::softfloat::float64_t(0)) {
                        result = ::math::softfloat::float64_t(-M_PI / 2);
                    } else {                                       // y == 0
                        result = ::math::softfloat::float64_t(0);  // Undefined, but return 0
                    }
                }

                sum = sum + result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Add float benchmark
    double atan2FloatTime = runBenchmark(
        "Atan2 (float)",
        [&](int n) -> double {
            float sum = 0;
            for (int k = 0; k < n; k++) {
                float result =
                    std::atan2f(data.atan2_pairs_float[k].first, data.atan2_pairs_float[k].second);
                sum += result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    // Add double benchmark
    double atan2DoubleTime = runBenchmark(
        "Atan2 (double)",
        [&](int n) -> double {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                double result =
                    std::atan2(data.atan2_pairs_double[k].first, data.atan2_pairs_double[k].second);
                sum += result;
            }
            return sum;
        },
        iterations);

    atan2Result.times["Fixed64"] = atan2Time;
    atan2Result.times["SoftDouble"] = atan2SoftDoubleTime;
    atan2Result.times["float"] = atan2FloatTime;
    atan2Result.times["double"] = atan2DoubleTime;
    results.push_back(atan2Result);

    // Power function benchmark
    BenchmarkResult powResult;
    powResult.operation = "Pow";

    double fixedPowTime = runBenchmark(
        "Fixed Power",
        [&](int n) -> double {
            int64_t sum = 0;
            for (int k = 0; k < n; k++) {
                auto result =
                    math::fp::Fixed64Math::Pow(data.pow_pairs[k].first, data.pow_pairs[k].second);
                sum += result.value();
            }
            return static_cast<double>(sum);
        },
        iterations);

    double softDoublePowTime = runBenchmark(
        "SoftDouble Power",
        [&](int n) -> double {
            ::math::softfloat::float64_t sum(0);
            for (int k = 0; k < n; k++) {
                auto result = ::math::softfloat::pow(data.pow_pairs_softdouble[k].first,
                                                     data.pow_pairs_softdouble[k].second);
                sum = sum + result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    double floatPowTime = runBenchmark(
        "Float Power",
        [&](int n) -> double {
            float sum = 0;
            for (int k = 0; k < n; k++) {
                float result =
                    std::powf(data.pow_pairs_float[k].first, data.pow_pairs_float[k].second);
                sum += result;
            }
            return static_cast<double>(sum);
        },
        iterations);

    double doublePowTime = runBenchmark(
        "Double Power",
        [&](int n) -> double {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                double result =
                    std::pow(data.pow_pairs_double[k].first, data.pow_pairs_double[k].second);
                sum += result;
            }
            return sum;
        },
        iterations);

    powResult.times["Fixed64"] = fixedPowTime;
    powResult.times["SoftDouble"] = softDoublePowTime;
    powResult.times["float"] = floatPowTime;
    powResult.times["double"] = doublePowTime;
    results.push_back(powResult);

    return results;
}

}  // namespace benchmark