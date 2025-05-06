#include <gtest/gtest.h>
#include <algorithm>
#include <numeric>
#include <vector>

#include "fixed64_random.h"

using namespace math::fp;

class Fixed64RandomTest : public ::testing::Test {
 protected:
    void SetUp() override {
        // Set up common test data
        rng = std::make_unique<Fixed64Random>(42);  // Use fixed seed for deterministic tests
    }

    void TearDown() override {
        rng.reset();
    }

    std::unique_ptr<Fixed64Random> rng;
};

// Test basic random number generation
TEST_F(Fixed64RandomTest, BasicRandomGeneration) {
    // Test random() returns values in [0, 1)
    for (int i = 0; i < 1000; ++i) {
        auto value = rng->random();
        EXPECT_GE(value, Fixed64_16::Zero());
        EXPECT_LT(value, Fixed64_16::One());
    }

    // Test random(max) returns values in [0, max)
    Fixed64_16 max(10);
    for (int i = 0; i < 1000; ++i) {
        auto value = rng->random(max);
        EXPECT_GE(value, Fixed64_16::Zero());
        EXPECT_LT(value, max);
    }

    // Test random(min, max) returns values in [min, max)
    Fixed64_16 min(5);
    max = Fixed64_16(15);
    for (int i = 0; i < 1000; ++i) {
        auto value = rng->random(min, max);
        EXPECT_GE(value, min);
        EXPECT_LT(value, max);
    }
}

// Test random integer generation
TEST_F(Fixed64RandomTest, RandomIntegerGeneration) {
    // Test randomInteger() returns values in [0, 100)
    for (int i = 0; i < 1000; ++i) {
        auto value = rng->randomInteger();
        EXPECT_GE(value, 0);
        EXPECT_LT(value, 100);
    }

    // Test randomInteger(max) returns values in [0, max)
    int max = 10;
    for (int i = 0; i < 1000; ++i) {
        auto value = rng->randomInteger(Fixed64_16(max));
        EXPECT_GE(value, 0);
        EXPECT_LT(value, max);
    }

    // Test randomInteger(min, max) returns values in [min, max)
    int min = 5;
    max = 15;
    for (int i = 0; i < 1000; ++i) {
        auto value = rng->randomInteger(Fixed64_16(min), Fixed64_16(max));
        EXPECT_GE(value, min);
        EXPECT_LT(value, max);
    }
}

// Test weighted random selection
TEST_F(Fixed64RandomTest, WeightedRandomSelection) {
    // Test with valid weights
    std::vector<Fixed64_16> weights = {
        Fixed64_16(1),  // 10%
        Fixed64_16(2),  // 20%
        Fixed64_16(3),  // 30%
        Fixed64_16(4)   // 40%
    };

    std::vector<int> counts(4, 0);
    const int numTrials = 10000;

    for (int i = 0; i < numTrials; ++i) {
        int index = rng->randomWeights(weights);
        EXPECT_GE(index, 0);
        EXPECT_LT(index, 4);
        counts[index]++;
    }

    // Check if the distribution roughly matches the weights
    double totalWeight =
        std::accumulate(weights.begin(), weights.end(), Fixed64_16::Zero()).value();
    for (size_t i = 0; i < weights.size(); ++i) {
        double expected = static_cast<double>(weights[i].value()) / totalWeight;
        double actual = static_cast<double>(counts[i]) / numTrials;
        EXPECT_NEAR(actual, expected, 0.05);  // Allow 5% deviation
    }

    // Test with empty weights
    std::vector<Fixed64_16> emptyWeights;
    EXPECT_EQ(rng->randomWeights(emptyWeights), -1);

    // Test with negative weights
    std::vector<Fixed64_16> negativeWeights = {Fixed64_16(1), Fixed64_16(-1), Fixed64_16(2)};
    EXPECT_EQ(rng->randomWeights(negativeWeights), -1);

    // Test with zero weights
    std::vector<Fixed64_16> zeroWeights = {Fixed64_16(0), Fixed64_16(0), Fixed64_16(0)};
    EXPECT_EQ(rng->randomWeights(zeroWeights), -1);
}

