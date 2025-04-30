# Features

## Efficient Computational Model

Fixed64 uses `int64_t` for representing fixed-point numbers, ensuring fast and efficient operations by leveraging native integer arithmetic. This 64-bit base type provides both wide range and high precision.

## Cross-Platform Consistency

Through bit-precise implementation, Fixed64 ensures consistent results across different platforms - crucial for applications requiring deterministic outcomes. Unlike many fixed-point libraries that rely on simple multiplication/division for float/double conversions (which can introduce platform-specific variations), Fixed64 directly manipulates the bits according to IEEE 754 floating-point standard.

The bit-manipulation techniques in `primitives.h` fundamentally solve cross-platform consistency issues by:
- Performing deterministic bit-level operations for all conversions
- Precisely controlling rounding behavior
- Eliminating compiler and hardware-specific floating-point optimizations
- Ensuring identical binary representation of values across any platform

## Performance Optimizations

Fixed64 is a C++ port of [FixedMath.Net](https://github.com/asik/FixedMath.Net), with significant performance improvements:

- **Optimized Arithmetic**: Multiplication and division operations use industry-best practices from GCC, ARM CMSIS-DSP, and SoftFloat libraries
- **Efficient Square Root**: Optimized for both accuracy and speed
- **Performance Gains**: 3-5x faster than the original C# implementation

## Comprehensive Operation Support

Fixed64 supports a wide array of mathematical operations:

- **Basic Arithmetic**: Addition (`+`), subtraction (`-`), multiplication (`*`), division (`/`) and their assignment variants (`+=`, `-=`, `*=`, `/=`)
- **Comparison Operations**: Greater than (`>`), less than (`<`), equality (`==`), etc.
- **Trigonometric Functions**: Basic (`Sin`, `Cos`, `Tan`) and inverse (`Asin`, `Acos`, `Atan`, `Atan2`)
- **Logarithmic Functions**: Natural logarithm (`Log`)
- **Exponential Functions**: `Exp`, `Pow`, `Pow2`
- **Rounding Operations**: `Floor`, `Ceil`, `Round`, `Trunc`
- **Value Manipulation**: `Abs`, `Min`, `Max`, `Clamp`, `Clamp01`, `Sign`, `IsNearlyEqual`
- **Interpolation Functions**: `Lerp`, `LerpUnclamped`, `InverseLerp`, `LerpAngle`
- **Angle Utilities**: `NormalizeAngle`, `Repeat`
- **Fractional Operations**: `Fractions` (extract fractional part)

## Template-Based Precision Control

Fixed64 is implemented as a template class `Fixed64<P>` where `P` specifies the number of fractional bits. This design allows you to optimize for either range or precision based on your application's needs:

| Type | Format | Integer Bits | Fractional Bits | Range | Precision | √ Max |
|------|--------|--------------|----------------|-------|-----------|-------|
| `Fixed64_16` | Q47.16 | 47 | 16 | ±1.4×10¹⁴ | 1.5×10⁻⁵ | ±1.2×10⁷ |
| `Fixed64_32` | Q31.32 | 31 | 32 | ±2.1×10⁹ | 2.3×10⁻¹⁰ | ±4.6×10⁴ |
| `Fixed64_40` | Q23.40 | 23 | 40 | ±8.3×10⁶ | 9.1×10⁻¹³ | ±2.9×10³ |

## System Requirements

- C++20 or later (uses `std::bit_cast`)
- Tested with: Clang 10.0+, GCC 10.0+

## Header-Only Library

Consists solely of header files, simplifying integration and eliminating the need for linking with precompiled libraries.

## Predefined Constants and Special Values

Includes commonly used constants (`Zero()`, `One()`, `Pi()`) and special values (Infinity, NegInfinity, NaN) for handling exceptional cases. 