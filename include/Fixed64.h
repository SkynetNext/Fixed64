#pragma once

#include <cmath>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "FixLut.h"

namespace Skynet
{
  struct Fixed64Param
  {
  public:
    bool isRaw;
    union
    {
      double raw;
      struct
      {
        int32_t high;
        uint32_t low;
      };
    };
  };

  struct Fixed64Wrapper
  {
    int32_t high;
    uint32_t low;
  };

  class Fixed64
  {
  public:
    int64_t value;

    Fixed64() : value(0) {}
    constexpr explicit Fixed64(int64_t v) : value(v) {}

    // Convert int to Fixed64
    constexpr Fixed64(int value)
        : value(static_cast<int64_t>(value) << FixLut::PRECISION) {}

    static constexpr Fixed64 parseLong(int64_t value)
    {
      return Fixed64(value << FixLut::PRECISION);
    }

    // Convert to string
    std::string toString() const
    {
      return std::to_string(static_cast<double>(*this));
    }

    // Convert a string to Fixed64
    static Fixed64 fromString(const std::string &str)
    {
      try
      {
        // Attempt to convert the string to a double
        double value = std::stod(str);
        // Create a Fixed64 object
        Fixed64 fixed;
        // Use the parseFloat member function to convert the double to Fixed64
        fixed.parseFloat(value);
        return fixed;
      }
      catch (const std::invalid_argument &e)
      {
        // If conversion fails (invalid input string), throw an exception
        throw std::invalid_argument("Invalid string for Fixed64 conversion");
      }
      catch (const std::out_of_range &e)
      {
        // If the conversion result is out of range, throw an exception
        throw std::out_of_range(
            "String value out of range for Fixed64 conversion");
      }
    }

    template <typename T>
    void parseFloat(const T &value)
    {
      int64_t integerPart = static_cast<int64_t>(std::floor(value));
      T fractionalPart = value - integerPart;
      this->value = (integerPart << FixLut::PRECISION) +
                    static_cast<int64_t>(fractionalPart * FixLut::ONE);
    }

    Fixed64(const Fixed64Param &param)
    {
      if (param.isRaw)
      {
        parseFloat(param.raw);
      }
      else
      {
        value = static_cast<int64_t>(param.high) << 32 |
                static_cast<int64_t>(param.low);
      }
    }

    Fixed64(const Fixed64Wrapper &param)
    {
      value = static_cast<int64_t>(param.high) << 32 |
              static_cast<int64_t>(param.low);
    }

    Fixed64(float value) { parseFloat(value); }

    Fixed64(double value) { parseFloat(value); }

    explicit operator float() const { return value / 65536.f; }

    explicit operator double() const { return value / 65536.0; }

    // Convert from Fixed64 to int
    explicit operator int() const
    {
      return static_cast<int>(value >> FixLut::PRECISION);
    }

    // Convert from Fixed64 to long
    explicit operator int64_t() const { return value >> FixLut::PRECISION; }

    operator Fixed64Param() const
    {
      Fixed64Param param;
      param.isRaw = false;
      param.high = static_cast<int32_t>(value >> 32);
      param.low = static_cast<uint32_t>(value & 0xFFFFFFFF);
      return param;
    }

    operator Fixed64Wrapper() const
    {
      Fixed64Wrapper data;
      data.high = static_cast<int32_t>(value >> 32);
      data.low = static_cast<uint32_t>(value & 0xFFFFFFFF);
      return data;
    }

    bool isInteger() const
    {
      return (value & ((1LL << FixLut::PRECISION) - 1)) == 0;
    }

    static bool isInteger(Fixed64 n)
    {
      return (n.value & ((1LL << FixLut::PRECISION) - 1)) == 0;
    }

    // Unary minus
    constexpr friend Fixed64 operator-(Fixed64 a)
    {
      a.value = -a.value;
      return a;
    }

    // Addition
    // += operator for Fixed64 and Fixed64
    constexpr Fixed64 &operator+=(const Fixed64 &rhs)
    {
      this->value += rhs.value;
      return *this;
    }

    // += operator for Fixed64 and int
    constexpr Fixed64 &operator+=(int rhs)
    {
      this->value += static_cast<int64_t>(rhs) << FixLut::PRECISION;
      return *this;
    }

    constexpr friend Fixed64 operator+(Fixed64 a, Fixed64 b)
    {
      a.value += b.value;
      return a;
    }

