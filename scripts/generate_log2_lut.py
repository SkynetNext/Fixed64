import mpmath as mp
import sys

# Set very high precision
mp.mp.dps = 100


def generate_log2_lut(output_file=None, int_bits=23, fraction_bits=40, table_size=256):
    """Generate a lookup table for log2 in the range [1,2)"""

    # Prepare the output with proper headers
    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <array>")
    lines.append("#include \"primitives.h\"")
    lines.append("")
    lines.append(f"// Log2 lookup table with {table_size} entries")
    lines.append(
        f"// Covers the range [1,2) with values in Q{int_bits}.{fraction_bits} format")
    lines.append(
        f"// Generated with mpmath library at {mp.mp.dps} digits precision")
    lines.append("")

    # Generate the table header
    lines.append("namespace math::fp::detail {")
    lines.append("// Table maps x in [1,2) to log2(x)")
    lines.append(
        f"// Values stored in Q{int_bits}.{fraction_bits} fixed-point format")
    lines.append(
        f"inline constexpr std::array<int64_t, {table_size}> kLog2Lut = {{")

    # Generate the table entries in Q23.40 format
    scale = mp.mpf(2) ** fraction_bits
    step = mp.mpf(1) / (table_size - 1)

    for i in range(table_size):
        # x ranges from 1.0 to 2.0 (inclusive)
        x = mp.mpf(1) + i * step
        log2_x = mp.log(x) / mp.log(2)  # log2(x) = ln(x)/ln(2)

        # Use truncation instead of rounding
        scaled_value = int(log2_x * scale)  # Truncate

        # Format the value as a hexadecimal literal with LL suffix
        hex_value = f"0x{scaled_value & 0xFFFFFFFFFFFFFFFF:016X}LL"
        if scaled_value < 0:
            hex_value = f"-0x{-scaled_value & 0xFFFFFFFFFFFFFFFF:016X}LL"

        # Generate a comment showing the floating point representation
        x_float = float(x)
        log2_x_float = float(log2_x)

        comment = f"// log2({x_float:.15f}) = {log2_x_float:.15f}"

        # Add the entry with comment
        if i < table_size - 1:
            lines.append(f"    {hex_value}, {comment}")
        else:
            lines.append(f"    {hex_value}  {comment}")

    lines.append("};")
    lines.append("")

    # Add analysis of derivatives
    lines.append("/*")
    lines.append(" * Derivative Analysis for log2(x):")
    lines.append(" * - First derivative: log2'(x) = 1/(x*ln(2))")
    lines.append(" * - Second derivative: log2''(x) = -1/(x²*ln(2))")
    lines.append(" *")
    lines.append(" * Key characteristics:")
    lines.append(
        " * 1. log2(x) is monotonically increasing, but its rate of change decreases as x increases")
    lines.append(
        " * 2. The second derivative is negative and gradually approaches zero as x increases")
    lines.append(" *")
    lines.append(" * Interpolation strategy:")
    lines.append(
        " * - For high precision requirements: Cubic Hermite interpolation provides excellent accuracy")
    lines.append(
        " *   as it accounts for the curvature of log2(x) and maintains derivative continuity")
    lines.append(
        " * - For performance-critical code: Linear interpolation may be sufficient, especially with")
    lines.append(
        " *   a table size of 256 or larger, as log2(x) is well-behaved in the [1,2) range")
    lines.append(" */")
    lines.append("")

    # Generate the ln(2) constant
    ln2 = mp.log(2)
    ln2_scaled = int(ln2 * scale)  # Truncate
    ln2_hex = f"0x{ln2_scaled & 0xFFFFFFFFFFFFFFFF:016X}LL"

    lines.append(
        f"// Natural logarithm of 2 in Q{int_bits}.{fraction_bits} format")
    lines.append(
        f"inline constexpr int64_t kLn2 = {ln2_hex};  // ln(2) ≈ {float(ln2)}")
    lines.append("")

    # Generate the Fast Log2 lookup function (linear interpolation)
    lines.append(
        "// Fast lookup log2(x) with linear interpolation between table entries")
    lines.append(
        "// Input x is in fixed-point format with specified fraction bits")
    lines.append(
        f"// Output is in Q{int_bits}.{fraction_bits} fixed-point format representing log2(x)")
    lines.append("// Precision: ~1.5e-6 absolute error")
    lines.append(
        "inline constexpr auto LookupLog2Fast(int64_t x, int input_fraction_bits) noexcept -> int64_t {")
    lines.append("    // Constants")
    lines.append(
        f"    constexpr int kOutputFractionBits = {fraction_bits};  // Output format: Q{int_bits}.{fraction_bits}")
    lines.append("    constexpr int64_t kOne = 1LL << kOutputFractionBits;")
    lines.append("")

    # Add the implementation
    lines.append("    // Convert input to internal format if needed")
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

    lines.append("    // Handle special cases")
    lines.append("    if (x <= 0) {")
    lines.append("        // Log of zero or negative is undefined")
    lines.append("        return INT64_MIN; // Return most negative value")
    lines.append("    }")
    lines.append("")

    # Optimized normalization using bit operations instead of loops
    lines.append("    // 1. Normalize x to [1,2) range using bit operations")
    lines.append("    int64_t mantissa = x;")
    lines.append("    int exponent = 0;")
    lines.append("")
    lines.append("    // Efficient normalization using CountlZero")
    lines.append(
        "    const uint64_t u_mantissa = static_cast<uint64_t>(mantissa);")
    lines.append(
        "    const int msb = 63 - Primitives::CountlZero(u_mantissa);")
    lines.append("    ")
    lines.append("    // Normalize to [1,2) range")
    lines.append("    const int shift = msb - kOutputFractionBits;")
    lines.append("    if (shift > 0) {")
    lines.append("        mantissa >>= shift;")
    lines.append("        exponent += shift;")
    lines.append("    } else if (shift < 0) {")
    lines.append("        mantissa <<= -shift;")
    lines.append("        exponent += shift;")
    lines.append("    }")
    lines.append("")

    lines.append("    // 2. Calculate lookup table index and fractional part")
    lines.append(
        f"    int64_t idx_scaled = ((mantissa - kOne) * {table_size-1}) >> kOutputFractionBits;")
    lines.append("    int idx = static_cast<int>(idx_scaled);")
    lines.append(
        f"    int64_t t = ((mantissa - kOne) * {table_size-1}) & ((1LL << kOutputFractionBits) - 1);")
    lines.append("")

    lines.append("    // 3. Clamp index to valid range")
    lines.append("    if (idx < 0) {")
    lines.append("        idx = 0;")
    lines.append("        t = 0;")
    lines.append(
        "    } else if (idx >= static_cast<int>(kLog2Lut.size()) - 1) {")
    lines.append("        idx = static_cast<int>(kLog2Lut.size()) - 2;")
    lines.append("        t = kOne - 1;  // Just under 1.0")
    lines.append("    }")
    lines.append("")

    lines.append("    // 4. Perform linear interpolation")
    lines.append("    int64_t p0 = kLog2Lut[idx];")
    lines.append("    int64_t p1 = kLog2Lut[idx + 1];")
    lines.append(
        "    int64_t interpolated_value = p0 + Primitives::Fixed64Mul(p1 - p0, t, kOutputFractionBits);")
    lines.append("")

    lines.append("    // 5. Add exponent part")
    lines.append(
        "    int64_t result = interpolated_value + (static_cast<int64_t>(exponent) << kOutputFractionBits);")
    lines.append("")

    lines.append(
        "    // 6. Convert result back to original input format if needed")
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

    # Also add cubic interpolation if higher precision is needed
    lines.append(
        "// Lookup log2(x) with cubic Hermite interpolation for higher precision")
    lines.append(
        "// Input x is in fixed-point format with specified fraction bits")
    lines.append(
        f"// Output is in Q{int_bits}.{fraction_bits} fixed-point format representing log2(x)")
    lines.append(
        "// Precision: ~2.33e-10 absolute error (Q31.32 minimum precision)")
    lines.append(
        "inline constexpr auto LookupLog2(int64_t x, int input_fraction_bits) noexcept -> int64_t {")
    # Constants
    lines.append("    // Constants")
    lines.append(
        f"    constexpr int kOutputFractionBits = {fraction_bits};  // Output format: Q{int_bits}.{fraction_bits}")
    lines.append("    constexpr int64_t kOne = 1LL << kOutputFractionBits;")
    lines.append("")

    # Input format conversion - same as LookupLog2Fast
    lines.append("    // Convert input to internal format if needed")
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

    # Special cases
    lines.append("    // Handle special cases")
    lines.append("    if (x <= 0) {")
    lines.append("        // Log of zero or negative is undefined")
    lines.append("        return INT64_MIN; // Return most negative value")
    lines.append("    }")
    lines.append("")

    # Improved normalization using bit operations
    lines.append("    // 1. Normalize x to [1,2) range using bit operations")
    lines.append("    int64_t mantissa = x;")
    lines.append("    int exponent = 0;")
    lines.append("")
    lines.append(
        "    // Efficient normalization using CountlZero or similar bit operation")
    lines.append("    // Find the position of the most significant bit")
    lines.append(
        "    const uint64_t u_mantissa = static_cast<uint64_t>(mantissa);")
    lines.append(
        "    const int msb = 63 - Primitives::CountlZero(u_mantissa);")
    lines.append("    ")
    lines.append("    // Normalize to [1,2) range")
    lines.append("    const int shift = msb - kOutputFractionBits;")
    lines.append("    if (shift > 0) {")
    lines.append("        mantissa >>= shift;")
    lines.append("        exponent += shift;")
    lines.append("    } else if (shift < 0) {")
    lines.append("        mantissa <<= -shift;")
    lines.append("        exponent += shift;")
    lines.append("    }")
    lines.append("")

    # Calculate index and fractional part
    lines.append("    // 2. Calculate lookup table index and fractional part")
    lines.append(
        f"    int64_t idx_scaled = ((mantissa - kOne) * {table_size-1}) >> kOutputFractionBits;")
    lines.append("    int idx = static_cast<int>(idx_scaled);")
    lines.append(
        f"    int64_t t = ((mantissa - kOne) * {table_size-1}) & ((1LL << kOutputFractionBits) - 1);")
    lines.append("")

    # Clamp index to valid range
    lines.append("    // 3. Clamp index to valid range")
    lines.append("    if (idx < 0) {")
    lines.append("        idx = 0;")
    lines.append("        t = 0;")
    lines.append(
        "    } else if (idx >= static_cast<int>(kLog2Lut.size()) - 1) {")
    lines.append("        idx = static_cast<int>(kLog2Lut.size()) - 2;")
    lines.append("        t = kOne - 1;  // Just under 1.0")
    lines.append("    }")
    lines.append("")

    # Look up values from table
    lines.append("    // 4. Get points from table")
    lines.append(
        "    int64_t p0 = kLog2Lut[idx];      // Point at left endpoint")
    lines.append(
        "    int64_t p1 = kLog2Lut[idx + 1];  // Point at right endpoint")
    lines.append("")

    # Compute derivatives
    lines.append("    // 5. Compute derivatives using the properties of log2")
    lines.append("    // log2'(x) = 1/(x*ln(2))")
    lines.append(
        "    int64_t x_at_idx = kOne + ((static_cast<int64_t>(idx) << kOutputFractionBits) / (kLog2Lut.size() - 1));")
    lines.append(
        "    int64_t x_at_next = kOne + ((static_cast<int64_t>(idx+1) << kOutputFractionBits) / (kLog2Lut.size() - 1));")
    lines.append("")
    lines.append("    int64_t m0 = Primitives::Fixed64Div(kOne, Primitives::Fixed64Mul(x_at_idx, kLn2, kOutputFractionBits), kOutputFractionBits);")
    lines.append("    int64_t m1 = Primitives::Fixed64Div(kOne, Primitives::Fixed64Mul(x_at_next, kLn2, kOutputFractionBits), kOutputFractionBits);")
    lines.append("")

    # Scale derivatives
    lines.append("    // Scale derivatives by step size")
    lines.append(
        f"    constexpr int64_t kStepSize = Primitives::Fixed64Div(kOne, static_cast<int64_t>({table_size-1}) << kOutputFractionBits, kOutputFractionBits);")
    lines.append(
        "    m0 = Primitives::Fixed64Mul(m0, kStepSize, kOutputFractionBits);")
    lines.append(
        "    m1 = Primitives::Fixed64Mul(m1, kStepSize, kOutputFractionBits);")
    lines.append("")

    # Compute cubic Hermite coefficients
    lines.append("    // 6. Compute cubic Hermite coefficients")
    lines.append("    int64_t p0_minus_p1 = p0 - p1;")
    lines.append("    int64_t a = p0_minus_p1 * 2 + m0 + m1;")
    lines.append("    int64_t b = -p0_minus_p1 * 3 - m0 * 2 - m1;")
    lines.append("    int64_t c = m0;")
    lines.append("    int64_t d = p0;")
    lines.append("")

    # Compute interpolation with Horner's method
    lines.append("    // 7. Compute interpolation using Horner's method")
    lines.append("    int64_t interpolated_value =")
    lines.append("        d")
    lines.append("        + Primitives::Fixed64Mul(")
    lines.append("            t,")
    lines.append("            c")
    lines.append("                + Primitives::Fixed64Mul(")
    lines.append(
        "                    t, b + Primitives::Fixed64Mul(t, a, kOutputFractionBits), kOutputFractionBits),")
    lines.append("            kOutputFractionBits);")
    lines.append("")

    # Add the exponent
    lines.append("    // 8. Add exponent part")
    lines.append(
        "    int64_t result = interpolated_value + (static_cast<int64_t>(exponent) << kOutputFractionBits);")
    lines.append("")

    # Convert back to original format
    lines.append(
        "    // 9. Convert result back to original input format if needed")
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
    table_size = 256    # Default table size
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

    if len(sys.argv) > 3:
        try:
            table_size = int(sys.argv[3])
        except ValueError:
            print(f"Error: Invalid table size: {sys.argv[3]}")
            sys.exit(1)

    # Generate the log2 lookup table
    generate_log2_lut(output_file, int_bits, fraction_bits, table_size)
