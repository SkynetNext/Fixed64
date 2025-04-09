import mpmath as mp
import sys

# Set very high precision
mp.mp.dps = 100


def generate_sin_lut(output_file=None, int_bits=23, fraction_bits=40):
    """Generate a lookup table for sin in the range [0,pi/2]"""

    # Use exactly 512 entries for the first quadrant
    lut_size = 512

    # Prepare the output with proper headers
    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <array>")
    lines.append("#include \"primitives.h\"")
    lines.append("")
    lines.append(f"// Sin lookup table with {lut_size} entries")
    lines.append(
        f"// Covers the range [0,pi/2] with values in Q{int_bits}.{fraction_bits} format")
    lines.append(
        f"// Generated with mpmath library at {mp.mp.dps} digits precision")
    lines.append("")

    # Generate the table header
    lines.append("namespace math::fp::detail {")
    lines.append("// Table maps x in [0,pi/2] to sin(x)")
    lines.append(
        f"// Values stored in Q{int_bits}.{fraction_bits} fixed-point format")
    lines.append(
        f"inline constexpr std::array<int64_t, {lut_size}> kSinLut = {{")

    # Generate the table entries in Q23.40 format
    scale = mp.mpf(2) ** fraction_bits
    pi_over_2 = mp.pi / 2
    angle_step = pi_over_2 / (lut_size - 1)

    for i in range(lut_size):
        angle = mp.mpf(i) * angle_step
        sin_x = mp.sin(angle)

        # Use truncation instead of rounding
        scaled_value = int(sin_x * scale)  # Truncate instead of round

        # Format the value as a hexadecimal literal with LL suffix
        hex_value = f"0x{scaled_value & 0xFFFFFFFFFFFFFFFF:016X}LL"
        if scaled_value < 0:
            hex_value = f"-0x{-scaled_value & 0xFFFFFFFFFFFFFFFF:016X}LL"

        # Generate a comment showing the floating point representation
        angle_float = float(angle)
        sin_x_float = float(sin_x)

        comment = f"// sin({angle_float:.14f}) = {sin_x_float:.14f}"

        # Add the entry with comment
        if i < lut_size - 1:
            lines.append(f"    {hex_value}, {comment}")
        else:
            lines.append(f"    {hex_value}  {comment}")

    lines.append("};")
    lines.append("")

    # Calculate constants in Q23.40 format with truncation
    pi = mp.pi
    pi_over_2 = pi / 2
    two_pi = pi * 2
    pi_scaled = int(pi * scale)  # Truncate
    pi_over_2_scaled = int(pi_over_2 * scale)  # Truncate
    two_pi_scaled = int(two_pi * scale)  # Truncate
    lut_interval_scaled = int(
        (lut_size - 1) / float(pi_over_2) * scale)  # Truncate

    pi_hex = f"0x{pi_scaled & 0xFFFFFFFFFFFFFFFF:016X}LL"
    pi_over_2_hex = f"0x{pi_over_2_scaled & 0xFFFFFFFFFFFFFFFF:016X}LL"
    two_pi_hex = f"0x{two_pi_scaled & 0xFFFFFFFFFFFFFFFF:016X}LL"
    lut_interval_hex = f"0x{lut_interval_scaled & 0xFFFFFFFFFFFFFFFF:016X}LL"

    # Generate the Fast Sin lookup function (linear interpolation)
    lines.append(
        "// Fast lookup sin(x) with linear interpolation between table entries")
    lines.append(
        "// Input x is in fixed-point format with specified fraction bits representing angle in radians")
    lines.append(
        f"// Output is in Q{int_bits}.{fraction_bits} fixed-point format representing sin(x)")
    lines.append("// Precision: ~1e-6 when input_fraction_bits=32")
    lines.append(
        "inline constexpr auto LookupSinFast(int64_t x, int input_fraction_bits) noexcept -> int64_t {")
    lines.append("    // Constants")
    lines.append(f"    constexpr int64_t kPi = {pi_hex};  // pi = {float(pi)}")
    lines.append(
        f"    constexpr int64_t kPiOver2 = {pi_over_2_hex};  // pi/2 = {float(pi_over_2)}")
    lines.append(
        f"    constexpr int64_t kTwoPi = {two_pi_hex};  // 2*pi = {float(two_pi)}")
    lines.append(
        f"    constexpr int64_t kLutInterval = {lut_interval_hex};  // LUT conversion factor")
    lines.append(
        f"    constexpr int kOutputFractionBits = {fraction_bits};  // Output format: Q{int_bits}.{fraction_bits}")
    lines.append("")

    lines.append("    // Convert input to Q{int_bits}.{fraction_bits} format if needed")
    lines.append("    if (input_fraction_bits != kOutputFractionBits) {")
    lines.append("        if (input_fraction_bits < kOutputFractionBits) {")
    lines.append(
        "            x <<= (kOutputFractionBits - input_fraction_bits);")
    lines.append("        } else {")
    lines.append(
        "            x >>= (input_fraction_bits - kOutputFractionBits);")
    lines.append("        }")
    lines.append("    }")
    lines.append("")

    lines.append("    // 1. Normalize angle to [0, 2*pi)")
    lines.append("    x = x % kTwoPi;")
    lines.append("    if (x < 0) {")
    lines.append("        x += kTwoPi;")
    lines.append("    }")
    lines.append("")

    lines.append("    // 2. Determine quadrant and map to [0, pi/2]")
    lines.append("    bool flip_sign = false;")
    lines.append("    if (x > kPi) {")
    lines.append("        // 3rd and 4th quadrants: sin(x) = -sin(x - pi)")
    lines.append("        x -= kPi;")
    lines.append("        flip_sign = true;")
    lines.append("    }")
    lines.append("    if (x > kPiOver2) {")
    lines.append("        // 2nd and 4th quadrants: sin(x) = sin(pi - x)")
    lines.append("        x = kPi - x;")
    lines.append("    }")
    lines.append("")

    lines.append("    // 3. Calculate lookup table index and fractional part")
    lines.append(
        "    int64_t idx_scaled = Primitives::Fixed64Mul(x, kLutInterval, kOutputFractionBits);")
    lines.append(
        "    int idx = static_cast<int>(idx_scaled >> kOutputFractionBits);")
    lines.append(
        "    int64_t frac = idx_scaled & ((1LL << kOutputFractionBits) - 1);")
    lines.append("")

    lines.append("    // 4. Clamp index to valid range")
    lines.append("    if (idx < 0) {")
    lines.append("        idx = 0;")
    lines.append("        frac = 0;")
    lines.append(
        "    } else if (idx >= static_cast<int>(kSinLut.size()) - 1) {")
    lines.append("        idx = static_cast<int>(kSinLut.size()) - 2;")
    lines.append("        frac = (1LL << kOutputFractionBits) - 1;")
    lines.append("    }")
    lines.append("")

    lines.append("    // 5. Linear interpolation between table entries")
    lines.append("    int64_t y0 = kSinLut[idx];")
    lines.append("    int64_t y1 = kSinLut[idx + 1];")
    lines.append("    int64_t diff = y1 - y0;")
    lines.append(
        "    int64_t interpolated_value = y0 + Primitives::Fixed64Mul(diff, frac, kOutputFractionBits);")
    lines.append("")

    lines.append("    // 6. Apply sign flip if necessary")
    lines.append("    if (flip_sign) {")
    lines.append("        interpolated_value = -interpolated_value;")
    lines.append("    }")
    lines.append("")

    lines.append(
        "    // 7. Convert result back to original input format if needed")
    lines.append("    if (input_fraction_bits != kOutputFractionBits) {")
    lines.append("        if (input_fraction_bits < kOutputFractionBits) {")
    lines.append(
        "            interpolated_value >>= (kOutputFractionBits - input_fraction_bits);")
    lines.append("        } else {")
    lines.append(
        "            interpolated_value <<= (input_fraction_bits - kOutputFractionBits);")
    lines.append("        }")
    lines.append("    }")
    lines.append("")

    lines.append("    return interpolated_value;")
    lines.append("}")
    lines.append("")

    # Generate the Hermite interpolation version
    lines.append(
        "// Lookup sin(x) with optimized Hermite cubic interpolation between table entries")
    lines.append(
        "// Input x is in fixed-point format with specified fraction bits representing angle in radians")
    lines.append(
        f"// Output is in Q{int_bits}.{fraction_bits} fixed-point format representing sin(x)")
    lines.append(
        "// Precision: ~1.0e-9 when input_fraction_bits=32 (about 1500x more accurate than fast version)")
    lines.append(
        "inline constexpr auto LookupSin(int64_t x, int input_fraction_bits) noexcept -> int64_t {")
    lines.append("    // Constants")
    lines.append(f"    constexpr int64_t kPi = {pi_hex};  // pi = {float(pi)}")
    lines.append(
        f"    constexpr int64_t kPiOver2 = {pi_over_2_hex};  // pi/2 = {float(pi_over_2)}")
    lines.append(
        f"    constexpr int64_t kTwoPi = {two_pi_hex};  // 2*pi = {float(two_pi)}")
    lines.append(
        f"    constexpr int64_t kLutInterval = {lut_interval_hex};  // LUT conversion factor")
    lines.append(
        f"    constexpr int kOutputFractionBits = {fraction_bits};  // Output format: Q{int_bits}.{fraction_bits}")
    lines.append(
        "    constexpr int64_t kOne = 1LL << kOutputFractionBits;  // 1.0 in fixed-point")
    lines.append("")

    lines.append("    // Convert input to Q{int_bits}.{fraction_bits} format if needed")
    lines.append("    if (input_fraction_bits != kOutputFractionBits) {")
    lines.append("        if (input_fraction_bits < kOutputFractionBits) {")
    lines.append(
        "            x <<= (kOutputFractionBits - input_fraction_bits);")
    lines.append("        } else {")
    lines.append(
        "            x >>= (input_fraction_bits - kOutputFractionBits);")
    lines.append("        }")
    lines.append("    }")
    lines.append("")

    lines.append("    // 1. Normalize angle to [0, 2*pi)")
    lines.append("    x = x % kTwoPi;")
    lines.append("    if (x < 0) {")
    lines.append("        x += kTwoPi;")
    lines.append("    }")
    lines.append("")

    lines.append("    // 2. Determine quadrant and map to [0, pi/2]")
    lines.append("    bool flip_sign = false;")
    lines.append("    if (x > kPi) {")
    lines.append("        // 3rd and 4th quadrants: sin(x) = -sin(x - pi)")
    lines.append("        x -= kPi;")
    lines.append("        flip_sign = true;")
    lines.append("    }")
    lines.append("    if (x > kPiOver2) {")
    lines.append("        // 2nd and 4th quadrants: sin(x) = sin(pi - x)")
    lines.append("        x = kPi - x;")
    lines.append("    }")
    lines.append("")

    lines.append("    // 3. Calculate lookup table index and fractional part")
    lines.append(
        "    int64_t idx_scaled = Primitives::Fixed64Mul(x, kLutInterval, kOutputFractionBits);")
    lines.append(
        "    int idx = static_cast<int>(idx_scaled >> kOutputFractionBits);")
    lines.append(
        "    int64_t t = idx_scaled & ((1LL << kOutputFractionBits) - 1);  // Fractional part [0,1)")
    lines.append("")

    lines.append("    // 4. Clamp index to valid range")
    lines.append("    if (idx < 0) {")
    lines.append("        idx = 0;")
    lines.append("        t = 0;")
    lines.append(
        "    } else if (idx >= static_cast<int>(kSinLut.size()) - 1) {")
    lines.append("        idx = static_cast<int>(kSinLut.size()) - 2;")
    lines.append("        t = kOne - 1;  // Just under 1.0")
    lines.append("    }")
    lines.append("")

    lines.append("    // 5. Get points from table")
    lines.append(
        "    int64_t p0 = kSinLut[idx];      // Point at left endpoint")
    lines.append(
        "    int64_t p1 = kSinLut[idx + 1];  // Point at right endpoint")
    lines.append("")

    lines.append(
        "    // 6. Compute derivatives using the fact that sin'(x) = cos(x)")
    lines.append("    // We can use the identity cos(x) = sin(x + pi/2)")
    lines.append(
        "    // For the first quadrant, we can use cos(x) = sin(pi/2 - x) when x is in [0,pi/2]")
    lines.append(
        "    int cos_idx = static_cast<int>(kSinLut.size()) - 1 - idx;")
    lines.append(
        "    int64_t m0 = kSinLut[cos_idx];  // Derivative (cos) at left endpoint")
    lines.append(
        "    int64_t m1 = cos_idx > 0 ? kSinLut[cos_idx - 1] : 0;  // Derivative at right endpoint")
    lines.append("")

    lines.append("    // Scale derivatives by step size")
    lines.append("    constexpr int64_t kStepSize = Primitives::Fixed64Div(")
    lines.append(
        "        kPiOver2, (kSinLut.size() - 1) << kOutputFractionBits, kOutputFractionBits);")
    lines.append(
        "    m0 = Primitives::Fixed64Mul(m0, kStepSize, kOutputFractionBits);")
    lines.append(
        "    m1 = Primitives::Fixed64Mul(m1, kStepSize, kOutputFractionBits);")
    lines.append("")

    lines.append("    // 7. Compute optimized Hermite coefficients")
    lines.append("    // p(t) = ((a*t + b)*t + c)*t + d  (Horner's method)")
    lines.append("    // where:")
    lines.append("    // a = 2(p₀-p₁) + m₀+m₁")
    lines.append("    // b = 3(p₁-p₀) - 2m₀-m₁")
    lines.append("    // c = m₀")
    lines.append("    // d = p₀")
    lines.append("    int64_t p0_minus_p1 = p0 - p1;")
    lines.append("    int64_t a = p0_minus_p1 * 2 + m0 + m1;")
    lines.append("    int64_t b = -p0_minus_p1 * 3 - m0 * 2 - m1;")
    lines.append("    int64_t c = m0;")
    lines.append("    int64_t d = p0;")
    lines.append("")

    lines.append("    // 8. Compute interpolation using Horner's method")
    lines.append("    int64_t result =")
    lines.append("        d")
    lines.append("        + Primitives::Fixed64Mul(")
    lines.append("            t,")
    lines.append("            c")
    lines.append("                + Primitives::Fixed64Mul(")
    lines.append(
        "                    t, b + Primitives::Fixed64Mul(t, a, kOutputFractionBits), kOutputFractionBits),")
    lines.append("            kOutputFractionBits);")
    lines.append("")

    lines.append("    // 9. Apply sign flip if necessary")
    lines.append("    if (flip_sign) {")
    lines.append("        result = -result;")
    lines.append("    }")
    lines.append("")

    lines.append(
        "    // 10. Convert result back to original input format if needed")
    lines.append("    if (input_fraction_bits != kOutputFractionBits) {")
    lines.append("        if (input_fraction_bits < kOutputFractionBits) {")
    lines.append(
        "            result >>= (kOutputFractionBits - input_fraction_bits);")
    lines.append("        } else {")
    lines.append(
        "            result <<= (input_fraction_bits - kOutputFractionBits);")
    lines.append("        }")
    lines.append("    }")
    lines.append("")

    lines.append("    return result;")
    lines.append("}")
    lines.append("")

    lines.append("}  // namespace math::fp::detail")

    # Write to file or stdout
    if output_file:
        with open(output_file, "w", encoding="utf-8") as f:
            f.write("\n".join(lines))
    else:
        print("\n".join(lines))


if __name__ == "__main__":
    int_bits = 23       # 23 bits for integer part
    fraction_bits = 40  # 40 bits for fractional part
    output_file = None

    # Parse command line arguments if provided
    if len(sys.argv) > 1:
        output_file = sys.argv[1]

    if len(sys.argv) > 2:
        try:
            fraction_bits = int(sys.argv[2])
            int_bits = 63 - fraction_bits  # Ensure we stay within 64-bit
        except ValueError:
            print(f"Error: Invalid fraction bits: {sys.argv[2]}")
            sys.exit(1)

    # Generate the sin lookup table
    generate_sin_lut(output_file, int_bits, fraction_bits)
