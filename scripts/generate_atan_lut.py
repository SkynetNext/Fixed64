import mpmath as mp
import sys

# Set very high fraction_bits
mp.mp.dps = 100


def generate_atan_lut(output_file=None, entries=512, scale_bits=63):
    """Generate a lookup table for atan in the range [0,1]"""

    # Prepare the output with proper headers
    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <array>")
    lines.append("")
    lines.append(f"// Atan lookup table with {entries} entries")
    lines.append(
        f"// Covers the range [0,1] with values scaled by 2^{scale_bits}")
    lines.append(
        f"// Generated with mpmath library at {mp.mp.dps} digits fraction_bits")
    lines.append("")

    # Generate the table header
    lines.append("namespace math::fp::detail {")
    lines.append("// Table maps x in [0,1] to atan(x)")
    lines.append(f"// Values scaled by 2^{scale_bits}")
    lines.append(
        f"inline constexpr std::array<int64_t, {entries + 1}> kAtanLut = {{")

    # Generate the table entries
    scale = mp.mpf(2) ** scale_bits
    for i in range(entries + 1):
        x = mp.mpf(i) / entries
        atan_x = mp.atan(x)
        scaled_value = int(atan_x * scale)

        # Format the value as a hexadecimal literal with LL suffix
        hex_value = f"0x{scaled_value:016X}LL"

        # Generate a shorter comment with just the first ~20 digits of fraction_bits
        # Convert to float first to avoid mpf formatting issues
        x_float = float(x)
        atan_x_float = float(atan_x)
        comment = f"// atan({x_float:.9f}) = {atan_x_float:.18f}"

        # Add the entry to the table
        if i < entries:
            lines.append(f"    {hex_value},  {comment}")
        else:
            lines.append(f"    {hex_value}   {comment}")

    # Close the table
    lines.append("};")
    lines.append("")

    # Add the LookupAtan function
    lines.append(
        "// Lookup atan(x) with linear interpolation between table entries")
    lines.append(
        "inline constexpr auto LookupAtan(int64_t x, int fraction_bits) noexcept -> int64_t {")
    lines.append("    // Ensure x is in [0,1] range scaled by 2^fraction_bits")
    lines.append("    if (x <= 0)")
    lines.append("        return 0;")
    lines.append("    ")
    lines.append(
        "    const int64_t one = static_cast<int64_t>(1) << fraction_bits;")
    lines.append("    if (x >= one)")
    lines.append(
        f"        return kAtanLut[kAtanLut.size() - 1] >> ({scale_bits} - fraction_bits);")
    lines.append("    ")
    lines.append("    // Scale x to table index")
    lines.append(
        f"    const int64_t scale = static_cast<int64_t>(kAtanLut.size() - 1);")
    lines.append(
        "    const int64_t idx_scaled = (x * scale) >> fraction_bits;")
    lines.append("    const int index = static_cast<int>(idx_scaled);")
    lines.append("    ")
    lines.append("    // Get table values for interpolation")
    lines.append("    const int64_t y0 = kAtanLut[index];")
    lines.append("    const int64_t y1 = kAtanLut[index + 1];")
    lines.append("    ")
    lines.append(
        "    // Calculate fractional part for interpolation (keeping fraction_bits)")
    lines.append(
        "    const int64_t frac = (x * scale) - (idx_scaled << fraction_bits);")
    lines.append("    ")
    lines.append("    // Linear interpolation")
    lines.append(
        "    int64_t result = y0 + ((y1 - y0) * frac >> fraction_bits);")
    lines.append("    ")
    lines.append("    // Return scaled to requested fraction_bits")
    lines.append(f"    return result >> ({scale_bits} - fraction_bits);")
    lines.append("}")
    lines.append("")

    lines.append("}  // namespace math::fp::detail")

    # Write to file or stdout
    if output_file:
        with open(output_file, "w") as f:
            f.write("\n".join(lines))
    else:
        print("\n".join(lines))


if __name__ == "__main__":
    entries = 512  # Default number of entries
    scale_bits = 63  # Default scale factor (standard for Fixed64)
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
            scale_bits = int(sys.argv[3])
        except ValueError:
            print(f"Error: Invalid scale bits: {sys.argv[3]}")
            sys.exit(1)

    # Only call generate_atan_lut once with all parameters
    generate_atan_lut(output_file, entries, scale_bits)
