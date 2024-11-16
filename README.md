# Fixed64: A High-Performance, Cross-Platform Fixed-Point Arithmetic Library

## Overview

Fixed64 is a comprehensive fixed-point arithmetic library designed to deliver high computational efficiency and consistency across different hardware and software platforms. Built on the solid foundation of the `int64_t` data type, Fixed64 enables precise mathematical operations with the added benefit of deterministic behavior, making it an ideal choice for applications requiring consistent results across various environments.

## Key Features

### Efficient Computational Model

At the heart of the Fixed64 library is its use of `int64_t` for representing fixed-point numbers. This choice ensures that operations are both fast and efficient, leveraging the native integer arithmetic capabilities of the underlying hardware. The use of a 64-bit integer as the base type allows for a wide range of values and high precision, making Fixed64 suitable for a variety of computational needs.

### Cross-Platform Consistency

Fixed64 ensures cross-platform consistency, especially through the `Fixed64::parseFloat` function. This function accurately converts floating-point numbers into Fixed64's fixed-point representation, ensuring consistent results across different platforms. This is crucial for applications requiring deterministic outcomes across various hardware and software environments.

### Comprehensive Operation Support

Fixed64 supports a wide array of mathematical operations, including:

- Basic arithmetic operations: addition (`+`), subtraction (`-`), multiplication (`*`), and division (`/`).
- Advanced mathematical functions: sine (`Sin`), cosine (`Cos`), tangent (`Tan`), exponential (`Exp`), and power (`Pow2`).
- Trigonometric functions and their inverse: including `Sin`, `Cos`, `Tan`, `Acos`, `Asin`, and `Atan`.
- Utility functions: square root (`Sqrt`), reciprocal (`Rcp`), and more.

### Predefined Constants

To further enhance usability and reduce construction costs, Fixed64 includes a collection of predefined constants within the `Fixed64Const` class. These constants represent commonly used values such as `Zero`, `One`, `Pi`, and many others, facilitating easy and efficient usage within applications.

### Infinity and NaN Handling

Fixed64 provides special constants for representing positive infinity, negative infinity, and Not a Number (NaN) scenarios. These constants allow for the handling of exceptional cases in mathematical operations, enhancing the robustness and reliability of applications using the library.

## Addendum: Header-Only Library

Fixed64 is a header-only library, meaning that it consists solely of header files without the need for separate source files or precompiled binaries. This design choice simplifies integration into projects, as developers only need to include the relevant header files in their codebase to use Fixed64. It eliminates the need for linking with precompiled libraries, making the library easily portable and usable across various platforms and compilers. Being a header-only library also facilitates ease of distribution and updating, as the entire library can be updated by replacing the header files.

## Usage Example

```cpp
#include "Fixed64.h"

using namespace Skynet;

int main() {
    Fixed64 a = Fixed64Const::Pi;
    Fixed64 b = Fixed64(1.2345);
    Fixed64 result = a * b;

    std::cout << "Result: " << result << std::endl;

    return 0;
}
```

This simple example demonstrates how to use Fixed64 to perform a multiplication operation and output the result.

```cpp
#include "Fixed64.h"

using namespace Skynet;

int main() {
    Fixed64 angle = Fixed64Const::PiOver2; // Approximately PI/2
    Fixed64 sinValue = FixedMath::Sin(angle);
    Fixed64 cosValue = FixedMath::Cos(angle);
    Fixed64 tanValue = FixedMath::Tan(angle);

    std::cout << "Sin: " << sinValue << std::endl;
    std::cout << "Cos: " << cosValue << std::endl;
    std::cout << "Tan: " << tanValue << std::endl;

    return 0;
}
```

This example demonstrates how to use Fixed64 to compute the sine, cosine, and tangent of an angle. These functions are crucial for applications involving trigonometric calculations, providing high precision and performance.

## Conclusion

Fixed64 is designed to meet the needs of developers looking for a high-performance, cross-platform consistent library for fixed-point arithmetic. With its efficient computational model, comprehensive operation support, and a wide range of predefined constants, Fixed64 is well-suited for applications demanding high precision and consistency.