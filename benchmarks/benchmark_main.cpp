#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include "advanced_math_benchmark.h"
#include "basic_operations_benchmark.h"
#include "benchmark_utils.h"

using namespace std;
using namespace benchmark;

// Helper function to center text in a field of specified width
string center_text(const string& text, int width) {
    int padding = width - text.length();
    int left_padding = padding / 2;
    int right_padding = padding - left_padding;
    return string(left_padding, ' ') + text + string(right_padding, ' ');
}

// Format and display results as a table
void printResultsTable(const vector<BenchmarkResult>& results) {
    // Get unique implementations
    std::set<string> implementations;
    for (const auto& result : results) {
        for (const auto& [impl, time] : result.times) {
            implementations.insert(impl);
        }
    }

    // Print header
    cout << "\n========= FIXED64 PERFORMANCE BENCHMARK =========\n" << endl;

    // Calculate column width
    const int OP_COL_WIDTH = 20;
    const int TIME_COL_WIDTH = 24;  // Increased to accommodate longer values

    // Print header row - center-aligned headers
    cout << left << setw(OP_COL_WIDTH) << "Test Scenario";
    for (const auto& impl : implementations) {
        cout << " | " << center_text(impl, TIME_COL_WIDTH - 2);
    }
    cout << " |" << endl;

    // Print separator
    cout << string(OP_COL_WIDTH, '-');
    for (size_t i = 0; i < implementations.size(); i++) {
        cout << "-+-" << string(TIME_COL_WIDTH - 2, '-');
    }
    cout << "-|" << endl;

    // Find base implementation for relative speedup
    string baseImpl = *implementations.begin();

    // Print each operation row
    for (const auto& result : results) {
        // Left-align operation names (row headers)
        cout << left << setw(OP_COL_WIDTH) << result.operation;

        double baseTime = 0;
        if (result.times.count(baseImpl)) {
            baseTime = result.times.at(baseImpl);
        }

        for (const auto& impl : implementations) {
            cout << " | ";
            if (result.times.count(impl)) {
                double time = result.times.at(impl);

                // Format time with right alignment
                stringstream timeStr;
                timeStr << right << setw(9) << fixed << setprecision(2) << time << " ms";

                // If this isn't the base implementation, add speedup ratio
                string fullStr;
                if (impl != baseImpl && baseTime > 0) {
                    double ratio = time / baseTime;
                    stringstream ratioStr;
                    ratioStr << " (" << right << setw(6) << fixed << setprecision(2) << ratio
                             << "x)";
                    fullStr = timeStr.str() + ratioStr.str();
                } else {
                    fullStr =
                        timeStr.str() + "          ";  // Add spaces to align with ratio format
                }

                // Right-align the full string in its column
                cout << right << setw(TIME_COL_WIDTH - 2) << fullStr;
            } else {
                cout << right << setw(TIME_COL_WIDTH - 2) << "N/A";
            }
        }
        cout << " |" << endl;
    }

    cout << "\n" << endl;
}

int main() {
    const int ITERATIONS = 10000000;  // 10M iterations

    cout << "==== Fixed64 Performance Benchmark ====" << endl << endl;
    cout << "Build Information:" << endl;
    cout << "Size of void*: " << sizeof(void*) << " bytes" << endl;
    cout << "Size of size_t: " << sizeof(size_t) << " bytes" << endl;
    cout << "Size of int64_t: " << sizeof(int64_t) << " bytes" << endl << endl;

    // Run basic operations benchmark
    cout << "Part 1: Basic Operations Benchmark" << endl;
    cout << "----------------------------------" << endl;
    auto basicResults = runBasicOperationsBenchmark(ITERATIONS);

    // Run advanced math functions benchmark
    cout << "\nPart 2: Advanced Math Functions Benchmark" << endl;
    cout << "-----------------------------------------" << endl;
    auto advancedResults = runAdvancedMathBenchmark(ITERATIONS);

    // Display all results in a table
    vector<BenchmarkResult> allResults;
    allResults.insert(allResults.end(), basicResults.begin(), basicResults.end());
    allResults.insert(allResults.end(), advancedResults.begin(), advancedResults.end());

    printResultsTable(allResults);

    return 0;
}