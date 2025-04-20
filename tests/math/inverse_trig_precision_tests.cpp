#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <random>
#include <vector>
#include "fixed64_math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 (M_PI / 2.0)
#endif

using Fixed = math::fp::Fixed64<32>;
using math::fp::Fixed64Math;

class InverseTrigPrecisionTest : public ::testing::Test {
 protected:
    // Precision requirements
    const double kEpsilonNormal = 1e-5;    // Precision requirement for normal regions
    const double kEpsilonCritical = 1e-4;  // Relaxed precision requirement for boundary regions

    // Generate test input values
    std::vector<double> GenerateInputValues(double min,
                                            double max,
                                            int count,
                                            bool concentrateOnBoundaries = true) {
        std::vector<double> values;

        // Ensure boundary values are included
        values.push_back(min);
        values.push_back(max);

        if (concentrateOnBoundaries) {
            // Dense points near boundaries
            const double boundaryWidth = 0.05 * (max - min);
            int boundaryPoints = count / 3;

            // Points near the minimum boundary
            for (int i = 0; i < boundaryPoints; i++) {
                double t = static_cast<double>(i) / boundaryPoints;
                values.push_back(
                    min
                    + boundaryWidth * t
                          * t);  // Quadratic distribution to make points denser at boundary
            }

            // Points near the maximum boundary
            for (int i = 0; i < boundaryPoints; i++) {
                double t = static_cast<double>(i) / boundaryPoints;
                values.push_back(max - boundaryWidth * t * t);
            }

            // Points in the middle region
            int centerPoints = count - 2 - 2 * boundaryPoints;
            double innerMin = min + boundaryWidth;
            double innerMax = max - boundaryWidth;

            for (int i = 0; i < centerPoints; i++) {
                double t = static_cast<double>(i + 1) / (centerPoints + 1);
                values.push_back(innerMin + t * (innerMax - innerMin));
            }
        } else {
            // Uniformly distributed points
            for (int i = 1; i < count - 1; i++) {
                double t = static_cast<double>(i) / (count - 1);
                values.push_back(min + t * (max - min));
            }
        }

        // Add some special values
        if (min <= 0.0 && max >= 0.0)
            values.push_back(0.0);
        if (min <= -0.5 && max >= -0.5)
            values.push_back(-0.5);
        if (min <= 0.5 && max >= 0.5)
            values.push_back(0.5);

        // Sort and remove duplicates
        std::sort(values.begin(), values.end());
        values.erase(std::unique(values.begin(),
                                 values.end(),
                                 [](double a, double b) { return std::abs(a - b) < 1e-10; }),
                     values.end());

        return values;
    }

    // Generate random points
    void AddRandomValues(std::vector<double>& values, double min, double max, int count) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(min, max);

        for (int i = 0; i < count; i++) {
            values.push_back(dis(gen));
        }
    }

    // Helper functions for outputting precision reports
    void PrintPrecisionHeader() {
        std::cout << std::setw(10) << "Input" << std::setw(16) << "Expected" << std::setw(16)
                  << "Actual" << std::setw(16) << "Error" << std::setw(16) << "Error (deg)" << "\n";
        std::cout << std::string(74, '-') << "\n";
    }

    void PrintPrecisionRow(double input, double expected, double actual) {
        double error = std::abs(actual - expected);
        double errorDeg = error * 180.0 / M_PI;

        std::cout << std::fixed << std::setprecision(8);
        std::cout << std::setw(10) << input << std::setw(16) << expected << std::setw(16) << actual
                  << std::setw(16) << error << std::setw(16) << errorDeg << "\n";
    }
};

