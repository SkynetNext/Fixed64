# Fixed64: A High-Performance, Cross-Platform Fixed-Point Arithmetic Library

## Overview

Fixed64 is a fixed-point arithmetic library delivering high computational efficiency and consistency across platforms. Built on `int64_t`, it enables precise, deterministic mathematical operations ideal for applications requiring consistent results in various environments.

## Key Features

### Efficient Computational Model

Fixed64 uses `int64_t` for representing fixed-point numbers, ensuring fast and efficient operations by leveraging native integer arithmetic. This 64-bit base type provides both wide range and high precision.

### Cross-Platform Consistency

Through bit-precise implementation, Fixed64 ensures consistent results across different platforms - crucial for applications requiring deterministic outcomes. Unlike many fixed-point libraries that rely on simple multiplication/division for float/double conversions (which can introduce platform-specific variations), Fixed64 directly manipulates the bits according to IEEE 754 floating-point standard.

The bit-manipulation techniques in `Primitives.h` fundamentally solve cross-platform consistency issues by:
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

- **Basic Arithmetic**: Addition, subtraction, multiplication, division and their assignment variants
- **Comparison Operations**: Greater than, less than, equality, etc.
- **Trigonometric Functions**: Basic (`Sin`, `Cos`, `Tan`) and inverse (`Asin`, `Acos`, `Atan`, `Atan2`)
- **Exponential Functions**: `Exp`, `Pow2`
- **Rounding Operations**: `Floor`, `Ceil`, `Round`, `Trunc`
- **Value Manipulation**: `Abs`, `Min`, `Max`, `Clamp`, `Sign`
- **Interpolation Functions**: `Lerp`, `InverseLerp`, `LerpAngle`
- **Angle Utilities**: `NormalizeAngle`, `Repeat`

All mathematical functions are also available through the standard C++ `std` namespace.

### System Requirements

- C++20 or later (uses `std::bit_cast`)
- Tested with: Clang 10.0+, GCC 10.0+

### Template-Based Precision Control

Implemented as `Fixed64<P>` where P specifies fractional bits. Predefined types:
- `Fixed64_16`: 16 fractional bits (Q47.16)
- `Fixed64_32`: 32 fractional bits (Q31.32)
- `Fixed64_48`: 48 fractional bits (Q15.48)

### Header-Only Library

Consists solely of header files, simplifying integration and eliminating the need for linking with precompiled libraries.

### Predefined Constants and Special Values

Includes commonly used constants (`Zero()`, `One()`, `Pi()`) and special values (Infinity, NegInfinity, NaN) for handling exceptional cases.

## Usage Examples

### Basic Operations

```cpp
#include "Fixed64.h"

using math::fp::Fixed64_16;

int main() {
    Fixed64_16 a = Fixed64_16::Pi();
    Fixed64_16 b(1.2345);
    Fixed64_16 result = a * b;

    std::cout << "Result: " << result.ToString() << std::endl;
    return 0;
}
```

### Trigonometric Functions

```cpp
#include "Fixed64.h"
#include "Fixed64Math.h"

using math::fp::Fixed64_32;
using math::fp::Fixed64Math;

int main() {
    // Calculate trigonometric functions at π/4
    Fixed64_32 angle = Fixed64_32::Pi() / Fixed64_32(4);
    
    Fixed64_32 sinValue = Fixed64Math::Sin(angle);
    Fixed64_32 cosValue = Fixed64Math::Cos(angle);
    Fixed64_32 tanValue = Fixed64Math::Tan(angle);
    
    std::cout << "Sin(π/4): " << sinValue.ToString() << std::endl;
    std::cout << "Cos(π/4): " << cosValue.ToString() << std::endl;
    std::cout << "Tan(π/4): " << tanValue.ToString() << std::endl;
    
    // Angle conversion example
    Fixed64_32 degrees = Fixed64_32(45);
    Fixed64_32 radians = degrees * Fixed64_32::Deg2Rad();
    std::cout << "45° in radians: " << radians.ToString() << std::endl;
    
    return 0;
}
```

## Conclusion

Fixed64 meets the needs of developers requiring high-performance, cross-platform consistent fixed-point arithmetic. Its efficient computational model, comprehensive operation support, and predefined constants make it well-suited for applications demanding high precision and consistency.