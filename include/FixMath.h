#pragma once

#include <array>

#include "Fixed64.h"

class FixedMath {
 private:
  static inline const Fixed64 _atan2Number1{-883LL};
  static inline const Fixed64 _atan2Number2{3767LL};
  static inline const Fixed64 _atan2Number3{7945LL};
  static inline const Fixed64 _atan2Number4{12821LL};
  static inline const Fixed64 _atan2Number5{21822LL};
  static inline const Fixed64 _atan2Number6{65536LL};
  static inline const Fixed64 _atan2Number7{102943LL};
  static inline const Fixed64 _atan2Number8{205887LL};
  static inline const Fixed64 _atanApproximatedNumber1{16036LL};
  static inline const Fixed64 _atanApproximatedNumber2{4345LL};
  static inline const Fixed64 _pow2Number1{177LL};
  static inline const Fixed64 _expNumber1{94548LL};
  static inline const std::array<uint8_t, 32> _bsrLookup{
      0, 9,  1,  10, 13, 21, 2,  29, 11, 14, 16, 18, 22, 25, 3, 30,
      8, 12, 20, 28, 15, 17, 24, 7,  19, 27, 23, 6,  26, 5,  4, 31};

 public:
  static inline const Fixed64 PI = Fixed64Const::Pi;
  static inline Fixed64 Epsilon = Fixed64Const::ENotation3;
  static inline const Fixed64 Rad2Deg = Fixed64Const::RadToDeg;
  static inline const Fixed64 Deg2Rad = Fixed64Const::DegToRad;

  static int BitScanReverse(uint32_t num) {
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    return _bsrLookup[(num * 0x07C4ACDDU) >> 27];
  }

  static int CountLeadingZeroes(uint32_t num) {
    return num == 0 ? 32 : BitScanReverse(num) ^ 31;
  }

  static Fixed64 Pow2(Fixed64 num) {
    if (num.value > 1638400) {
      return Fixed64Const::Max;
    }

    int i = static_cast<int>(num);
    num = Fractions(num) * _pow2Number1 + Fixed64Const::One;
    // 进行7次平方，相当于2的(2^7)次幂
    for (int j = 0; j < 7; ++j) {
      num *= num;
    }

    if (i >= 0) {
      return num * num * Fixed64::parseLong(1LL << i);
    } else {
      Fixed64 divisor = Fixed64::parseLong(1LL << (-i));
      return num * num / divisor;
    }
  }

  static Fixed64 ExpApproximated(Fixed64 num) {
    return Pow2(num * _expNumber1);
  }

  static Fixed64 Sin(Fixed64 num) {
    num.value %= Fixed64Const::Pi2.value;
    num *= Fixed64Const::OneDivPi2;
    auto raw = FixLut::sin(num.value);
    Fixed64 result;
    result.value = raw;
    return result;
  }

  static Fixed64 Cos(Fixed64 num) {
    num.value %= Fixed64Const::Pi2.value;
    num *= Fixed64Const::OneDivPi2;
    return Fixed64(FixLut::cos(num.value));
  }

  static Fixed64 Tan(Fixed64 num) {
    num.value %= Fixed64Const::Pi2.value;
    num *= Fixed64Const::OneDivPi2;
    return Fixed64(FixLut::tan(num.value));
  }

  static Fixed64 Acos(Fixed64 num) {
    num.value += FixLut::ONE;
    num *= Fixed64Const::Point5;
    return Fixed64(FixLut::acos(num.value));
  }

  static Fixed64 Asin(Fixed64 num) {
    num.value += FixLut::ONE;
    num *= Fixed64Const::Point5;
    return Fixed64(FixLut::asin(num.value));
  }

  static Fixed64 Atan(Fixed64 num) { return Atan2(num, Fixed64Const::One); }

  static Fixed64 AtanApproximated(Fixed64 num) {
    auto absX = Abs(num);
    return Fixed64Const::PiQuarter * num -
           num * (absX - Fixed64Const::One) *
               (_atanApproximatedNumber1 + _atanApproximatedNumber2 * absX);
  }

  static Fixed64 Atan2(Fixed64 y, Fixed64 x) {
    auto absX = Abs(x);
    auto absY = Abs(y);
    auto t3 = absX;
    auto t1 = absY;
    auto t0 = Max(t3, t1);
    t1 = Min(t3, t1);
    t3 = Fixed64Const::One / t0;
    t3 = t1 * t3;
    auto t4 = t3 * t3;
    t0 = _atan2Number1;
    t0 = t0 * t4 + _atan2Number2;
    t0 = t0 * t4 - _atan2Number3;
    t0 = t0 * t4 + _atan2Number4;
    t0 = t0 * t4 - _atan2Number5;
    t0 = t0 * t4 + _atan2Number6;
    t3 = t0 * t3;
    t3 = absY > absX ? _atan2Number7 - t3 : t3;
    t3 = x < Fixed64Const::Zero ? _atan2Number8 - t3 : t3;
    t3 = y < Fixed64Const::Zero ? -t3 : t3;
    return t3;
  }

  static Fixed64 Rcp(Fixed64 num) { return Fixed64(4294967296LL / num.value); }

  static Fixed64 Rsqrt(Fixed64 num) {
    return Fixed64(4294967296LL / Sqrt(num).value);
  }

