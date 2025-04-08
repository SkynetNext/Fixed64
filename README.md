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

Includes commonly used constants (`Zero()`, `One()`, `Pi()`) and special values (Infinity, NegInfinity, NaN) for handling exceptional cases.

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

| Operation      | Fixed64 (ms) | SoftDouble(ms) | Speedup vs SoftDouble | double (ms) | Speedup vs double |
|----------------|--------------|----------------|-----------------------|-------------|-------------------|
| Addition       | 7.11         | 86.96          | 12.24x                | 10.66       | 1.50x             |
| Subtraction    | 7.31         | 90.83          | 12.43x                | 10.57       | 1.45x             |
| Multiplication | 21.33        | 169.67         | 7.95x                 | 12.90       | 0.60x             |
| Division       | 136.03       | 217.31         | 1.60x                 | 12.80       | 0.09x             |
| Square Root    | 168.22       | 183.43         | 1.09x                 | 15.42       | 0.09x             |

These benchmarks show that:

- Fixed64 outperforms SoftDouble by significant margins on addition (12.2x) and subtraction (12.4x)
- Fixed64 multiplication is 8x faster than SoftDouble
- Fixed64 division and square root operations are moderately faster than SoftDouble
- Hardware floating-point (double) outperforms both software implementations for division and square root operations

### Advanced Mathematical Functions

| Function | Fixed64 (ms) | SoftDouble (ms) | Speedup vs SoftDouble | double (ms) | Speedup vs double |
|----------|--------------|-----------------|------------------------|-------------|-------------------|
| Pow2     | 102.79       | 2582.24         | 25.12x                 | 592.58      | 5.77x             |
| Sin      | 38.01        | 1867.90         | 49.14x                 | 100.15      | 2.63x             |
| Acos     | 63.56        | 2323.08         | 36.55x                 | 203.28      | 3.20x             |
| Exp      | 139.37       | 1691.18         | 12.13x                 | 43.51       | 0.31x             |
| Log      | 293.11       | 1894.10         | 6.46x                  | 46.90       | 0.16x             |
| Atan     | 99.16        | 1855.05         | 18.71x                 | 186.94      | 1.89x             |
| Atan2    | 282.52       | 2214.61         | 7.84x                  | 401.48      | 1.42x             |
| Pow      | 519.61       | 3729.06         | 7.18x                  | 204.63      | 0.39x             |

For advanced functions, Fixed64 demonstrates remarkable performance:

1. Fixed64 is dramatically faster than SoftDouble, with speedups ranging from 6.5x to 49.1x
2. Fixed64 outperforms even hardware double precision for many functions:
   - Pow2, Sin, Acos, Atan, and Atan2 are all faster in Fixed64 than using native hardware doubles
   - These advantages stem from optimized algorithms specifically designed for the fixed-point representation
3. For Exp, Log, and Pow, hardware floating-point remains faster, as expected for these complex functions

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