    constexpr friend Fixed64 operator+(Fixed64 a, int b)
    {
      a.value += static_cast<int64_t>(b) << FixLut::PRECISION;
      return a;
    }

    constexpr friend Fixed64 operator+(int a, Fixed64 b)
    {
      b.value += static_cast<int64_t>(a) << FixLut::PRECISION;
      return b;
    }

    // Subtraction
    constexpr friend Fixed64 operator-(Fixed64 a, Fixed64 b)
    {
      a.value -= b.value;
      return a;
    }

    constexpr friend Fixed64 operator-(Fixed64 a, int b)
    {
      a.value -= static_cast<int64_t>(b) << FixLut::PRECISION;
      return a;
    }

    constexpr friend Fixed64 operator-(int a, Fixed64 b)
    {
      b.value = (static_cast<int64_t>(a) << FixLut::PRECISION) - b.value;
      return b;
    }

    constexpr Fixed64 &operator-=(const Fixed64 &rhs)
    {
      this->value -= rhs.value;
      return *this;
    }

    constexpr Fixed64 &operator-=(int rhs)
    {
      this->value -= static_cast<int64_t>(rhs) << FixLut::PRECISION;
      return *this;
    }

    // Multiplication
    constexpr friend Fixed64 operator*(Fixed64 a, Fixed64 b)
    {
      a.value = (a.value * b.value) >> FixLut::PRECISION;
      return a;
    }

    constexpr friend Fixed64 operator*(Fixed64 a, int b)
    {
      a.value *= b;
      return a;
    }

    constexpr friend Fixed64 operator*(int a, Fixed64 b)
    {
      b.value *= a;
      return b;
    }

    constexpr Fixed64 &operator*=(const Fixed64 &b)
    {
      value = (value * b.value) >> FixLut::PRECISION;
      return *this;
    }

    constexpr Fixed64 &operator*=(int b)
    {
      value *= b;
      return *this;
    }

    // Division
    constexpr friend Fixed64 operator/(Fixed64 a, Fixed64 b)
    {
      if (b.value == 0)
      {
        return a < 0 ? Fixed64(NegativeInfinity) : Fixed64(PositiveInfinity);
      }
      a.value = (a.value << FixLut::PRECISION) / b.value;
      return a;
    }

    constexpr friend Fixed64 operator/(Fixed64 a, int b)
    {
      if (b == 0)
      {
        return a < 0 ? Fixed64(NegativeInfinity) : Fixed64(PositiveInfinity);
      }
      a.value /= b;
      return a;
    }

    constexpr friend Fixed64 operator/(int a, Fixed64 b)
    {
      if (b.value == 0)
      {
        return a < 0 ? Fixed64(NegativeInfinity) : Fixed64(PositiveInfinity);
      }
      b.value = (static_cast<int64_t>(a) << FixLut::PRECISION) / b.value;
      return b;
    }

    // /= operator overload
    constexpr Fixed64 &operator/=(const Fixed64 &rhs)
    {
      if (rhs.value == 0)
      {
        // Handle division by 0 case
        this->value = this->value < 0 ? Fixed64::NegativeInfinity
                                      : Fixed64::PositiveInfinity;
      }
      else
      {
        // In division, the precision factor is usually shifted left before the division to maintain precision
        this->value = (this->value << FixLut::PRECISION) / rhs.value;
      }
      return *this;
    }

    constexpr Fixed64 &operator/=(int rhs)
    {
      if (rhs == 0)
      {
        // Handle division by 0 case
        this->value = this->value < 0 ? Fixed64::NegativeInfinity
                                      : Fixed64::PositiveInfinity;
      }
      else
      {
        this->value /= rhs;
      }
      return *this;
    }

    friend Fixed64 operator%(Fixed64 a, Fixed64 b)
    {
      a.value %= b.value; // Modulo operation between two Fixed64 numbers
      return a;
    }

    friend Fixed64 operator%(Fixed64 a, int b)
    {
      a.value %= static_cast<int64_t>(b) << FixLut::PRECISION; // Modulo operation between Fixed64 and integer
      return a;
    }

    friend Fixed64 operator%(int a, Fixed64 b)
    {
      b.value = (static_cast<int64_t>(a) << FixLut::PRECISION) % b.value; // Modulo operation between integer and Fixed64
      return b;
    }

    // Comparison operators
    friend bool operator<(Fixed64 a, Fixed64 b) { return a.value < b.value; } // Less than