  static Fixed64 Sqrt(Fixed64 num) {
    Fixed64 r;

    if (num.value == 0) {
      r.value = 0;
    } else {
      int64_t b = (num.value >> 1) + 1;
      int64_t c = (b + (num.value / b)) >> 1;

      while (c < b) {
        b = c;
        c = (b + (num.value / b)) >> 1;
      }

      r.value = b << (FixLut::PRECISION >> 1);
    }

    return r;
  }

  static Fixed64 Floor(Fixed64 num) {
    num.value = num.value >> FixLut::PRECISION << FixLut::PRECISION;
    return num;
  }

  static int FloorToInt(Fixed64 num) { return static_cast<int>(Floor(num)); }

  static Fixed64 Ceil(Fixed64 num) {
    int64_t fractions = num.value & FixLut::FractionsMask;

    if (fractions == 0) {
      return num;
    }

    num.value = num.value >> FixLut::PRECISION << FixLut::PRECISION;
    num.value += FixLut::ONE;
    return num;
  }

  static int CeilToInt(Fixed64 num) { return static_cast<int>(Ceil(num)); }

  static Fixed64 Fractions(Fixed64 num) {
    return Fixed64(num.value & FixLut::FractionsMask);
  }

  static int RoundToInt(Fixed64 num) {
    int64_t fraction = num.value & FixLut::FractionsMask;

    if (fraction >= FixLut::HALF) {
      return static_cast<int>(num) + 1;
    }

    return static_cast<int>(num);
  }

  static Fixed64 Round(Fixed64 num) {
    int64_t fraction = num.value & FixLut::FractionsMask;

    if (fraction >= FixLut::HALF) {
      return Fixed64(((num.value >> FixLut::PRECISION) + 1)
                     << FixLut::PRECISION);
    }

    return Fixed64(num.value >> FixLut::PRECISION << FixLut::PRECISION);
  }

  static Fixed64 Min(Fixed64 a, Fixed64 b) { return a.value < b.value ? a : b; }

  static Fixed64 Max(Fixed64 a, Fixed64 b) { return a.value > b.value ? a : b; }

  static Fixed64 Abs(Fixed64 num) { return Fixed64(std::abs(num.value)); }

  static Fixed64 Clamp(Fixed64 num, Fixed64 min, Fixed64 max) {
    if (num.value < min.value) {
      return min;
    }
    if (num.value > max.value) {
      return max;
    }
    return num;
  }

  static Fixed64 Clamp01(Fixed64 num) {
    if (num.value < 0) {
      return Fixed64Const::Zero;
    }
    if (num.value > FixLut::ONE) {
      return Fixed64(FixLut::ONE);
    }
    return num;
  }

  static Fixed64 Lerp(Fixed64 from, Fixed64 to, Fixed64 t) {
    t = Clamp01(t);
    return from + (to - from) * t;
  }

  static Fixed64 InverseLerp(Fixed64 a, Fixed64 b, Fixed64 value) {
    if (a != b) {
      return Clamp01((value - a) / (b - a));
    }
    return Fixed64Const::Zero;
  }

  static Fixed64 Repeat(Fixed64 value, Fixed64 length) {
    return Clamp(value - Floor(value / length) * length, Fixed64Const::Zero, length);
  }

  static Fixed64 LerpAngle(Fixed64 from, Fixed64 to, Fixed64 t) {
    Fixed64 num = Repeat(to - from, Fixed64Const::Pi2);
    return Lerp(
        from, from + (num > Fixed64(FixLut::PI) ? num - Fixed64Const::Pi2 : num), t);
  }

  static Fixed64 NormalizeRadians(Fixed64 angle) {
    angle.value = std::fmod(angle.value, FixLut::PI);
    return angle;
  }

  static Fixed64 LerpUnclamped(Fixed64 from, Fixed64 to, Fixed64 t) {
    return from + (to - from) * t;
  }

  static Fixed64 Sign(Fixed64 num) {
    return num.value < 0 ? Fixed64(-FixLut::ONE) : Fixed64(FixLut::ONE);
  }

  static bool IsOppositeSign(Fixed64 a, Fixed64 b) {
    return ((a.value ^ b.value) < 0);
  }

  static Fixed64 SetSameSign(Fixed64 target, Fixed64 reference) {
    return IsOppositeSign(target, reference) ? target * Fixed64(-FixLut::ONE)
                                             : target;
  }

  static Fixed64 Pow2(int power) { return Fixed64(FixLut::ONE << power); }

  static Fixed64 Exp(Fixed64 num) {
    if (num == Fixed64Const::Zero) return Fixed64Const::One;
    if (num == Fixed64Const::One) return Fixed64Const::E;
    if (num.value >= 2097152) return Fixed64Const::Max;
    if (num.value <= -786432) return Fixed64Const::Zero;

    bool neg = num.value < 0;
    if (neg) num = -num;

    Fixed64 result = num + Fixed64Const::One;
    Fixed64 term = num;

    for (int i = 2; i < 30; ++i) {
      term = term * (num / Fixed64::parseLong(i));
      result = result + term;

      if (term.value < 500 && ((i > 15) || (term.value < 20))) break;
    }

    if (neg) result = Fixed64Const::One / result;

    return result;
  }
};
