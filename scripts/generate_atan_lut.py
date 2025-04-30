import mpmath as mp
import sys

# Set very high precision
mp.mp.dps = 100


def generate_atan_lut(output_file=None, entries=512, fraction_bits=32):
    """Generate a lookup table for atan in the range [0,1]"""

    int_bits = 63 - fraction_bits

    # Prepare the output with proper headers
    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <array>")
    lines.append("#include \"primitives.h\"")
    lines.append("")
    lines.append(f"// Atan lookup table with {entries + 1} entries")
    lines.append(
        f"// Covers the range [0,1] with values in Q{int_bits}.{fraction_bits} format")
    lines.append(
        f"// Generated with mpmath library at {mp.mp.dps} digits precision")
    lines.append("")

    # Generate the table header
    lines.append("namespace math::fp::detail {")
    lines.append("// Table maps x in [0,1] to atan(x)")
    lines.append(f"// Values stored in Q{int_bits}.{fraction_bits} fixed-point format")
    lines.append(
        f"inline constexpr std::array<int64_t, {entries + 1}> kAtanLut = {{")

    # Generate the table entries
    scale = mp.mpf(2) ** fraction_bits
    pi_over_2 = mp.pi / 2
    pi_over_2_scaled = int(pi_over_2 * scale)  # Truncate
    pi_over_2_hex = f"0x{pi_over_2_scaled & 0xFFFFFFFFFFFFFFFF:016X}LL"
    for i in range(entries):
        x = mp.mpf(i) / (entries - 1)
        atan_x = mp.atan(x)
        scaled_value = int(atan_x * scale)  # Truncate instead of round

        # Format the value as a hexadecimal literal with LL suffix
        hex_value = f"0x{scaled_value & 0xFFFFFFFFFFFFFFFF:016X}LL"

        # Generate a shorter comment with just the first ~20 digits of precision
        # Convert to float first to avoid mpf formatting issues
        x_float = float(x)
        atan_x_float = float(atan_x)
        comment = f"// atan({x_float:.11f}) = {atan_x_float:.11f}"

        # Add the entry to the table
        if i < entries - 1:
            lines.append(f"    {hex_value},  {comment}")
        else:
            # For the last entry, add it twice to avoid index out of bounds
            lines.append(f"    {hex_value},  {comment}")
            lines.append(f"    {hex_value}   {comment}")

    # Close the table
    lines.append("};")
    lines.append("")

    # Add the LookupAtanFast function (renamed from original LookupAtan)
    lines.append(
        "// Fast lookup atan(x) with linear interpolation between table entries")
    lines.append(
        "// Input x is in fixed-point format with specified fraction bits representing a value in [-1,1]")
    lines.append(
        f"// Output is in fixed-point format with the same fraction bits representing atan(x)")
    lines.append("// Precision: ~3.1e-7 when fraction_bits=32")
    lines.append(
        "inline constexpr auto LookupAtanFast(int64_t x, int input_fraction_bits) noexcept -> int64_t {")
    lines.append("    // Constants")
    lines.append(
        f"    constexpr int kOutputFractionBits = {fraction_bits};  // Internal calculation format")
    lines.append("    constexpr int64_t kOne = 1LL << kOutputFractionBits;")
    lines.append("")

    lines.append("    // Handle negative input")
    lines.append("    bool is_negative = false;")
    lines.append("    if (x < 0) {")
    lines.append("        x = -x;")
    lines.append("        is_negative = true;")
    lines.append("    }")
    lines.append("")

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

    lines.append("    // 1. Ensure x is in [0,1] range")
    lines.append("    if (x <= 0) {")
    lines.append("        return 0;")
    lines.append("    }")

    lines.append("    bool use_reciprocal = false;")
    lines.append("    if (x >= kOne) {")
    lines.append("        // For x > 1, use atan(x) = π/2 - atan(1/x)")
    lines.append("        use_reciprocal = true;")
    lines.append("        // Calculate 1/x in fixed-point")
    lines.append(
        "        x = Primitives::Fixed64Div(kOne, x, kOutputFractionBits);")
    lines.append("    }")
    lines.append("")

    lines.append("    // 2. Scale x to table index")
    lines.append(
        "    constexpr int64_t kScale = static_cast<int64_t>(kAtanLut.size() - 2);")
    lines.append(
        "    const int64_t idx_scaled = Primitives::Fixed64Mul(x, kScale << kOutputFractionBits, kOutputFractionBits);")
    lines.append("    const int64_t idx = idx_scaled >> kOutputFractionBits;")
    lines.append(
        "    const int64_t t = idx_scaled & ((1LL << kOutputFractionBits) - 1);  // Fractional part [0,1)")
    lines.append("")

    lines.append("    // 4. Get table values for interpolation")
    lines.append("    const int64_t y0 = kAtanLut[idx];")
    lines.append("    const int64_t y1 = kAtanLut[idx + 1];")
    lines.append("")

    lines.append("    // 5. Linear interpolation")
    lines.append(
        "    int64_t result = y0 + (((y1 - y0) * t) >> kOutputFractionBits);")
    lines.append("")

    lines.append("    // Apply reciprocal formula if needed")
    lines.append("    if (use_reciprocal) {")
    lines.append("        // π/2 in our fixed-point format")
    lines.append(
        f"        constexpr int64_t kHalfPi = {pi_over_2_hex};  // pi/2 = {float(pi_over_2)}")
    lines.append("        result = kHalfPi - result;")
    lines.append("    }")
    lines.append("")

    lines.append("    // 6. Convert result back to input format if needed")
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

    lines.append("    // Apply sign")
    lines.append("    return is_negative ? -result : result;")
    lines.append("}")
    lines.append("")

    # Add a new quadratic interpolation version
    lines.append(
        "// High precision lookup atan(x) with quadratic interpolation between table entries")
    lines.append(
        "// Input x is in fixed-point format with specified fraction bits representing a value in [-1,1]")
    lines.append(
        f"// Output is in fixed-point format with the same fraction bits representing atan(x)")
    lines.append("// Precision: ~5.5e-10 when fraction_bits=32")
    lines.append(
        "inline constexpr auto LookupAtan(int64_t x, int input_fraction_bits) noexcept -> int64_t {")
    lines.append("    // Constants")
    lines.append(
        f"    constexpr int kOutputFractionBits = {fraction_bits};  // Internal calculation format")
    lines.append("    constexpr int64_t kOne = 1LL << kOutputFractionBits;")
    lines.append("")

    lines.append("    // Handle negative input")
    lines.append("    bool is_negative = false;")
    lines.append("    if (x < 0) {")
    lines.append("        x = -x;")
    lines.append("        is_negative = true;")
    lines.append("    }")
    lines.append("")

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

    lines.append("    // 1. Ensure x is in [0,1] range")
    lines.append("    if (x <= 0) {")
    lines.append("        return 0;")
    lines.append("    }")

    lines.append("    bool use_reciprocal = false;")
    lines.append("    if (x >= kOne) {")
    lines.append("        // For x > 1, use atan(x) = π/2 - atan(1/x)")
    lines.append("        use_reciprocal = true;")
    lines.append("        // Calculate 1/x in fixed-point")
    lines.append(
        "        x = Primitives::Fixed64Div(kOne, x, kOutputFractionBits);")
    lines.append("    }")
    lines.append("")

    lines.append("    // 2. Scale x to table index")
    lines.append(
        "    constexpr int64_t kScale = static_cast<int64_t>(kAtanLut.size() - 2);")
    lines.append(
        "    const int64_t idx_scaled = Primitives::Fixed64Mul(x, kScale << kOutputFractionBits, kOutputFractionBits);")
    lines.append("    const int64_t idx = idx_scaled >> kOutputFractionBits;")
    lines.append(
        "    const int64_t t = idx_scaled & ((1LL << kOutputFractionBits) - 1);  // Fractional part [0,1)")
    lines.append("")

    lines.append("    // 4. Get table values for quadratic interpolation")
    lines.append("    // Need three points: (x0,y0), (x1,y1), (x2,y2)")
    lines.append(
        "    const int64_t y1 = kAtanLut[idx];       // Current point")
    lines.append("    const int64_t y2 = kAtanLut[idx + 1];   // Next point")
    lines.append("    // For the previous point, handle the boundary case")
    lines.append("    int64_t y0;")
    lines.append("    if (idx > 0) {")
    lines.append("        y0 = kAtanLut[idx - 1];  // Previous point")
    lines.append("    } else {")
    lines.append(
        "        // At the boundary, mirror the slope for better extrapolation")
    lines.append("        y0 = y1 - (y2 - y1);")
    lines.append("    }")
    lines.append("")

    lines.append("    // 5. Quadratic interpolation")
    lines.append(
        "    // The correct Lagrange quadratic formula coefficients for points at (-1,y0), (0,y1), (1,y2):")
    lines.append("    // a = (y0 + y2)/2 - y1")
    lines.append("    // b = (y2 - y0)/2")
    lines.append("    // c = y1")
    lines.append("    // c = y0")
    lines.append("    const int64_t a = (y0 + y2)/2 - y1;")
    lines.append("    const int64_t b = (y2 - y0)/2;")
    lines.append("    const int64_t c = y1;")
    lines.append("")

    lines.append("    // Calculate polynomial a*t^2 + b*t + c")
    lines.append(
        "    const int64_t t_squared = Primitives::Fixed64Mul(t, t, kOutputFractionBits);")
    lines.append("    int64_t result = c;")
    lines.append(
        "    result += Primitives::Fixed64Mul(b, t, kOutputFractionBits);")
    lines.append(
        "    result += Primitives::Fixed64Mul(a, t_squared, kOutputFractionBits);")
    lines.append("")

    lines.append("    // Apply reciprocal formula if needed")
    lines.append("    if (use_reciprocal) {")
    lines.append("        // π/2 in our fixed-point format")
    lines.append(
        f"    constexpr int64_t kHalfPi = {pi_over_2_hex};  // pi/2 = {float(pi_over_2)}")
    lines.append("        result = kHalfPi - result;")
    lines.append("    }")
    lines.append("")

    lines.append("    // 6. Convert result back to input format if needed")
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

    lines.append("    // Apply sign")
    lines.append("    return is_negative ? -result : result;")
    lines.append("}")
    lines.append("")

    # Keep the original linear interpolation function
    lines.append(
        "// Fast lookup atan(x) with linear interpolation between table entries")
    lines.append(
        "// Input x is in fixed-point format with specified fraction bits representing a value in [-1,1]")
    lines.append(
        f"// Output is in fixed-point format with the same fraction bits representing atan(x)")

    lines.append("}  // namespace math::fp::detail")

    # Write to file or stdout
    if output_file:
        with open(output_file, "w", encoding="utf-8") as f:
            f.write("\n".join(lines))
    else:
        print("\n".join(lines))


if __name__ == "__main__":
    entries = 512  # Default number of entries
    fraction_bits = 32  # Default fraction bits (Q31.32 format)
    output_file = None

    # Parse command line arguments if provided
    if len(sys.argv) > 1:
        try:
            entries = int(sys.argv[1])
        except ValueError:
            # Not a number, assume it's a filename
            output_file = sys.argv[1]

    if len(sys.argv) > 2:
        try:
            entries = int(sys.argv[2])
        except ValueError:
            print(f"Error: Invalid number of entries: {sys.argv[2]}")
            sys.exit(1)

    if len(sys.argv) > 3:
        try:
            fraction_bits = int(sys.argv[3])
        except ValueError:
            print(f"Error: Invalid fraction bits: {sys.argv[3]}")
            sys.exit(1)

    # Generate the atan lookup table
    generate_atan_lut(output_file, entries, fraction_bits)
