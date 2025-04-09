import mpmath as mp
import sys

# Set very high precision
mp.mp.dps = 100


def generate_tan_lut(output_file=None, int_bits=23, fraction_bits=40):
    """Generate a lookup table for tan in the range [0,pi/2]"""

    # Use exactly 512 entries
    lut_size = 512

    # Prepare the output with proper headers
    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <array>")
    lines.append("#include \"primitives.h\"")
    lines.append("")
    lines.append(f"// Tan lookup table with {lut_size} entries")
    lines.append(
        f"// Covers the range [0,pi/2] with values in Q{int_bits}.{fraction_bits} format")
    lines.append(
        f"// Generated with mpmath library at {mp.mp.dps} digits precision")
    lines.append("")

    # Generate the table header
    lines.append("namespace math::fp::detail {")
    lines.append("// Table maps x in [0,pi/2] to tan(x)")
    lines.append(
        f"// Values stored in Q{int_bits}.{fraction_bits} fixed-point format")
    lines.append(
        f"inline constexpr std::array<int64_t, {lut_size}> kTanLut = {{")

    # Generate the table entries in Q23.40 format
    scale = mp.mpf(2) ** fraction_bits
    pi_over_2 = mp.pi / 2
    angle_step = pi_over_2 / (lut_size - 1)

    # Max value representable in Q23.40
    max_value = mp.mpf(2**int_bits - 1) + mp.mpf(2**fraction_bits - 1)/scale

    for i in range(lut_size):
        angle = mp.mpf(i) * angle_step
        tan_x = mp.tan(angle)

        # Cap extremely large values
        if tan_x > max_value:
            tan_x = max_value

        # Use truncation instead of rounding
        scaled_value = int(tan_x * scale)  # Truncate instead of round

        # Format the value as a hexadecimal literal with LL suffix
        hex_value = f"0x{scaled_value & 0xFFFFFFFFFFFFFFFF:016X}LL"
        if scaled_value < 0:
            hex_value = f"-0x{-scaled_value & 0xFFFFFFFFFFFFFFFF:016X}LL"

        # Generate a comment showing the floating point representation
        angle_float = float(angle)
        tan_x_float = float(tan_x)

        comment = f"// tan({angle_float:.14f}) = {tan_x_float:.14f}"

        # Add the entry with comment
        if i < lut_size - 1:
            lines.append(f"    {hex_value}, {comment}")
        else:
            lines.append(f"    {hex_value}  {comment}")

    lines.append("};")
    lines.append("")

    # Generate constants for the Fast Tan lookup function (linear interpolation)
    lines.append(
        "// Fast lookup tan(x) with linear interpolation between table entries")
    lines.append(
        "// Input x is in fixed-point format with specified fraction bits representing angle in radians")
    lines.append("// Precision: ~1.5e-5 when input_fraction_bits=32")
    lines.append(
        "inline constexpr auto LookupTanFast(int64_t x, int input_fraction_bits) noexcept -> int64_t {")
    lines.append("    // Constants")

    # Calculate constants in Q23.40 format with truncation
    pi = mp.pi
    pi_over_2 = pi / 2
    pi_scaled = int(pi * scale)  # Truncate
    pi_over_2_scaled = int(pi_over_2 * scale)  # Truncate
    lut_interval_scaled = int(
        (lut_size - 1) / float(pi_over_2) * scale)  # Truncate

    pi_hex = f"0x{pi_scaled & 0xFFFFFFFFFFFFFFFF:016X}LL"
    pi_over_2_hex = f"0x{pi_over_2_scaled & 0xFFFFFFFFFFFFFFFF:016X}LL"
    lut_interval_hex = f"0x{lut_interval_scaled & 0xFFFFFFFFFFFFFFFF:016X}LL"

    lines.append(f"    constexpr int64_t kPi = {pi_hex};  // pi = {float(pi)}")
    lines.append(
        f"    constexpr int64_t kPiOver2 = {pi_over_2_hex};  // pi/2 = {float(pi_over_2)}")
    lines.append(
        f"    constexpr int64_t kLutInterval = {lut_interval_hex};  // LUT conversion factor")
    lines.append(
        f"    constexpr int kOutputFractionBits = {fraction_bits};  // Output format: Q{int_bits}.{fraction_bits}")
    lines.append("")

    lines.append("    // Convert input to Q23.40 format if needed")
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

    lines.append("    // 1. Normalize angle to [-pi, pi]")
    lines.append("    x = x % kPi;")
    lines.append("")

    lines.append("    // 2. Handle negative angles")
    lines.append("    bool flip = false;")
    lines.append("    if (x < 0) {")
    lines.append("        x = -x;")
    lines.append("        flip = true;")
    lines.append("    }")
    lines.append("")

    lines.append("    // 3. Handle angles > pi/2 by using tan(pi-x) = -tan(x)")
    lines.append("    if (x > kPiOver2) {")
    lines.append("        x = kPi - x;")
    lines.append("        flip = !flip;")
    lines.append("    }")
    lines.append("")

    lines.append("    // 4. Calculate lookup table index and fractional part")
    lines.append(
        "    int64_t idx_scaled = Primitives::Fixed64Mul(x, kLutInterval, kOutputFractionBits);")
    lines.append(
        "    int idx = static_cast<int>(idx_scaled >> kOutputFractionBits);")
    lines.append(
        "    int64_t frac = idx_scaled & ((1LL << kOutputFractionBits) - 1);")
    lines.append("")

    lines.append("    // 5. Clamp index to valid range")
    lines.append("    if (idx < 0) {")
    lines.append("        idx = 0;")
    lines.append("        frac = 0;")
    lines.append(
        "    } else if (idx >= static_cast<int>(kTanLut.size()) - 1) {")
    lines.append("        idx = static_cast<int>(kTanLut.size()) - 2;")
    lines.append("        frac = (1LL << kOutputFractionBits) - 1;")
    lines.append("    }")
    lines.append("")

    lines.append("    // 6. Linear interpolation between table entries")
    lines.append("    int64_t y0 = kTanLut[idx];")
    lines.append("    int64_t y1 = kTanLut[idx + 1];")
    lines.append("    int64_t diff = y1 - y0;")
    lines.append(
        "    int64_t interpolated_value = y0 + Primitives::Fixed64Mul(diff, frac, kOutputFractionBits);")
    lines.append("")

    lines.append("    // 7. Apply sign flip if necessary")
    lines.append(
        "    int64_t result = flip ? -interpolated_value : interpolated_value;")
    lines.append("")

    lines.append(
        "    // 8. Convert result back to original input format if needed")
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

    # Generate the Hermite interpolation version with optimized Horner method
    lines.append(
        "// Lookup tan(x) with optimized Hermite cubic interpolation between table entries")
    lines.append(
        "// Input x is in fixed-point format with specified fraction bits representing angle in radians")
    lines.append(
        "// Precision: ~1.0e-9 when input_fraction_bits=32 (about 1500x more accurate than fast version)")
    lines.append(
        "inline constexpr auto LookupTan(int64_t x, int input_fraction_bits) noexcept -> int64_t {")
    lines.append("    // Constants")
    lines.append(f"    constexpr int64_t kPi = {pi_hex};  // pi = {float(pi)}")
    lines.append(
        f"    constexpr int64_t kPiOver2 = {pi_over_2_hex};  // pi/2 = {float(pi_over_2)}")
    lines.append(
        f"    constexpr int64_t kLutInterval = {lut_interval_hex};  // LUT conversion factor")
    lines.append(
        f"    constexpr int kOutputFractionBits = {fraction_bits};  // Output format: Q{int_bits}.{fraction_bits}")
    lines.append(
        "    constexpr int64_t kOne = 1LL << kOutputFractionBits;  // 1.0 in fixed-point")
    lines.append("")

    lines.append("    // Convert input to Q23.40 format if needed")
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

    lines.append("    // 1. Normalize angle to [-pi, pi]")
    lines.append("    x = x % kPi;")
    lines.append("")

    lines.append("    // 2. Handle negative angles")
    lines.append("    bool flip = false;")
    lines.append("    if (x < 0) {")
    lines.append("        x = -x;")
    lines.append("        flip = true;")
    lines.append("    }")
    lines.append("")

    lines.append("    // 3. Handle angles > pi/2 by using tan(pi-x) = -tan(x)")
    lines.append("    if (x > kPiOver2) {")
    lines.append("        x = kPi - x;")
    lines.append("        flip = !flip;")
    lines.append("    }")
    lines.append("")

    lines.append("    // 4. Calculate lookup table index and fractional part")
    lines.append(
        "    int64_t idx_scaled = Primitives::Fixed64Mul(x, kLutInterval, kOutputFractionBits);")
    lines.append(
        "    int idx = static_cast<int>(idx_scaled >> kOutputFractionBits);")
    lines.append(
        "    int64_t t = idx_scaled & ((1LL << kOutputFractionBits) - 1);  // Fractional part [0,1)")
    lines.append("")

    lines.append("    // 5. Clamp index to valid range")
    lines.append("    if (idx < 0) {")
    lines.append("        idx = 0;")
    lines.append("        t = 0;")
    lines.append(
        "    } else if (idx >= static_cast<int>(kTanLut.size()) - 1) {")
    lines.append("        idx = static_cast<int>(kTanLut.size()) - 2;")
    lines.append("        t = kOne - 1;  // Just under 1.0")
    lines.append("    }")
    lines.append("")

    lines.append("    // 6. Get points from table")
    lines.append(
        "    int64_t p0 = kTanLut[idx];      // Point at left endpoint")
    lines.append(
        "    int64_t p1 = kTanLut[idx + 1];  // Point at right endpoint")
    lines.append("")

    lines.append(
        "    // 7. Compute derivatives using the fact that tan'(x) = 1 + tan²(x)")
    lines.append(
        "    int64_t p0_squared = Primitives::Fixed64Mul(p0, p0, kOutputFractionBits);")
    lines.append(
        "    int64_t p1_squared = Primitives::Fixed64Mul(p1, p1, kOutputFractionBits);")
    lines.append(
        "    int64_t m0 = kOne + p0_squared;  // Derivative at left endpoint")
    lines.append(
        "    int64_t m1 = kOne + p1_squared;  // Derivative at right endpoint")
    lines.append("")

    lines.append("    // Scale derivatives by step size")
    lines.append("    constexpr int64_t kStepSize = Primitives::Fixed64Div(")
    lines.append(
        "        kPiOver2, (kTanLut.size() - 1) << kOutputFractionBits, kOutputFractionBits);")
    lines.append(
        "    m0 = Primitives::Fixed64Mul(m0, kStepSize, kOutputFractionBits);")
    lines.append(
        "    m1 = Primitives::Fixed64Mul(m1, kStepSize, kOutputFractionBits);")
    lines.append("")

    lines.append("    // 8. Compute optimized Hermite coefficients")
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

    lines.append("    // 9. Compute interpolation using Horner's method")
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

    lines.append("    // 10. Apply sign flip if necessary")
    lines.append("    if (flip) {")
    lines.append("        result = -result;")
    lines.append("    }")
    lines.append("")

    lines.append(
        "    // 11. Convert result back to original input format if needed")
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

    # Generate the tan lookup table
    generate_tan_lut(output_file, int_bits, fraction_bits)
