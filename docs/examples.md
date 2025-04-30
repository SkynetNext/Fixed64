# Usage Examples

## Basic Operations

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

## Mathematical Functions

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

## Trigonometric Functions

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

## Advanced Features

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