    friend bool operator<(Fixed64 a, int b)
    {
      return a.value < (static_cast<int64_t>(b) << FixLut::PRECISION); // Less than with integer
    }

    friend bool operator<(int a, Fixed64 b)
    {
      return (static_cast<int64_t>(a) << FixLut::PRECISION) < b.value; // Less than with integer
    }

    friend bool operator<=(Fixed64 a, Fixed64 b) { return a.value <= b.value; } // Less than or equal to

    friend bool operator<=(Fixed64 a, int b)
    {
      return a.value <= (static_cast<int64_t>(b) << FixLut::PRECISION); // Less than or equal to with integer
    }

    friend bool operator<=(int a, Fixed64 b)
    {
      return (static_cast<int64_t>(a) << FixLut::PRECISION) <= b.value; // Less than or equal to with integer
    }

    friend bool operator>(Fixed64 a, Fixed64 b) { return a.value > b.value; } // Greater than

    friend bool operator>(Fixed64 a, int b)
    {
      return a.value > (static_cast<int64_t>(b) << FixLut::PRECISION); // Greater than with integer
    }

    friend bool operator>(int a, Fixed64 b)
    {
      return (static_cast<int64_t>(a) << FixLut::PRECISION) > b.value; // Greater than with integer
    }

    friend bool operator>=(Fixed64 a, Fixed64 b) { return a.value >= b.value; } // Greater than or equal to

    friend bool operator>=(Fixed64 a, int b)
    {
      return a.value >= (static_cast<int64_t>(b) << FixLut::PRECISION); // Greater than or equal to with integer
    }

    friend bool operator>=(int a, Fixed64 b)
    {
      return (static_cast<int64_t>(a) << FixLut::PRECISION) >= b.value; // Greater than or equal to with integer
    }

    // Equality operator
    friend bool operator==(Fixed64 a, Fixed64 b) { return a.value == b.value; } // Equal to

    friend bool operator==(Fixed64 a, int b)
    {
      return a.value == (static_cast<int64_t>(b) << FixLut::PRECISION); // Equal to with integer
    }

    friend bool operator==(int a, Fixed64 b)
    {
      return (static_cast<int64_t>(a) << FixLut::PRECISION) == b.value; // Equal to with integer
    }

    // Inequality operator
    friend bool operator!=(Fixed64 a, Fixed64 b) { return a.value != b.value; } // Not equal to

    friend bool operator!=(Fixed64 a, int b)
    {
      return a.value != (static_cast<int64_t>(b) << FixLut::PRECISION); // Not equal to with integer
    }

    friend bool operator!=(int a, Fixed64 b)
    {
      return (static_cast<int64_t>(a) << FixLut::PRECISION) != b.value; // Not equal to with integer
    }

    // Check for infinity
    static bool IsInfinity(Fixed64 value)
    {
      return value.value == Fixed64::NegativeInfinity || value.value == Fixed64::PositiveInfinity; // Determines if the value is infinity
    }

    // Check for NaN
    static bool IsNaN(Fixed64 value) { return value.value == Fixed64::NaN; } // Determines if the value is NaN

    // Comparison function
    int CompareTo(Fixed64 other) const
    {
      if (value < other.value)
        return -1; // Less than
      if (value > other.value)
        return 1; // Greater than
      return 0;   // Equal
    }

    // Equality function
    bool Equals(Fixed64 other) const { return value == other.value; } // Determines if two Fixed64 values are equal

    // Override Object's Equals method
    bool Equals(const Fixed64 &obj) const { return value == obj.value; } // Determines if another object is equal to this instance

    // Get hash code
    std::size_t GetHashCode() const { return std::hash<int64_t>()(value); } // Computes the hash code for the value

    static constexpr int Size = 8; // Size of the Fixed64 type in bytes

  private:
    static constexpr int64_t PositiveInfinity = INT64_MAX;
    static constexpr int64_t NegativeInfinity = INT64_MIN + 1;
    static constexpr int64_t NaN = INT64_MIN;
  };

  inline bool operator>(const Fixed64Param &a, const Fixed64Param &b)
  {
    return Fixed64(a) > Fixed64(b);
  }

  inline bool operator<=(const Fixed64Param &a, const Fixed64Param &b)
  {
    return !(a > b);
  }

  inline std::ostream &operator<<(std::ostream &os, const Fixed64 &obj)
  {
    os << obj.toString();
    return os;
  }