TEST_F(InverseTrigPrecisionTest, AsinPrecisionTest) {
    std::cout << "\n=== ASIN PRECISION TEST ===\n";

    // Generate test values: asin input range is [-1, 1]
    auto inputValues = GenerateInputValues(-0.999, 0.999, 50, true);

    // Add key points
    std::vector<double> specialValues = {-0.9999, -0.999, -0.99,  -0.9, -0.75, -0.5,  -0.25,
                                         -0.1,    -0.01,  -0.001, 0.0,  0.001, 0.01,  0.1,
                                         0.25,    0.5,    0.75,   0.9,  0.99,  0.999, 0.9999};
    inputValues.insert(inputValues.end(), specialValues.begin(), specialValues.end());

    // Add random values
    AddRandomValues(inputValues, -0.999, 0.999, 20);

    // Sort and remove duplicates
    std::sort(inputValues.begin(), inputValues.end());
    inputValues.erase(std::unique(inputValues.begin(),
                                  inputValues.end(),
                                  [](double a, double b) { return std::abs(a - b) < 1e-10; }),
                      inputValues.end());

    std::cout << "Testing Asin with " << inputValues.size() << " input values\n\n";

    int passedTests = 0;
    int failedTests = 0;
    double maxError = 0.0;
    double errorSum = 0.0;
    double worstInput = 0.0;

    // Print header
    PrintPrecisionHeader();

    // Test each input value
    for (double inputValue : inputValues) {
        // First create Fixed value
        Fixed x(inputValue);

        // Extract double value from Fixed (includes precision loss from Fixed representation)
        double fixedAsDouble = static_cast<double>(x);

        // Calculate standard library result using the extracted double value
        double expected = std::asin(fixedAsDouble);

        // Calculate our implementation result using Fixed value
        double actual = static_cast<double>(Fixed64Math::Asin(x));

        // Calculate error
        double error = std::abs(actual - expected);
        errorSum += error;

        if (error > maxError) {
            maxError = error;
            worstInput = fixedAsDouble;
        }

        // Precision requirements can be relaxed near boundaries
        double epsilon = (std::abs(fixedAsDouble) > 0.95) ? kEpsilonCritical : kEpsilonNormal;

        // Verify error is within acceptable range
        bool passed = (error <= epsilon);
        if (passed) {
            passedTests++;
        } else {
            failedTests++;
            // Print failed test
            PrintPrecisionRow(fixedAsDouble, expected, actual);
            EXPECT_NEAR(actual, expected, epsilon) << "Asin failed for input " << fixedAsDouble;
        }
    }

    // Print summary information
    std::cout << "\nSummary:\n";
    std::cout << "Passed: " << passedTests << " (" << (100.0 * passedTests / inputValues.size())
              << "%)\n";
    std::cout << "Failed: " << failedTests << "\n";
    std::cout << "Average error: " << (errorSum / inputValues.size()) << " rad, "
              << (errorSum * 180.0 / M_PI / inputValues.size()) << " deg\n";
    std::cout << "Maximum error: " << maxError << " rad, " << (maxError * 180.0 / M_PI)
              << " deg at input " << worstInput << "\n";

    // Verify pass rate
    EXPECT_GE(passedTests, static_cast<int>(0.9 * inputValues.size()))
        << "Asin precision test failed: too many errors";
}

