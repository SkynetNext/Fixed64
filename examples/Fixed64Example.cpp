#include "Fixed64.h"
#include <iostream>

int main()
{
    // Create some Fixed64 numbers
    Fixed64 num1(3.75); // Represents 3.75 in fixed-point notation
    Fixed64 num2(1.25); // Represents 1.25

    // Using constants
    Fixed64 one = Fixed64::One; // Represents 1.0
    std::cout << "One: " << one << std::endl;

    // Addition
    Fixed64 sum = num1 + num2;
    std::cout << "Sum: " << sum << std::endl; // Should print 5.0

    // Using the += operator
    sum += one;
    std::cout << "Sum after += One: " << sum << std::endl; // Should print 6.0

    // Subtraction
    Fixed64 difference = num1 - num2;
    std::cout << "Difference: " << difference << std::endl; // Should print 2.5

    // Using the -= operator
    difference -= one;
    std::cout << "Difference after -= One: " << difference << std::endl; // Should print 1.5

    // Multiplication
    Fixed64 product = num1 * num2;
    std::cout << "Product: " << product << std::endl; // Should print 4.6875

    // Using the *= operator
    product *= one;
    std::cout << "Product after *= One: " << product << std::endl; // Should print 4.6875

    // Division
    Fixed64 quotient = num1 / num2;
    std::cout << "Quotient: " << quotient << std::endl; // Should print 3.0

    // Using the /= operator
    quotient /= one;
    std::cout << "Quotient after /= One: " << quotient << std::endl; // Should print 3.0

    // Handling division by zero
    Fixed64 zero(0);
    Fixed64 divisionByZero = num1 / zero;
    if (divisionByZero == Fixed64::PositiveInfinity)
    {
        std::cout << "Division by zero results in PositiveInfinity" << std::endl;
    }
    else if (divisionByZero == Fixed64::NegativeInfinity)
    {
        std::cout << "Division by zero results in NegativeInfinity" << std::endl;
    }

    return 0;
}