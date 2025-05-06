#pragma once

#include <stdlib.h>
#include <cstdint>
#include <random>
#include <vector>

#include "fixed64.h"
#include "fixed64_math.h"

namespace math::fp {

/**
 * @brief A deterministic random number generator for fixed-point numbers
 *
 * This class provides a deterministic random number generator that produces
 * fixed-point numbers with guaranteed cross-platform consistency. It uses
 * a combination of Mersenne Twister for high-quality random numbers and
 * custom algorithms for fixed-point number generation.
 *
 * Features:
 * - Deterministic random number generation
 * - Support for various fixed-point number types
 * - Weighted random selection
 * - Probability-based decision making
 * - Range-based random number generation
 */
class Fixed64Random {
 private:
    int32_t seed = 0;
    int32_t randomCount = 0;
    std::mt19937 rng;
    constexpr static Fixed64_16 INT32_MAX_PLUS_ONE = Fixed64_16(INT32_MAX) + Fixed64_16(1);

 public:
    /**
     * @brief Construct a new Fixed64Random object
     * @param seed Initial seed value (0 for random seed)
     */
    explicit Fixed64Random(int32_t seed = 0) noexcept {
        setSeed(seed);
    }

    /**
     * @brief Set the random number generator seed
     * @param seed New seed value (0 for random seed)
     */
    auto setSeed(int32_t seed = 0) noexcept -> void {
        randomCount = 0;
        if (seed == 0) {
            std::random_device rd;
            this->seed = rd() % (INT32_MAX);
        } else {
            this->seed = seed;
        }
        rng.seed(this->seed);
    }

    /**
     * @brief Get the current seed value
     * @return Current seed value
     */
    [[nodiscard]] auto getSeed() const noexcept -> int32_t {
        return seed;
    }

    /**
     * @brief Get the number of random numbers generated
     * @return Count of generated random numbers
     */
    [[nodiscard]] auto getRandomCount() const noexcept -> int32_t {
        return randomCount;
    }

    /**
     * @brief Generate a random fixed-point number in range [0, 1)
     * @return Random number in range [0, 1)
     */
    [[nodiscard]] auto random() noexcept -> Fixed64_16 {
        int32_t nextRandom = next();
        return Fixed64_16(nextRandom & INT32_MAX) / INT32_MAX_PLUS_ONE;
    }

    /**
     * @brief Generate a random fixed-point number in range [0, max)
     * @tparam T Fixed-point number type
     * @param max Upper bound (exclusive)
     * @return Random number in range [0, max)
     */
    template <typename T>
        requires detail::IsFixed64<T> || detail::IsConstructible<T>
    [[nodiscard]] auto random(const T& max) noexcept -> T {
        return static_cast<T>(random() * max);
    }

    /**
     * @brief Generate a random fixed-point number in range [min, max)
     * @tparam T1 Type of minimum value
     * @tparam T2 Type of maximum value
     * @param min Lower bound (inclusive)
     * @param max Upper bound (exclusive)
     * @return Random number in range [min, max)
     */
    template <typename T1, typename T2>
        requires(detail::IsFixed64<T1> || detail::IsConstructible<T1>)
                && (detail::IsFixed64<T2> || detail::IsConstructible<T2>)
    [[nodiscard]] auto random(const T1& min, const T2& max) noexcept -> T2 {
        return static_cast<T2>(random() * (max - min) + min);
    }

    /**
     * @brief Generate the next random integer using the internal algorithm
     * @return Next random integer
     */
    [[nodiscard]] auto next() noexcept -> int32_t {
        int32_t x = seed;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        seed = x;
        randomCount++;
        return x;
    }

    /**
     * @brief Generate a random integer in range [0, 100)
     * @return Random integer in range [0, 100)
     */
    [[nodiscard]] auto randomInteger() noexcept -> int32_t {
        return randomInteger(Fixed64_16(100));
    }

    /**
     * @brief Generate a random integer in range [0, max)
     * @tparam T Fixed-point number type
     * @param max Upper bound (exclusive)
     * @return Random integer in range [0, max)
     */
    template <typename T>
        requires detail::IsFixed64<T> || detail::IsConstructible<T>
    [[nodiscard]] auto randomInteger(const T& max) noexcept -> int32_t {
        return static_cast<int32_t>(random(max));
    }

