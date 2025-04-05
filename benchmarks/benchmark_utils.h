#pragma once

#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <thread>
#include <vector>

namespace benchmark {

// Benchmark framework
template <typename Func>
double runBenchmark(const std::string& name, Func func, int iterations, bool printResult = true) {
    // Warmup run
    func(std::min(10000, iterations));

    // Force alignment to prevent cache effects
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // Perform multiple timing runs and take the average
    const int TIMING_RUNS = 3;
    double totalTime = 0.0;

    for (int run = 0; run < TIMING_RUNS; run++) {
        auto start = std::chrono::high_resolution_clock::now();
        // Call the function and get a result that must be used
        auto result = func(iterations);
        auto end = std::chrono::high_resolution_clock::now();

        // Store the time
        double time =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
        totalTime += time;

        // Make sure the result is used to prevent optimization
        if (run == 0 && printResult) {
            std::cout << "  " << name << " - Run " << run + 1 << " result: " << std::scientific
                      << std::setprecision(6) << result << std::endl;
        }
    }

    return totalTime / TIMING_RUNS;
}

// Data structure to hold benchmark results
struct BenchmarkResult {
    std::string operation;
    std::map<std::string, double> times;  // implementation -> time in ms
};

}  // namespace benchmark