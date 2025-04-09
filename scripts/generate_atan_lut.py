import mpmath as mp
import sys

# Set very high precision
mp.mp.dps = 100


def generate_atan_lut(output_file=None, entries=512, fraction_bits=32):
    """Generate a lookup table for atan in the range [0,1]"""

    # Prepare the output with proper headers
    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <array>")
    lines.append("#include \"primitives.h\"")
    lines.append("")
    lines.append(f"// Atan lookup table with {entries} entries")
    lines.append(
        f"// Covers the range [0,1] with values in Q31.{fraction_bits} format")
    lines.append(
        f"// Generated with mpmath library at {mp.mp.dps} digits precision")
    lines.append("")

    # Generate the table header
    lines.append("namespace math::fp::detail {")
    lines.append("// Table maps x in [0,1] to atan(x)")
    lines.append(f"// Values stored in Q31.{fraction_bits} fixed-point format")
    lines.append(
        f"inline constexpr std::array<int64_t, {entries}> kAtanLut = {{")

    # Generate the table entries
    scale = mp.mpf(2) ** fraction_bits
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
        comment = f"// atan({x_float:.9f}) = {atan_x_float:.18f}"

        # Add the entry to the table
        if i < entries - 1:
            lines.append(f"    {hex_value},  {comment}")
        else:
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
    lines.append("// Precision: ~1.5e-5 when fraction_bits=32")
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
    lines.append("    if (x >= kOne) {")
    lines.append("        // Return the maximum value (atan(1) = pi/4)")
    lines.append("        int64_t result = kAtanLut[kAtanLut.size() - 1];")
    lines.append("        if (input_fraction_bits != kOutputFractionBits) {")
    lines.append(
        "            if (input_fraction_bits < kOutputFractionBits) {")
    lines.append(
        "                result >>= (kOutputFractionBits - input_fraction_bits);")
    lines.append("            } else {")
    lines.append(
        "                result <<= (input_fraction_bits - kOutputFractionBits);")
    lines.append("            }")
    lines.append("        }")
    lines.append("        return is_negative ? -result : result;")
    lines.append("    }")
    lines.append("")

    lines.append("    // 2. Scale x to table index")
    lines.append(
        "    const int64_t scale = static_cast<int64_t>(kAtanLut.size() - 1);")
    lines.append(
        "    const int64_t idx_scaled = Primitives::Fixed64Mul(x, scale << kOutputFractionBits, kOutputFractionBits);")
    lines.append("    const int64_t idx = idx_scaled >> kOutputFractionBits;")
    lines.append(
        "    const int64_t t = idx_scaled & ((1LL << kOutputFractionBits) - 1);  // Fractional part [0,1)")
    lines.append("")

    lines.append("    // 3. Clamp index to valid range")
    lines.append("    if (idx >= static_cast<int64_t>(kAtanLut.size()) - 1) {")
    lines.append("        // Return the maximum value (atan(1) = pi/4)")
    lines.append("        int64_t result = kAtanLut[kAtanLut.size() - 1];")
    lines.append("        if (input_fraction_bits != kOutputFractionBits) {")
    lines.append(
        "            if (input_fraction_bits < kOutputFractionBits) {")
    lines.append(
        "                result >>= (kOutputFractionBits - input_fraction_bits);")
    lines.append("            } else {")
    lines.append(
        "                result <<= (input_fraction_bits - kOutputFractionBits);")
    lines.append("            }")
    lines.append("        }")
    lines.append("        return is_negative ? -result : result;")
    lines.append("    }")
    lines.append("")

    lines.append("    // 4. Get table values for interpolation")
    lines.append("    const int64_t y0 = kAtanLut[idx];")
    lines.append("    const int64_t y1 = kAtanLut[idx + 1];")
    lines.append("")

    lines.append("    // 5. Linear interpolation")
    lines.append(
        "    int64_t result = y0 + Primitives::Fixed64Mul(y1 - y0, t, kOutputFractionBits);")
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
