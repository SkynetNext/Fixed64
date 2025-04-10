# Fixed64: A High-Performance, Cross-Platform Fixed-Point Arithmetic Library

## Overview

Fixed64 is a fixed-point arithmetic library delivering high computational efficiency and consistency across platforms. Built on `int64_t`, it enables precise, deterministic mathematical operations ideal for applications requiring consistent results in various environments.

## Key Features

### Efficient Computational Model

Fixed64 uses `int64_t` for representing fixed-point numbers, ensuring fast and efficient operations by leveraging native integer arithmetic. This 64-bit base type provides both wide range and high precision.

### Cross-Platform Consistency

Through bit-precise implementation, Fixed64 ensures consistent results across different platforms - crucial for applications requiring deterministic outcomes. Unlike many fixed-point libraries that rely on simple multiplication/division for float/double conversions (which can introduce platform-specific variations), Fixed64 directly manipulates the bits according to IEEE 754 floating-point standard.

The bit-manipulation techniques in `primitives.h` fundamentally solve cross-platform consistency issues by:
- Performing deterministic bit-level operations for all conversions
- Precisely controlling rounding behavior
- Eliminating compiler and hardware-specific floating-point optimizations
- Ensuring identical binary representation of values across any platform

This approach guarantees bit-exact matching results between different operating systems, compilers, and CPU architectures.

### Performance Optimizations

