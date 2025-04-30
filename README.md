# Fixed64: A High-Performance, Cross-Platform Fixed-Point Arithmetic Library

## Overview

Fixed64 is a fixed-point arithmetic library delivering high computational efficiency and consistency across platforms. Built on `int64_t`, it enables precise, deterministic mathematical operations ideal for applications requiring consistent results in various environments.

## Key Features

- **High Performance**: 3-5x faster than the original C# implementation
- **Cross-Platform Consistency**: Bit-exact results across different platforms
- **Comprehensive Math Support**: Full range of arithmetic, trigonometric, and mathematical functions
- **Template-Based Precision**: Choose between range and precision with different bit configurations
- **Header-Only**: Easy integration with no linking required

## Quick Start

```cpp
#include "fixed64.h"

using math::fp::Fixed64<32>;  // 32-bit fractional precision

int main() {
    Fixed64<32> a(5.5);
    Fixed64<32> b(2.25);
    
    Fixed64<32> sum = a + b;        // 7.75
    Fixed64<32> product = a * b;    // 12.375
    
    std::cout << "Result: " << sum << std::endl;
    return 0;
}
```

## Documentation

- [Features](docs/features.md) - Detailed feature descriptions
- [Examples](docs/examples.md) - Usage examples and code snippets
- [Performance](docs/performance.md) - Performance benchmarks and analysis

## System Requirements

- C++20 or later (uses `std::bit_cast`)
- Tested with: Clang 10.0+, GCC 10.0+
