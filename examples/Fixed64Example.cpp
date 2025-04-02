#include <iostream>
#include "Fixed64.h"
#include "Fixed64Math.h"

// Option 1: Import the entire namespace (not recommended for large namespaces)
// using namespace math::fp;

// Option 2: Import specific template class (preferred)
using math::fp::Fixed64;

// Option 3: Import specific specializations
using math::fp::Fixed64_16;
using math::fp::Fixed64_32;
using math::fp::Fixed64Math;

int main() {
    // Create some Fixed64<16> numbers - now works correctly
    Fixed64<16> num1(3.75);
    Fixed64<16> num2(1.25);
    Fixed64<16> num3 = num1 + num2;

    // Alternatively, use the type alias
    Fixed64_16 num4(5.5);

    // Using constants
    math::fp::Fixed64<16> one = math::fp::Fixed64<16>::One();  // Represents 1.0
    std::cout << "One: " << one.ToString() << std::endl;

    // Addition
    math::fp::Fixed64<16> sum = num1 + num2;
    std::cout << "Sum: " << sum.ToString() << std::endl;  // Should print 5.0

    // Using the += operator
    sum += one;
    std::cout << "Sum after += One: " << sum.ToString() << std::endl;  // Should print 6.0

    // Subtraction
    math::fp::Fixed64<16> difference = num1 - num2;
    std::cout << "Difference: " << difference.ToString() << std::endl;  // Should print 2.5

    // Using the -= operator
    difference -= one;
    std::cout << "Difference after -= One: " << difference.ToString()
              << std::endl;  // Should print 1.5

    // Multiplication
    math::fp::Fixed64<16> product = num1 * num2;
    std::cout << "Product: " << product.ToString() << std::endl;  // Should print 4.6875

    // Using the *= operator
    product *= one;
    std::cout << "Product after *= One: " << product.ToString()
              << std::endl;  // Should print 4.6875

    // Division
    math::fp::Fixed64<16> quotient = num1 / num2;
    std::cout << "Quotient: " << quotient.ToString() << std::endl;  // Should print 3.0

    // Using the /= operator
    quotient /= one;
    std::cout << "Quotient after /= One: " << quotient.ToString() << std::endl;  // Should print 3.0

    // Handling division by zero
    math::fp::Fixed64<16> zero(0);
    math::fp::Fixed64<16> divisionByZero = num1 / zero;
    if (divisionByZero == math::fp::Fixed64<16>::Infinity()) {
        std::cout << "Division by zero results in Infinity" << std::endl;
    } else if (divisionByZero == math::fp::Fixed64<16>::NegInfinity()) {
        std::cout << "Division by zero results in NegInfinity" << std::endl;
    }

    // Add some math function examples
    std::cout << "\n--- Math Functions ---" << std::endl;
    math::fp::Fixed64<16> angle = math::fp::Fixed64<16>::Pi() / math::fp::Fixed64<16>(4);  // π/4
    std::cout << "Angle (π/4): " << angle.ToString() << std::endl;

    // Use Fixed64Math for trigonometric calculations
    Fixed64_32 angle32(angle);  // Convert to 32-bit precision for trigonometric functions
    std::cout << "Sin(π/4): " << Fixed64Math::Sin(angle32).ToString() << std::endl;
    std::cout << "Cos(π/4): " << Fixed64Math::Cos(angle32).ToString() << std::endl;

    // Square root
    math::fp::Fixed64<16> value(16);
    std::cout << "Sqrt(16): " << Fixed64Math::Sqrt(value).ToString() << std::endl;

    return 0;
}