    /**
     * @brief Generate a random integer in range [min, max)
     * @tparam T1 Type of minimum value
     * @tparam T2 Type of maximum value
     * @param min Lower bound (inclusive)
     * @param max Upper bound (exclusive)
     * @return Random integer in range [min, max)
     */
    template <typename T1, typename T2>
        requires(detail::IsFixed64<T1> || detail::IsConstructible<T1>)
                && (detail::IsFixed64<T2> || detail::IsConstructible<T2>)
    [[nodiscard]] auto randomInteger(const T1& min, const T2& max) noexcept -> int32_t {
        return static_cast<int32_t>(random(min, max));
    }

    /**
     * @brief Select a random index based on weights
     * @param weights Vector of weights for each index
     * @return Selected index, or -1 if weights are empty or invalid
     */
    [[nodiscard]] auto randomWeights(const std::vector<Fixed64_16>& weights) noexcept -> int {
        if (weights.empty()) {
            return -1;
        }

        // Calculate total weight and validate weights
        Fixed64_16 totalWeight(0);
        for (const auto& w : weights) {
            if (w < Fixed64_16::Zero()) {
                return -1;  // Invalid negative weight
            }
            totalWeight += w;
        }

        if (totalWeight <= Fixed64_16::Zero()) {
            return -1;  // Invalid total weight
        }

        // Generate random weight in range [0, totalWeight)
        Fixed64_16 randomWeight = random(Fixed64_16::Zero(), totalWeight);

        // Find the selected index
        Fixed64_16 accumulatedWeight(0);
        for (size_t index = 0; index < weights.size(); ++index) {
            accumulatedWeight += weights[index];
            if (randomWeight < accumulatedWeight) {
                return static_cast<int>(index);
            }
        }

        // This should never happen if weights are valid
        return static_cast<int>(weights.size() - 1);
    }

    /**
     * @brief Select a random element from an array
     * @tparam T Array element type
     * @param array Input array
     * @return Index of selected element, or -1 if array is empty
     */
    template <typename T>
    [[nodiscard]] auto randomArray(const std::vector<T>& array) noexcept -> int {
        if (array.empty()) {
            return -1;
        }
        return randomInteger(Fixed64_16(0), Fixed64_16(static_cast<int32_t>(array.size())));
    }

    /**
     * @brief Select a random element from an array based on weights
     * @tparam T Array element type
     * @param array Input array
     * @param weights Vector of weights for each element
     * @return Index of selected element, or -1 if arrays are empty or invalid
     */
    template <typename T>
    [[nodiscard]] auto randomArray(const std::vector<T>& array,
                                   const std::vector<Fixed64_16>& weights) noexcept -> int {
        if (array.empty() || weights.empty() || array.size() != weights.size()) {
            return -1;
        }
        return randomWeights(weights);
    }

    /**
     * @brief Make a random decision based on probability
     * @param probability Probability of returning true [0, 1]
     * @return True with given probability, false otherwise
     */
    [[nodiscard]] auto result01(const Fixed64_16& probability) noexcept -> bool {
        if (probability <= Fixed64_16::Zero()) {
            return false;
        }
        if (probability >= Fixed64_16::One()) {
            return true;
        }
        return random() < probability;
    }

    /**
     * @brief Make a random decision based on probability
     * @tparam T Fixed-point number type
     * @param probability Probability of returning true [0, 1]
     * @return True with given probability, false otherwise
     */
    template <typename T>
        requires detail::IsFixed64<T> || detail::IsConstructible<T>
    [[nodiscard]] auto result01(const T& probability) noexcept -> bool {
        return result01(static_cast<Fixed64_16>(probability));
    }

    /**
     * @brief Make a random decision based on percentage probability
     * @param probability Probability of returning true [0, 100]
     * @return True with given probability, false otherwise
     */
    [[nodiscard]] auto result(const Fixed64_16& probability) noexcept -> bool {
        if (probability <= Fixed64_16::Zero()) {
            return false;
        }
        if (probability >= Fixed64_16(100)) {
            return true;
        }
        return randomInteger() < probability;
    }

    /**
     * @brief Make a random decision based on percentage probability
     * @tparam T Fixed-point number type
     * @param probability Probability of returning true [0, 100]
     * @return True with given probability, false otherwise
     */
    template <typename T>
        requires detail::IsFixed64<T> || detail::IsConstructible<T>
    [[nodiscard]] auto result(const T& probability) noexcept -> bool {
        return result(static_cast<Fixed64_16>(probability));
    }

    /**
     * @brief Generate a random sign (-1 or 1)
     * @return -1 or 1 with equal probability
     */
    [[nodiscard]] auto randomBinarySign() noexcept -> int32_t {
        return random() < Fixed64_16::Half() ? -1 : 1;
    }
};
}  // namespace math::fp