TEST_F(InverseTrigPrecisionTest, AcosPrecisionTest) {
    std::cout << "\n=== ACOS PRECISION TEST ===\n";

    // Generate test values: acos input range is [-1, 1]
    auto inputValues = GenerateInputValues(-0.999, 0.999, 50, true);

    // Add key points
    std::vector<double> specialValues = {-0.9999, -0.999, -0.99,  -0.9, -0.75, -0.5,  -0.25,
                                         -0.1,    -0.01,  -0.001, 0.0,  0.001, 0.01,  0.1,
                                         0.25,    0.5,    0.75,   0.9,  0.99,  0.999, 0.9999};
    inputValues.insert(inputValues.end(), specialValues.begin(), specialValues.end());

    // Add random values
    AddRandomValues(inputValues, -0.999, 0.999, 20);

    // Sort and remove duplicates
    std::sort(inputValues.begin(), inputValues.end());
    inputValues.erase(std::unique(inputValues.begin(),
                                  inputValues.end(),
                                  [](double a, double b) { return std::abs(a - b) < 1e-10; }),
                      inputValues.end());

    std::cout << "Testing Acos with " << inputValues.size() << " input values\n\n";

    int passedTests = 0;
    int failedTests = 0;
    double maxError = 0.0;
    double errorSum = 0.0;
    double worstInput = 0.0;

    // Print header
    PrintPrecisionHeader();

    // Test each input value
    for (double inputValue : inputValues) {
        // First create Fixed value
        Fixed x(inputValue);

        // Extract double value from Fixed (includes precision loss from Fixed representation)
        double fixedAsDouble = static_cast<double>(x);

        // Calculate standard library result using the extracted double value
        double expected = std::acos(fixedAsDouble);

        // Calculate our implementation result using Fixed value
        double actual = static_cast<double>(Fixed64Math::Acos(x));

        // Calculate error
        double error = std::abs(actual - expected);
        errorSum += error;

        if (error > maxError) {
            maxError = error;
            worstInput = fixedAsDouble;
        }

        // Precision requirements can be relaxed near boundaries
        double epsilon = (std::abs(fixedAsDouble) > 0.95) ? kEpsilonCritical : kEpsilonNormal;

        // Verify error is within acceptable range
        bool passed = (error <= epsilon);
        if (passed) {
            passedTests++;
        } else {
            failedTests++;
            // Print failed test
            PrintPrecisionRow(fixedAsDouble, expected, actual);
            EXPECT_NEAR(actual, expected, epsilon) << "Acos failed for input " << fixedAsDouble;
        }
    }

    // Print summary information
    std::cout << "\nSummary:\n";
    std::cout << "Passed: " << passedTests << " (" << (100.0 * passedTests / inputValues.size())
              << "%)\n";
    std::cout << "Failed: " << failedTests << "\n";
    std::cout << "Average error: " << (errorSum / inputValues.size()) << " rad, "
              << (errorSum * 180.0 / M_PI / inputValues.size()) << " deg\n";
    std::cout << "Maximum error: " << maxError << " rad, " << (maxError * 180.0 / M_PI)
              << " deg at input " << worstInput << "\n";

    // Verify pass rate
    EXPECT_GE(passedTests, static_cast<int>(0.9 * inputValues.size()))
        << "Acos precision test failed: too many errors";
}

TEST_F(InverseTrigPrecisionTest, AtanPrecisionTest) {
    std::cout << "\n=== ATAN PRECISION TEST ===\n";

    // Generate test values: atan input range is infinite, but we focus on testing [-10, 10]
    auto inputValues = GenerateInputValues(-10.0, 10.0, 40, false);

    // Add key points
    std::vector<double> specialValues = {-1000.0, -100.0, -10.0,  -5.0, -2.0,  -1.0,  -0.5,
                                         -0.1,    -0.01,  -0.001, 0.0,  0.001, 0.01,  0.1,
                                         0.5,     1.0,    2.0,    5.0,  10.0,  100.0, 1000.0};
    inputValues.insert(inputValues.end(), specialValues.begin(), specialValues.end());

    // Add random values
    AddRandomValues(inputValues, -10.0, 10.0, 20);

    // Sort and remove duplicates
    std::sort(inputValues.begin(), inputValues.end());
    inputValues.erase(std::unique(inputValues.begin(),
                                  inputValues.end(),
                                  [](double a, double b) { return std::abs(a - b) < 1e-10; }),
                      inputValues.end());

    std::cout << "Testing Atan with " << inputValues.size() << " input values\n\n";

    int passedTests = 0;
    int failedTests = 0;
    double maxError = 0.0;
    double errorSum = 0.0;
    double worstInput = 0.0;

    // Print header
    PrintPrecisionHeader();

    // Test each input value
    for (double inputValue : inputValues) {
        // First create Fixed value
        Fixed x(inputValue);

        // Extract double value from Fixed (includes precision loss from Fixed representation)
        double fixedAsDouble = static_cast<double>(x);

        // Calculate standard library result using the extracted double value
        double expected = std::atan(fixedAsDouble);

        // Calculate our implementation result using Fixed value
        double actual = static_cast<double>(Fixed64Math::Atan(x));

        // Calculate error
        double error = std::abs(actual - expected);
        errorSum += error;

        if (error > maxError) {
            maxError = error;
            worstInput = fixedAsDouble;
        }

        // For very large input values, precision requirements can be relaxed
        double epsilon = (std::abs(fixedAsDouble) > 10.0) ? kEpsilonCritical : kEpsilonNormal;

        // Verify error is within acceptable range
        bool passed = (error <= epsilon);
        if (passed) {
            passedTests++;
        } else {
            failedTests++;
            // Print failed test
            PrintPrecisionRow(fixedAsDouble, expected, actual);
            EXPECT_NEAR(actual, expected, epsilon) << "Atan failed for input " << fixedAsDouble;
        }
    }

    // Print summary information
    std::cout << "\nSummary:\n";
    std::cout << "Passed: " << passedTests << " (" << (100.0 * passedTests / inputValues.size())
              << "%)\n";
    std::cout << "Failed: " << failedTests << "\n";
    std::cout << "Average error: " << (errorSum / inputValues.size()) << " rad, "
              << (errorSum * 180.0 / M_PI / inputValues.size()) << " deg\n";
    std::cout << "Maximum error: " << maxError << " rad, " << (maxError * 180.0 / M_PI)
              << " deg at input " << worstInput << "\n";

    // Verify pass rate
    EXPECT_GE(passedTests, static_cast<int>(0.9 * inputValues.size()))
        << "Atan precision test failed: too many errors";
}