  inline std::ostream &operator<<(std::ostream &os, const Fixed64Param &obj)
  {
    os << obj.isRaw;
    if (obj.isRaw)
    {
      os << obj.raw;
    }
    else
    {
      os << obj.high << "," << obj.low;
    }

    return os;
  }

  class Fixed64Const
  {
  public:
    static constexpr Fixed64 Max = Fixed64(INT64_MAX);
    static constexpr Fixed64 Min = Fixed64(INT64_MIN);
    static constexpr Fixed64 UsableMax = Fixed64(static_cast<int64_t>(2147483647LL));
    static constexpr Fixed64 UsableMin = Fixed64(static_cast<int64_t>(-2147483648LL));

    static constexpr Fixed64 Zero = Fixed64(0);
    static constexpr Fixed64 One = Fixed64(1);
    static constexpr Fixed64 Two = Fixed64(2);
    static constexpr Fixed64 Three = Fixed64(3);
    static constexpr Fixed64 Four = Fixed64(4);
    static constexpr Fixed64 Five = Fixed64(5);
    static constexpr Fixed64 Six = Fixed64(6);
    static constexpr Fixed64 Seven = Fixed64(7);
    static constexpr Fixed64 Eight = Fixed64(8);
    static constexpr Fixed64 Nine = Fixed64(9);
    static constexpr Fixed64 Ten = Fixed64(10);
    static constexpr Fixed64 NinetyNine = Fixed64(99);
    static constexpr Fixed64 Hundred = Fixed64(100);
    static constexpr Fixed64 TwoHundred = Fixed64(200);

    static constexpr Fixed64 Point01 = One / Hundred;
    static constexpr Fixed64 Point02 = Point01 * 2;
    static constexpr Fixed64 Point03 = Point01 * 3;
    static constexpr Fixed64 Point04 = Point01 * 4;
    static constexpr Fixed64 Point05 = Point01 * 5;
    static constexpr Fixed64 Point1 = One / 10;
    static constexpr Fixed64 Point2 = Point1 * 2;
    static constexpr Fixed64 Point25 = One / 4;
    static constexpr Fixed64 Point33 = One / 3;
    static constexpr Fixed64 Point5 = One / 2;
    static constexpr Fixed64 Point75 = One - Point25;
    static constexpr Fixed64 Point95 = One - Point05;
    static constexpr Fixed64 Point99 = One - Point01;
    static constexpr Fixed64 OnePoint01 = One + Point01;
    static constexpr Fixed64 OnePoint1 = One + Point1;
    static constexpr Fixed64 OnePoint5 = One + Point5;

    static constexpr Fixed64 ENotation1 = One / 10;
    static constexpr Fixed64 ENotation2 = One / 100;
    static constexpr Fixed64 ENotation3 = One / 1000;
    static constexpr Fixed64 ENotation4 = One / 10000;

    static constexpr Fixed64 MinusOne = Fixed64(-1);
    static constexpr Fixed64 Pi = Fixed64(static_cast<int64_t>(205887LL));
    static constexpr Fixed64 Pi2 = Pi * 2;
    static constexpr Fixed64 PiOver2 = Pi / 2;
    static constexpr Fixed64 PiQuarter = Pi * Point25;
    static constexpr Fixed64 PiHalf = Pi * Point5;
    static constexpr Fixed64 OneDivPi2 = One / Pi2;
    static constexpr Fixed64 DegToRad = Fixed64(static_cast<int64_t>(1143LL));
    static constexpr Fixed64 RadToDeg = Fixed64(static_cast<int64_t>(3754936LL));
    static constexpr Fixed64 Epsilon = Fixed64(static_cast<int64_t>(1LL));
    static constexpr Fixed64 E = Fixed64(static_cast<int64_t>(178145LL));

    static constexpr Fixed64 PiTimesTwo = Pi * 2;
    static constexpr Fixed64 DegreeToRadian = DegToRad;
    static constexpr Fixed64 RadianToDegree = RadToDeg;
    static constexpr Fixed64 MaxValue = Max;
    static constexpr Fixed64 MinValue = Min;
    static constexpr Fixed64 NaN = Min;
    static constexpr Fixed64 PositiveInfinity = Fixed64(INT64_MAX);
    static constexpr Fixed64 NegativeInfinity = Fixed64(INT64_MIN + 1);
  };
}