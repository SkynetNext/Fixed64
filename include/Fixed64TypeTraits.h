#pragma once

// Forward declaration of Fixed64 template
namespace math::fp {
template <int P>
class Fixed64;
}

#include <type_traits>

namespace math::fp::fixed64_traits {
// region Basic type detection
template <typename T>
struct is_fixed64_impl : std::false_type {};

// Specialized after including Fixed64.h
template <int P>
struct is_fixed64_impl<math::fp::Fixed64<P>> : std::true_type {};

template <typename T>
inline constexpr bool IsFixed64 = is_fixed64_impl<std::remove_cvref_t<T>>::value;
// endregion

// region Construction type classification
template <typename T>
inline constexpr bool IsConstructible = std::is_arithmetic_v<T>;

template <typename T>
inline constexpr bool IsCustomConstructible = !IsConstructible<T> && !IsFixed64<T>;
// endregion

// region Comparison type deduction
template <typename T1, typename T2>
struct CompareType;

// Handle Fixed64 with arithmetic types
template <int P, typename T>
    requires(std::is_arithmetic_v<T> && !IsFixed64<T>)
struct CompareType<math::fp::Fixed64<P>, T> {
    using type = math::fp::Fixed64<P>;
};

template <typename T, int P>
    requires(std::is_arithmetic_v<T> && !IsFixed64<T>)
struct CompareType<T, math::fp::Fixed64<P>> {
    using type = math::fp::Fixed64<P>;
};

// Handle comparisons between Fixed64 types
template <int P1, int P2>
struct CompareType<math::fp::Fixed64<P1>, math::fp::Fixed64<P2>> {
    using type = std::conditional_t<(P1 > P2), math::fp::Fixed64<P1>, math::fp::Fixed64<P2>>;
};

// Specialization for Fixed64 types with the same precision
template <int P>
struct CompareType<math::fp::Fixed64<P>, math::fp::Fixed64<P>> {
    using type = math::fp::Fixed64<P>;
};

// endregion
}  // namespace math::fp::fixed64_traits