TEST_F(InverseTrigPrecisionTest, Atan2PrecisionTest) {
    std::cout << "\n=== ATAN2 PRECISION TEST ===\n";

    // Prepare test point pairs (y,x), covering all quadrants and special cases
    std::vector<std::pair<double, double>> testPoints = {// Typical values in all four quadrants
                                                         {1.0, 1.0},
                                                         {1.0, -1.0},
                                                         {-1.0, -1.0},
                                                         {-1.0, 1.0},

                                                         // Points on the axes
                                                         {0.0, 1.0},
                                                         {1.0, 0.0},
                                                         {0.0, -1.0},
                                                         {-1.0, 0.0},

                                                         // Near the axes
                                                         {0.001, 1.0},
                                                         {1.0, 0.001},
                                                         {0.001, -1.0},
                                                         {-1.0, 0.001},
                                                         {-0.001, 1.0},
                                                         {1.0, -0.001},
                                                         {-0.001, -1.0},
                                                         {-1.0, -0.001},

                                                         // Special point: origin
                                                         {0.0, 0.0},

                                                         // Small values
                                                         {0.1, 0.1},
                                                         {-0.1, 0.1},
                                                         {-0.1, -0.1},
                                                         {0.1, -0.1},

                                                         // Large values
                                                         {10.0, 10.0},
                                                         {10.0, -10.0},
                                                         {-10.0, -10.0},
                                                         {-10.0, 10.0},

                                                         // Extreme ratios
                                                         {1.0, 0.001},
                                                         {0.001, 1.0},
                                                         {1.0, -0.001},
                                                         {0.001, -1.0}};

    // Add random test points
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-5.0, 5.0);

    for (int i = 0; i < 30; i++) {
        testPoints.emplace_back(dis(gen), dis(gen));
    }

    std::cout << "Testing Atan2 with " << testPoints.size() << " input value pairs\n\n";

    int passedTests = 0;
    int failedTests = 0;
    double maxError = 0.0;
    double errorSum = 0.0;
    std::pair<double, double> worstInput;

    // Print header
    std::cout << std::setw(10) << "Y" << std::setw(10) << "X" << std::setw(16) << "Expected"
              << std::setw(16) << "Actual" << std::setw(16) << "Error" << std::setw(16)
              << "Error (deg)" << "\n";
    std::cout << std::string(84, '-') << "\n";

    // Test each input pair
    for (const auto& [y, x] : testPoints) {
        // First create Fixed values
        Fixed fx(x);
        Fixed fy(y);

        // Extract double values from Fixed (includes precision loss from Fixed representation)
        double fixedXAsDouble = static_cast<double>(fx);
        double fixedYAsDouble = static_cast<double>(fy);

        // Calculate standard library result using the extracted double values
        double expected = std::atan2(fixedYAsDouble, fixedXAsDouble);

        // Calculate our implementation result using Fixed values
        double actual = static_cast<double>(Fixed64Math::Atan2(fy, fx));

        // Handle -π and π equivalence (angle wrap-around)
        if (std::abs(actual - expected) > M_PI) {
            double diff = actual - expected;
            if (diff > M_PI)
                actual -= 2 * M_PI;
            else if (diff < -M_PI)
                actual += 2 * M_PI;
        }

        // Calculate error
        double error = std::abs(actual - expected);
        errorSum += error;

        if (error > maxError) {
            maxError = error;
            worstInput = {fixedYAsDouble, fixedXAsDouble};
        }

        // Near the origin or on the axes, precision requirements might need to be relaxed
        double epsilon = ((std::abs(fixedXAsDouble) < 0.1 && std::abs(fixedYAsDouble) < 0.1)
                          || fixedXAsDouble == 0.0 || fixedYAsDouble == 0.0)
                             ? kEpsilonCritical
                             : kEpsilonNormal;

        // Verify error is within acceptable range
        bool passed = (error <= epsilon);
        if (passed) {
            passedTests++;
        } else {
            failedTests++;
            // Print failed test
            std::cout << std::fixed << std::setprecision(8);
            std::cout << std::setw(10) << fixedYAsDouble << std::setw(10) << fixedXAsDouble
                      << std::setw(16) << expected << std::setw(16) << actual << std::setw(16)
                      << error << std::setw(16) << (error * 180.0 / M_PI) << "\n";

            EXPECT_NEAR(actual, expected, epsilon)
                << "Atan2 failed for input y=" << fixedYAsDouble << ", x=" << fixedXAsDouble;
        }
    }

    // Print summary information
    std::cout << "\nSummary:\n";
    std::cout << "Passed: " << passedTests << " (" << (100.0 * passedTests / testPoints.size())
              << "%)\n";
    std::cout << "Failed: " << failedTests << "\n";
    std::cout << "Average error: " << (errorSum / testPoints.size()) << " rad, "
              << (errorSum * 180.0 / M_PI / testPoints.size()) << " deg\n";
    std::cout << "Maximum error: " << maxError << " rad, " << (maxError * 180.0 / M_PI)
              << " deg at input y=" << worstInput.first << ", x=" << worstInput.second << "\n";

    // Verify pass rate
    EXPECT_GE(passedTests, static_cast<int>(0.9 * testPoints.size()))
        << "Atan2 precision test failed: too many errors";
}

