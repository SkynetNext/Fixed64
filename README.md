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

Implemented as `Fixed64<P>` where P specifies fractional bits. Predefined types:
- `Fixed64_16`: 16 fractional bits (Q47.16)
- `Fixed64_32`: 32 fractional bits (Q31.32)
- `Fixed64_40`: 40 fractional bits (Q23.40)

### Header-Only Library

Consists solely of header files, simplifying integration and eliminating the need for linking with precompiled libraries.

### Predefined Constants and Special Values

Includes commonly used constants (`Zero()`, `One()`, `Pi()`) and special values (Infinity, NegInfinity, NaN) for handling exceptional cases.

## Usage Examples

### Basic Operations

```cpp
#include "Fixed64.h"

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
#include "Fixed64.h"
#include "Fixed64Math.h"

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
#include "Fixed64.h"
#include "Fixed64Math.h"

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
#include "Fixed64.h"
#include "Fixed64Math.h"
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

## Conclusion

Fixed64 meets the needs of developers requiring high-performance, cross-platform consistent fixed-point arithmetic. Its efficient computational model, comprehensive operation support, and predefined constants make it well-suited for applications demanding high precision and consistency.