# Fixed64 Testing Best Practices

## 1. Use English Comments
All comments in test code should be written in English for better international collaboration and understanding.

## 2. Use Fixed::Epsilon() for Error Margins
When comparing floating-point results, always use `Fixed::Epsilon()` as the error margin:

```cpp
constexpr double epsilon8 = static_cast<double>(math::fp::Fixed64<8>::Epsilon());
constexpr double epsilon16 = static_cast<double>(math::fp::Fixed64<16>::Epsilon());
constexpr double epsilon32 = static_cast<double>(math::fp::Fixed64<32>::Epsilon());
constexpr double epsilon40 = static_cast<double>(math::fp::Fixed64<40>::Epsilon());

// Good practice
EXPECT_NEAR(static_cast<double>(value), expected, epsilon16);

// Avoid using arbitrary values
// EXPECT_NEAR(static_cast<double>(value), expected, 1e-6); // Not recommended
```

## 3. Calculate Expected Values from Double Conversions
For multiplication, division, and modulo operations, calculate the expected value by converting Fixed values to double, rather than using literal values:

```cpp
// Good practice
EXPECT_NEAR(static_cast<double>(a % b),
            std::fmod(static_cast<double>(a), static_cast<double>(b)),
            static_cast<double>(Fixed::Epsilon()));

// For large values or values that might have precision issues
Fixed huge(1e10);
EXPECT_NEAR(static_cast<double>(huge), 1e10, static_cast<double>(Fixed::Epsilon()));

// For division with small values
EXPECT_NEAR(static_cast<double>(tiny1 / tiny2),
            static_cast<double>(tiny1) / static_cast<double>(tiny2),
            static_cast<double>(Fixed::Epsilon()));
```

## 4. Test Edge Cases Properly
When testing edge cases, ensure the test is actually verifying the correct behavior:

```cpp
// For very large values
Fixed huge(1e10);
EXPECT_NEAR(static_cast<double>(huge), 1e10, static_cast<double>(Fixed::Epsilon()));

// For very small values
Fixed tiny(1e-10);
EXPECT_NEAR(static_cast<double>(tiny), 0.0, static_cast<double>(Fixed::Epsilon()));
```

## 5. Comprehensive Test Coverage
Each function should be tested across its entire valid numerical range to ensure reliability:

- Test with zero, small positive, small negative, large positive, and large negative values
- Test with values near the limits of the representable range
- Test with edge cases specific to the function (e.g., division by near-zero values)
- For trigonometric functions, test with angles in different quadrants

## 6. Test File Organization
Organize test files with a granular approach:

- Each function or related group of functions should have its own test file
- Follow the existing directory structure in the math directory
- Name test files clearly to indicate what functionality they test
- Group related tests into logical test suites
Example file organization:

```plaintext
tests/
├── basic/
│   ├── ConstructionTests.cpp
│   ├── ConversionTests.cpp
│   ├── ArithmeticTests.cpp
│   └── ...
├── math/
│   ├── TrigTests.cpp
│   ├── RoundingTests.cpp
│   ├── InterpolationTests.cpp
│   ├── ExponentialTests.cpp
│   └── ...
└── # Fixed64 Testing Best Practices.md
```