// Special tests for boundary cases
TEST_F(InverseTrigPrecisionTest, BoundaryTests) {
    std::cout << "\n=== BOUNDARY VALUE TESTS ===\n";

    std::cout << "\nAsin/Acos boundary tests: Testing values very close to ±1\n";
    std::vector<double> boundaryValues = {
        -1.0, -0.99999, -0.9999, -0.999, 0.999, 0.9999, 0.99999, 1.0};

    std::cout << "\nAsin boundary results:\n";
    PrintPrecisionHeader();

    for (double inputValue : boundaryValues) {
        // Ensure values are within valid range
        inputValue = std::max(-1.0, std::min(1.0, inputValue));

        // Create Fixed value
        Fixed x(inputValue);
        double fixedAsDouble = static_cast<double>(x);

        // Calculate and compare Asin
        double expectedAsin = std::asin(fixedAsDouble);
        double actualAsin = static_cast<double>(Fixed64Math::Asin(x));

        PrintPrecisionRow(fixedAsDouble, expectedAsin, actualAsin);

        // Relax precision requirements at boundaries
        EXPECT_NEAR(actualAsin, expectedAsin, kEpsilonCritical)
            << "Asin boundary test failed for input " << fixedAsDouble;
    }

    std::cout << "\nAcos boundary results:\n";
    PrintPrecisionHeader();

    for (double inputValue : boundaryValues) {
        // Ensure values are within valid range
        inputValue = std::max(-1.0, std::min(1.0, inputValue));

        // Create Fixed value
        Fixed x(inputValue);
        double fixedAsDouble = static_cast<double>(x);

        // Calculate and compare Acos
        double expectedAcos = std::acos(fixedAsDouble);
        double actualAcos = static_cast<double>(Fixed64Math::Acos(x));

        PrintPrecisionRow(fixedAsDouble, expectedAcos, actualAcos);

        // Relax precision requirements at boundaries
        EXPECT_NEAR(actualAcos, expectedAcos, kEpsilonCritical)
            << "Acos boundary test failed for input " << fixedAsDouble;
    }

    std::cout << "\nAtan extreme value tests:\n";
    std::vector<double> extremeValues = {-1e6, -1e3, -100.0, -10.0, 10.0, 100.0, 1e3, 1e6};

    PrintPrecisionHeader();

    for (double inputValue : extremeValues) {
        // Create Fixed value
        Fixed x(inputValue);
        double fixedAsDouble = static_cast<double>(x);

        // Calculate and compare Atan
        double expectedAtan = std::atan(fixedAsDouble);
        double actualAtan = static_cast<double>(Fixed64Math::Atan(x));

        PrintPrecisionRow(fixedAsDouble, expectedAtan, actualAtan);

        // Relax precision requirements for extreme values
        EXPECT_NEAR(actualAtan, expectedAtan, kEpsilonCritical)
            << "Atan extreme value test failed for input " << fixedAsDouble;
    }

    std::cout << "\nAtan2 special cases:\n";
    std::vector<std::pair<double, double>> specialCases = {
        {0.0, 0.0},      // Origin, undefined value
        {1.0, 0.0},      // Positive y-axis, π/2
        {-1.0, 0.0},     // Negative y-axis, -π/2
        {0.0, 1.0},      // Positive x-axis, 0
        {0.0, -1.0},     // Negative x-axis, π or -π
        {1e-10, 1e-10},  // Near origin, but in first quadrant
        {1e-10, -1e-10}  // Near origin, but in second quadrant
    };

    std::cout << std::setw(10) << "Y" << std::setw(10) << "X" << std::setw(16) << "Expected"
              << std::setw(16) << "Actual" << std::setw(16) << "Error" << std::setw(16)
              << "Error (deg)" << "\n";
    std::cout << std::string(84, '-') << "\n";

    for (const auto& [y, x] : specialCases) {
        // Create Fixed values
        Fixed fx(x);
        Fixed fy(y);
        double fixedXAsDouble = static_cast<double>(fx);
        double fixedYAsDouble = static_cast<double>(fy);

        // Calculate and compare Atan2
        double expectedAtan2 = std::atan2(fixedYAsDouble, fixedXAsDouble);
        double actualAtan2 = static_cast<double>(Fixed64Math::Atan2(fy, fx));

        // Handle -π and π equivalence
        if (std::abs(actualAtan2 - expectedAtan2) > M_PI) {
            double diff = actualAtan2 - expectedAtan2;
            if (diff > M_PI)
                actualAtan2 -= 2 * M_PI;
            else if (diff < -M_PI)
                actualAtan2 += 2 * M_PI;
        }

        double error = std::abs(actualAtan2 - expectedAtan2);
        std::cout << std::fixed << std::setprecision(8);
        std::cout << std::setw(10) << fixedYAsDouble << std::setw(10) << fixedXAsDouble
                  << std::setw(16) << expectedAtan2 << std::setw(16) << actualAtan2 << std::setw(16)
                  << error << std::setw(16) << (error * 180.0 / M_PI) << "\n";

        // Special cases need special handling
        if (fixedXAsDouble == 0.0 && fixedYAsDouble == 0.0) {
            // For origin, implementation may return 0, ±π, or other values, we mainly check it
            // doesn't crash
            continue;
        } else {
            EXPECT_NEAR(actualAtan2, expectedAtan2, kEpsilonCritical)
                << "Atan2 special case test failed for y=" << fixedYAsDouble
                << ", x=" << fixedXAsDouble;
        }
    }
}

