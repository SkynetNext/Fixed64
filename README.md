# Fixed64: A High-Performance, Cross-Platform Fixed-Point Arithmetic Library

## Overview

Fixed64 is a comprehensive fixed-point arithmetic library designed to deliver high computational efficiency and consistency across different hardware and software platforms. Built on the solid foundation of the `int64_t` data type, Fixed64 enables precise mathematical operations with the added benefit of deterministic behavior, making it an ideal choice for applications requiring consistent results across various environments.

## Key Features

### Efficient Computational Model

At the heart of the Fixed64 library is its use of `int64_t` for representing fixed-point numbers. This choice ensures that operations are both fast and efficient, leveraging the native integer arithmetic capabilities of the underlying hardware. The use of a 64-bit integer as the base type allows for a wide range of values and high precision, making Fixed64 suitable for a variety of computational needs.

### Cross-Platform Consistency

Fixed64 ensures cross-platform consistency through its bit-precise implementation. The library accurately converts floating-point numbers into Fixed64's fixed-point representation, ensuring consistent results across different platforms. This is crucial for applications requiring deterministic outcomes across various hardware and software environments.

### Comprehensive Operation Support

Fixed64 supports a wide array of mathematical operations, including:

- Basic arithmetic operations and comparison: addition (`+`), subtraction (`-`), multiplication (`*`), division (`/`), addition assignment (`+=`), subtraction assignment (`-=`), multiplication assignment (`*=`), division assignment (`/=`), greater than (`>`), less than (`<`), greater than or equal to (`>=`), less than or equal to (`<=`), and equality (`==`).
- Advanced mathematical functions: sine (`Sin`), cosine (`Cos`), tangent (`Tan`), exponential (`Exp`), and power (`Pow2`).
- Trigonometric functions and their inverse: including `Sin`, `Cos`, `Tan`, `Acos`, `Asin`, and `Atan`.
- Utility functions: square root (`Sqrt`), and more.

### System Requirements

Fixed64 requires C++20 or later due to its use of modern C++ features such as `std::bit_cast`. The library has been tested with the following compilers:
- Clang 10.0 or later
- GCC 10.0 or later

### Template-Based Precision Control

Fixed64 is implemented as a template class `Fixed64<P>` where `P` specifies the number of fractional bits. This allows for flexible precision control based on application requirements. Common precision types are predefined:

- `Fixed64_16`: 16 fractional bits (Q47.16)
- `Fixed64_32`: 32 fractional bits (Q31.32)
- `Fixed64_48`: 48 fractional bits (Q15.48)

### Header-Only Library

Fixed64 is a header-only library, meaning that it consists solely of header files without the need for separate source files or precompiled binaries. This design choice simplifies integration into projects, as developers only need to include the relevant header files in their codebase to use Fixed64. It eliminates the need for linking with precompiled libraries, making the library easily portable and usable across various platforms and compilers.

### Predefined Constants

To further enhance usability and reduce construction costs, Fixed64 includes a collection of predefined constants accessible as static methods. These constants represent commonly used values such as `Zero()`, `One()`, `Pi()`, and many others, facilitating easy and efficient usage within applications.

### Infinity and NaN Handling

Fixed64 provides special constants for representing positive infinity, negative infinity, and Not a Number (NaN) scenarios. These constants allow for the handling of exceptional cases in mathematical operations, enhancing the robustness and reliability of applications using the library.

## Usage Example

```cpp
#include "Fixed64.h"

using math::fp::Fixed64;
using math::fp::Fixed64_16;

int main() {
    Fixed64<16> a = Fixed64<16>::Pi();
    Fixed64<16> b(1.2345);
    Fixed64<16> result = a * b;

    std::cout << "Result: " << result.ToString() << std::endl;

    return 0;
}
```

This simple example demonstrates how to use Fixed64 to perform a multiplication operation and output the result.

```cpp
#include "Fixed64.h"
#include "Fixed64Math.h"

using math::fp::Fixed64_32;
using math::fp::Fixed64Math;

int main() {
    Fixed64_32 angle = Fixed64_32::PiOver2(); // Approximately PI/2
    Fixed64_32 sinValue = Fixed64Math::Sin(angle);
    Fixed64_32 cosValue = Fixed64Math::Cos(angle);
    Fixed64_32 tanValue = Fixed64Math::Tan(angle);

    std::cout << "Sin: " << sinValue.ToString() << std::endl;
    std::cout << "Cos: " << cosValue.ToString() << std::endl;
    std::cout << "Tan: " << tanValue.ToString() << std::endl;

    return 0;
}
```

This example demonstrates how to use Fixed64 to compute the sine, cosine, and tangent of an angle. These functions are crucial for applications involving trigonometric calculations, providing high precision and performance.

## Advanced Example

```cpp
#include "Fixed64.h"
#include "Fixed64Math.h"

using math::fp::Fixed64_16;
using math::fp::Fixed64_32;
using math::fp::Fixed64Math;

int main() {
    // Define some angles (using radians)
    Fixed64_32 angleInRadians = Fixed64_32::Deg2Rad() * Fixed64_32(45); // Convert 45 degrees to radians

    // Calculate and output the values of sin, cos, tan
    Fixed64_32 sinValue = Fixed64Math::Sin(angleInRadians);
    Fixed64_32 cosValue = Fixed64Math::Cos(angleInRadians);
    Fixed64_32 tanValue = Fixed64Math::Tan(angleInRadians);

    std::cout << "Sin(45 degrees): " << sinValue.ToString() << std::endl;
    std::cout << "Cos(45 degrees): " << cosValue.ToString() << std::endl;
    std::cout << "Tan(45 degrees): " << tanValue.ToString() << std::endl;

    // Use atan2 to calculate an angle, and convert the result back to degrees
    Fixed64_32 y = Fixed64_32::One();
    Fixed64_32 x = Fixed64_32::One();
    Fixed64_32 atan2Value = Fixed64Math::Atan2(y, x);
    Fixed64_32 angleInDegrees = atan2Value * Fixed64_32::Rad2Deg();

    std::cout << "Atan2(1,1) in degrees: " << angleInDegrees.ToString() << std::endl;

    return 0;
}
```

## Conclusion

Fixed64 is designed to meet the needs of developers looking for a high-performance, cross-platform consistent library for fixed-point arithmetic. With its efficient computational model, comprehensive operation support, and a wide range of predefined constants, Fixed64 is well-suited for applications demanding high precision and consistency.