// Test array random selection
TEST_F(Fixed64RandomTest, ArrayRandomSelection) {
    // Test with valid array
    std::vector<int> array = {1, 2, 3, 4, 5};
    std::vector<int> counts(array.size(), 0);
    const int numTrials = 10000;

    for (int i = 0; i < numTrials; ++i) {
        int index = rng->randomArray(array);
        EXPECT_GE(index, 0);
        EXPECT_LT(index, static_cast<int>(array.size()));
        counts[index]++;
    }

    // Check if the distribution is roughly uniform
    double expected = 1.0 / array.size();
    for (int count : counts) {
        double actual = static_cast<double>(count) / numTrials;
        EXPECT_NEAR(actual, expected, 0.05);  // Allow 5% deviation
    }

    // Test with empty array
    std::vector<int> emptyArray;
    EXPECT_EQ(rng->randomArray(emptyArray), -1);

    // Test with weighted array
    std::vector<int> array2 = {1, 2, 3, 4};  // 4个元素
    std::vector<Fixed64_16> weights = {
        Fixed64_16(1),  // 10%
        Fixed64_16(2),  // 20%
        Fixed64_16(3),  // 30%
        Fixed64_16(4)   // 40%
    };

    counts.assign(array2.size(), 0);
    for (int i = 0; i < numTrials; ++i) {
        int index = rng->randomArray(array2, weights);
        EXPECT_GE(index, 0);
        EXPECT_LT(index, static_cast<int>(array2.size()));
        counts[index]++;
    }

    // Check if the distribution roughly matches the weights
    double totalWeight =
        std::accumulate(weights.begin(), weights.end(), Fixed64_16::Zero()).value();
    for (size_t i = 0; i < weights.size(); ++i) {
        double expected = static_cast<double>(weights[i].value()) / totalWeight;
        double actual = static_cast<double>(counts[i]) / numTrials;
        EXPECT_NEAR(actual, expected, 0.05);  // Allow 5% deviation
    }

    // Test with mismatched array and weights
    std::vector<Fixed64_16> mismatchedWeights = {Fixed64_16(1), Fixed64_16(2)};
    EXPECT_EQ(rng->randomArray(array2, mismatchedWeights), -1);
}

// Test probability-based decision making
TEST_F(Fixed64RandomTest, ProbabilityBasedDecisions) {
    // Test result01 with various probabilities
    const int numTrials = 10000;

    // Test with probability 0
    for (int i = 0; i < numTrials; ++i) {
        EXPECT_FALSE(rng->result01(Fixed64_16::Zero()));
    }

    // Test with probability 1
    for (int i = 0; i < numTrials; ++i) {
        EXPECT_TRUE(rng->result01(Fixed64_16::One()));
    }

    // Test with probability 0.5
    int trueCount = 0;
    for (int i = 0; i < numTrials; ++i) {
        if (rng->result01(Fixed64_16::Half())) {
            trueCount++;
        }
    }
    double actual = static_cast<double>(trueCount) / numTrials;
    EXPECT_NEAR(actual, 0.5, 0.05);  // Allow 5% deviation

    // Test result with percentage probabilities
    // Test with 0%
    for (int i = 0; i < numTrials; ++i) {
        EXPECT_FALSE(rng->result(Fixed64_16::Zero()));
    }

    // Test with 100%
    for (int i = 0; i < numTrials; ++i) {
        EXPECT_TRUE(rng->result(Fixed64_16(100)));
    }

    // Test with 50%
    trueCount = 0;
    for (int i = 0; i < numTrials; ++i) {
        if (rng->result(Fixed64_16(50))) {
            trueCount++;
        }
    }
    actual = static_cast<double>(trueCount) / numTrials;
    EXPECT_NEAR(actual, 0.5, 0.05);  // Allow 5% deviation
}

// Test random sign generation
TEST_F(Fixed64RandomTest, RandomSignGeneration) {
    const int numTrials = 10000;
    int positiveCount = 0;

    for (int i = 0; i < numTrials; ++i) {
        int sign = rng->randomBinarySign();
        EXPECT_TRUE(sign == 1 || sign == -1);
        if (sign == 1) {
            positiveCount++;
        }
    }

    // Check if the distribution is roughly 50/50
    double actual = static_cast<double>(positiveCount) / numTrials;
    EXPECT_NEAR(actual, 0.5, 0.05);  // Allow 5% deviation
}

// Test seed management
TEST_F(Fixed64RandomTest, SeedManagement) {
    // Test initial seed
    EXPECT_EQ(rng->getSeed(), 42);

    // Test seed change
    rng->setSeed(123);
    EXPECT_EQ(rng->getSeed(), 123);

    // Test random count
    EXPECT_EQ(rng->getRandomCount(), 0);
    auto r = rng->random();
    EXPECT_EQ(rng->getRandomCount(), 1);

    // Test random seed generation
    rng->setSeed(0);  // Should generate a random seed
    EXPECT_NE(rng->getSeed(), 0);
    EXPECT_GE(rng->getSeed(), 0);
    EXPECT_LT(rng->getSeed(), INT32_MAX);
}

// Test deterministic behavior with same seed
TEST_F(Fixed64RandomTest, DeterministicBehavior) {
    Fixed64Random rng1(42);
    Fixed64Random rng2(42);

    // Generate sequences and compare
    for (int i = 0; i < 1000; ++i) {
        EXPECT_EQ(rng1.random(), rng2.random());
        EXPECT_EQ(rng1.randomInteger(), rng2.randomInteger());
        EXPECT_EQ(rng1.randomBinarySign(), rng2.randomBinarySign());
    }
}

// Test different fixed-point precisions
TEST_F(Fixed64RandomTest, DifferentPrecisions) {
    // Test with Fixed64_32
    for (int i = 0; i < 1000; ++i) {
        auto value = rng->random(Fixed64_32(10));
        EXPECT_GE(value, Fixed64_32::Zero());
        EXPECT_LT(value, Fixed64_32(10));
    }

    // Test with Fixed64_40
    for (int i = 0; i < 1000; ++i) {
        auto value = rng->random(Fixed64_40(10));
        EXPECT_GE(value, Fixed64_40::Zero());
        EXPECT_LT(value, Fixed64_40(10));
    }
}