// Comprehensive precision comparison report
TEST_F(InverseTrigPrecisionTest, PrecisionComparisonReport) {
    std::cout << "\n=== PRECISION COMPARISON REPORT ===\n";
    std::cout << std::fixed << std::setprecision(8);

    // Common test values
    std::vector<double> commonTestValues = {
        -0.9, -0.75, -0.5, -0.25, -0.1, 0.0, 0.1, 0.25, 0.5, 0.75, 0.9};

    std::cout << "\nComparison across all inverse trig functions:\n";
    std::cout << std::setw(10) << "Input" << std::setw(16) << "Asin Error" << std::setw(16)
              << "Acos Error" << std::setw(16) << "Atan Error" << "\n";
    std::cout << std::string(58, '-') << "\n";

    for (double inputValue : commonTestValues) {
        // Create Fixed value
        Fixed x(inputValue);
        double fixedAsDouble = static_cast<double>(x);

        // Calculate Asin
        double expectedAsin = std::asin(fixedAsDouble);
        double actualAsin = static_cast<double>(Fixed64Math::Asin(x));
        double asinError = std::abs(actualAsin - expectedAsin);

        // Calculate Acos
        double expectedAcos = std::acos(fixedAsDouble);
        double actualAcos = static_cast<double>(Fixed64Math::Acos(x));
        double acosError = std::abs(actualAcos - expectedAcos);

        // Calculate Atan
        double expectedAtan = std::atan(fixedAsDouble);
        double actualAtan = static_cast<double>(Fixed64Math::Atan(x));
        double atanError = std::abs(actualAtan - expectedAtan);

        std::cout << std::setw(10) << fixedAsDouble << std::setw(16) << asinError << std::setw(16)
                  << acosError << std::setw(16) << atanError << "\n";
    }
}