Fixed64 is a C++ port of [FixedMath.Net](https://github.com/asik/FixedMath.Net), with significant performance improvements:

- **Optimized Arithmetic**: Multiplication and division operations use industry-best practices from GCC, ARM CMSIS-DSP, and SoftFloat libraries
- **Efficient Square Root**: Optimized for both accuracy and speed
- **Performance Gains**: 3-5x faster than the original C# implementation

### Comprehensive Operation Support

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

All mathematical functions are also available through the standard C++ `std` namespace.

### System Requirements

- C++20 or later (uses `std::bit_cast`)
- Tested with: Clang 10.0+, GCC 10.0+

### Template-Based Precision Control

Fixed64 is implemented as a template class `Fixed64<P>` where `P` specifies the number of fractional bits. This design allows you to optimize for either range or precision based on your application's needs:

| Type | Format | Integer Bits | Fractional Bits | Range | Precision | √ Max |
|------|--------|--------------|----------------|-------|-----------|-------|
| `Fixed64_16` | Q47.16 | 47 | 16 | ±1.4×10¹⁴ | 1.5×10⁻⁵ | ±1.2×10⁷ |
| `Fixed64_32` | Q31.32 | 31 | 32 | ±2.1×10⁹ | 2.3×10⁻¹⁰ | ±4.6×10⁴ |
| `Fixed64_40` | Q23.40 | 23 | 40 | ±8.3×10⁶ | 9.1×10⁻¹³ | ±2.9×10³ |

### Header-Only Library

Consists solely of header files, simplifying integration and eliminating the need for linking with precompiled libraries.

### Predefined Constants and Special Values

Includes commonly used constants (`Zero()`, `One()`, `Pi()`) and special values (Infinity(), -Infinity(), NaN()) for handling exceptional cases.

## Usage Examples

### Basic Operations

```cpp
#include "fixed64.h"

using math::fp::Fixed64<16>;

int main() {
    // Create fixed-point numbers
    Fixed64<16> a(5.5);
    Fixed64<16> b(2.25);
    
    // Basic arithmetic
    Fixed64<16> sum = a + b;        // 7.75
    Fixed64<16> diff = a - b;       // 3.25
    Fixed64<16> product = a * b;    // 12.375
    Fixed64<16> quotient = a / b;   // 2.444...
    
    // Comparison
    bool isGreater = (a > b);       // true
    
    // Constants
    Fixed64<16> pi = Fixed64<16>::Pi();
    
    std::cout << "Result: " << quotient << std::endl;
    return 0;
}
```

### Mathematical Functions

```cpp
#include "fixed64.h"
#include "fixed64_math.h"

using Fixed = math::fp::Fixed64<32>;
using math::fp::Fixed64Math;

int main() {
    Fixed value(3.75);
    
    // Rounding operations
    Fixed floor = Fixed64Math::Floor(value);    // 3.0
    Fixed ceil = Fixed64Math::Ceil(value);      // 4.0
    Fixed round = Fixed64Math::Round(value);    // 4.0
    
    // Min/Max and clamping
    Fixed min = Fixed64Math::Min(Fixed(1.5), Fixed(0.5));  // 0.5
    Fixed clamp = Fixed64Math::Clamp(value, Fixed(2.0), Fixed(3.5)); // 3.5
    
    // Square root and exponential
    Fixed sqrt = Fixed64Math::Sqrt(Fixed(16.0));  // 4.0
    
    std::cout << "Square root of 16: " << sqrt << std::endl;
    return 0;
}
```

### Trigonometric Functions

```cpp
#include "fixed64.h"
#include "fixed64_math.h"

using Fixed = math::fp::Fixed64<32>;
using math::fp::Fixed64Math;

int main() {
    // Calculate trigonometric functions at π/4
    Fixed angle = Fixed::Pi() / Fixed(4);
    
    Fixed sinValue = Fixed64Math::Sin(angle);   // ~0.7071
    Fixed cosValue = Fixed64Math::Cos(angle);   // ~0.7071
    Fixed tanValue = Fixed64Math::Tan(angle);   // ~1.0
    
    // Inverse trigonometric function
    Fixed atan2Value = Fixed64Math::Atan2(Fixed(1.0), Fixed(1.0)); // π/4
    
    // Angle conversion
    Fixed degrees = Fixed(45);
    Fixed radians = degrees * Fixed::Pi() / Fixed(180);
    
    std::cout << "Sin(π/4): " << sinValue << std::endl;
    return 0;
}
```

### Advanced Features

```cpp
#include "fixed64.h"
#include "fixed64_math.h"
#include <limits>

using Fixed16 = math::fp::Fixed64<16>;
using Fixed32 = math::fp::Fixed64<32>;
using math::fp::Fixed64Math;

int main() {
    // Interpolation
    Fixed16 lerp = Fixed64Math::Lerp(Fixed16(2), Fixed16(3), Fixed16(0.5));  // 2.5
    
    // Precision conversion
    Fixed32 highPrecision(3.14159265359);
    Fixed16 mediumPrecision(highPrecision);  // Converts precision
    
    // Standard library integration
    Fixed16 value(-5.5);
    Fixed16 abs = std::abs(value);      // 5.5
    
    // Special values
    Fixed16 inf = Fixed16::Infinity();
    bool isInf = std::isinf(inf);       // true
    
    std::cout << "Interpolated value: " << lerp << std::endl;
    return 0;
}
```

## Performance Benchmarks

Comprehensive benchmarks comparing Fixed64 with [soft_double](https://github.com/ckormanyos/soft_double.git) (software floating point) and hardware floating point (double) reveal the following performance characteristics. The soft_double library is a modern C++ header-only implementation that provides the same functionality as [Berkeley SoftFloat](https://github.com/ucb-bar/berkeley-softfloat-3.git).

### Basic Arithmetic Operations

| Operation      | Fixed64 (ms) | SoftDouble(ms) | Speedup vs SoftDouble | double (ms) | Speedup vs double | int64 (ms) | Speedup vs int64 |
|----------------|--------------|----------------|-----------------------|-------------|-------------------|------------|------------------|
| Addition       | 7.52         | 104.17         | 13.84x                | 16.64       | 2.21x             | 3.79       | 0.50x            |
| Subtraction    | 4.16         | 101.51         | 24.39x                | 16.02       | 3.85x             | 4.00       | 0.96x            |
| Multiplication | 20.40        | 173.04         | 8.48x                 | 12.67       | 0.62x             | 10.73      | 0.53x            |
| Division       | 212.34       | 299.60         | 1.41x                 | 14.83       | 0.07x             | 84.49      | 0.40x            |
| Square Root    | 202.36       | 229.31         | 1.13x                 | 20.57       | 0.10x             | N/A        | N/A              |

These benchmarks show that:

- Fixed64 outperforms SoftDouble by significant margins on addition (13.8x) and subtraction (24.4x)
- Fixed64 multiplication is 8.5x faster than SoftDouble
- Fixed64 division and square root operations are moderately faster than SoftDouble
- Hardware floating-point (double) outperforms both software implementations for division and square root operations
- Native integer operations are faster for basic arithmetic, as expected

### Advanced Mathematical Functions

| Function | Fixed64 (ms) | SoftDouble (ms) | Speedup vs SoftDouble | double (ms) | Speedup vs double |
|----------|--------------|-----------------|------------------------|-------------|-------------------|
| Pow2     | 163.31       | 3341.61         | 20.46x                 | 537.44      | 3.29x             |
| Exp      | 327.96       | 2068.18         | 6.31x                  | 55.38       | 0.17x             |
| Log      | 463.26       | 2439.76         | 5.27x                  | 59.23       | 0.13x             |
| Pow      | 879.24       | 4739.61         | 5.39x                  | 327.86      | 0.37x             |
| Atan2    | 322.15       | N/A             | N/A                    | 302.75      | 0.94x            |

### Trigonometric Functions (Standard vs Fast Implementation)

| Function | Implementation | Fixed64 (ms) | SoftDouble (ms) | Speedup vs SoftDouble | double (ms) | Speedup vs double |
|----------|---------------|--------------|-----------------|------------------------|-------------|-------------------|
| Sin      | Standard      | 284.61       | 2376.13         | 8.35x                  | 115.00      | 0.40x             |
|          | Fast          | 156.91       | 2538.54         | 16.18x                 | 121.75      | 0.78x             |
| Tan      | Standard      | 319.60       | 4854.96         | 15.19x                 | 157.15      | 0.49x             |
|          | Fast          | 131.60       | 4972.78         | 37.79x                 | 141.64      | 1.08x             |
| Acos     | Standard      | 157.84       | 2834.70         | 17.96x                 | 141.77      | 0.90x             |
|          | Fast          | 69.03        | 2961.86         | 42.91x                 | 141.89      | 2.06x             |
| Atan     | Standard      | 85.25        | 2525.56         | 29.62x                 | 186.05      | 2.18x             |
|          | Fast          | 53.34        | 2852.51         | 53.47x                 | 215.26      | 4.04x             |

For advanced functions, Fixed64 demonstrates remarkable performance:

1. Fixed64 is dramatically faster than SoftDouble, with speedups ranging from 5.3x to 53.5x
2. Fixed64 outperforms even hardware double precision for several functions:
   - Pow2 is 3.3x faster than hardware double
   - Fast implementations of Acos, Atan, and Tan are faster than hardware doubles
   - These advantages stem from optimized algorithms specifically designed for the fixed-point representation
3. For Exp, Log, and Pow, hardware floating-point remains faster, as expected for these complex functions
4. The fast implementations of trigonometric functions provide significant performance improvements over the standard versions

### Hardware Floating Point Comparison

When hardware floating point is available:

- Basic arithmetic operations (especially division and square root) are faster with hardware floating point
- However, Fixed64 maintains deterministic cross-platform behavior that hardware floating point cannot guarantee
- For many advanced functions, Fixed64 implementations are 1.5x-5.8x faster than equivalent hardware floating point operations
- Fixed64 addition and subtraction operations are also faster than hardware floating-point operations

### Conclusion

Fixed64 offers excellent performance for deterministic cross-platform arithmetic. The benchmarks highlight its particular strengths:

1. Significant performance advantage over software floating-point alternatives (SoftDouble)
2. Similar or better performance than hardware floating-point for many common operations
3. Exceptional performance for transcendental functions compared to both software and hardware alternatives
4. Guaranteed bit-exact results across different platforms and architectures

This makes Fixed64 an excellent choice for applications where both performance and deterministic cross-platform results are required.