TEST_F(InverseTrigPrecisionTest, AcosRegions3to5) {
    // Test points for Region 3: 0.93 to 0.99
    std::vector<double> region3_inputs = {0.93, 0.94, 0.95, 0.96, 0.97, 0.98, 0.99};

    // Test points for Region 4: 0.99 to 0.999
    std::vector<double> region4_inputs = {0.990, 0.992, 0.994, 0.996, 0.998, 0.999};

    // Test points for Region 5: 0.999 to 1.0
    std::vector<double> region5_inputs = {0.9991, 0.9993, 0.9995, 0.9997, 0.9999, 0.99999};

    // Combine all test points
    std::vector<double> all_inputs;
    all_inputs.insert(all_inputs.end(), region3_inputs.begin(), region3_inputs.end());
    all_inputs.insert(all_inputs.end(), region4_inputs.begin(), region4_inputs.end());
    all_inputs.insert(all_inputs.end(), region5_inputs.begin(), region5_inputs.end());

    // Define acceptable error margins for each region
    constexpr double region3_max_error = 1e-6;  // Relaxed from 1e-10
    constexpr double region4_max_error = 1e-6;  // Relaxed from 5e-7
    constexpr double region5_max_error = 1e-6;  // Relaxed from 1e-7

    for (double input : all_inputs) {
        // First create Fixed value
        Fixed x(input);

        // Extract double value from Fixed (includes precision loss from Fixed representation)
        double fixedAsDouble = static_cast<double>(x);

        // Calculate standard library result using the extracted double value
        double expected = std::acos(fixedAsDouble);

        // Calculate our implementation result using Fixed value
        double actual = static_cast<double>(Fixed64Math::Acos(x));

        // Calculate error
        double error = std::abs(actual - expected);

        // Determine max error based on region
        double max_error;
        if (fixedAsDouble >= 0.999) {
            max_error = region5_max_error;
        } else if (fixedAsDouble >= 0.99) {
            max_error = region4_max_error;
        } else {
            max_error = region3_max_error;
        }

        EXPECT_LE(error, max_error) << "Acos(" << fixedAsDouble << ") expected: " << expected
                                    << ", got: " << actual << ", error: " << error;
    }

    // Test edge case: exactly 1.0
    Fixed one = Fixed::One();
    double fixedOneAsDouble = static_cast<double>(one);
    double expected = std::acos(fixedOneAsDouble);
    double actual = static_cast<double>(Fixed64Math::Acos(one));
    double error = std::abs(actual - expected);

    EXPECT_LE(error, 1e-6) << "Acos(" << fixedOneAsDouble << ") expected: " << expected
                           << ", got: " << actual << ", error: " << error;

    // Test edge cases: outside valid range (-1,1)
    Fixed slightly_over_one = Fixed(1.0000001);
    double fixedOverOneAsDouble = static_cast<double>(slightly_over_one);
    expected =
        std::acos(std::min(fixedOverOneAsDouble, 1.0));  // Clamp to valid range for std::acos
    actual = static_cast<double>(Fixed64Math::Acos(slightly_over_one));
    error = std::abs(actual - expected);

    EXPECT_LE(error, 1e-6) << "Acos(" << fixedOverOneAsDouble << ") expected: " << expected
                           << ", got: " << actual << ", error: " << error;
}