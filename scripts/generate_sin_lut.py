import mpmath as mp
import sys

# Set very high precision
mp.mp.dps = 100


def generate_sin_lut(output_file=None, entries=4096, fraction_bits=62):
    """Generate a lookup table for sin in the range [0,2π]"""

    # Prepare the output with proper headers
    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <array>")
    lines.append("#include \"primitives.h\"")
    lines.append("")
    lines.append(f"// Sin lookup table with {entries} entries")
    lines.append(
        f"// Covers the range [0,2pi] with values scaled by 2^{fraction_bits}")
    lines.append(
        f"// Generated with mpmath library at {mp.mp.dps} digits precision")
    lines.append("")

    # Generate the table header
    lines.append("namespace math::fp::detail {")
    lines.append("// Table maps x in [0,2pi] to sin(x)")
    lines.append(f"// Values scaled by 2^{fraction_bits}")
    lines.append(
        f"inline constexpr std::array<int64_t, {entries + 1}> kSinLut = {{")

    # Generate the table entries in Q1.62 format
    scale = mp.mpf(2) ** fraction_bits
    two_pi = mp.mpf(2) * mp.pi
    for i in range(entries + 1):
        x = (mp.mpf(i) / entries) * two_pi
        sin_x = mp.sin(x)
        scaled_value = int(sin_x * scale)

        # Format the value as a hexadecimal literal with LL suffix
        hex_value = f"0x{scaled_value & 0xFFFFFFFFFFFFFFFF:016X}LL"
        if scaled_value < 0:
            hex_value = f"-0x{-scaled_value & 0xFFFFFFFFFFFFFFFF:016X}LL"

        # Generate a comment showing the floating point representation
        x_float = float(x)
        sin_x_float = float(sin_x)
        comment = f"// sin({x_float:.18f}) = {sin_x_float:.18f}"

        # Add the entry with comment
        if i < entries:
            lines.append(f"    {hex_value}, {comment}")
        else:
            lines.append(f"    {hex_value}  {comment}")

    lines.append("};")
    lines.append("")

    # Generate the LookupSin function
    lines.append(
        "// Lookup sin(x) with linear interpolation between table entries")
    lines.append(
        "inline constexpr auto LookupSin(int64_t x, int fraction_bits) noexcept -> int64_t {")
    lines.append("    // Constants")

    # Calculate 1/(2pi) in Q0.62 format for kInvTwoPi
    inv_two_pi = mp.mpf(1) / (mp.mpf(2) * mp.pi)
    inv_two_pi_scaled = int(inv_two_pi * (mp.mpf(2) ** 62))
    inv_two_pi_hex = f"0x{inv_two_pi_scaled & 0xFFFFFFFFFFFFFFFF:016X}LL"

    lines.append(
        f"    constexpr int64_t kInvTwoPi = {inv_two_pi_hex};  // 1/(2pi) in Q0.62 format")
    lines.append(
        "    constexpr int kTablePrecision = 12;                  // 4096 entries = 2^12")
    lines.append(
        "    constexpr int kIndexShift = 50;                      // 62 - 12 = 50")
    lines.append("")
    lines.append(
        "    // Convert to normalized range [0,1) using fixed-point multiplication")
    lines.append(
        "    x = Primitives::Fixed64Mul(x, kInvTwoPi, fraction_bits);")
    lines.append("")
    lines.append("    int sign = 1;")
    lines.append("    if (x < 0) {")
    lines.append("        x = -x;")
    lines.append("        sign = -1;")
    lines.append("    }")
    lines.append("")
    lines.append("    // 2. Calculate lookup table index and fractional part")
    lines.append("    int index = static_cast<int>(x >> kIndexShift);")
    lines.append(
        "    int64_t fraction = (x & ((1LL << kIndexShift) - 1)) << kTablePrecision;")
    lines.append("")
    lines.append("    // 3. Linear interpolation")
    lines.append("    int64_t a = kSinLut[index];")
    lines.append("    int64_t b = kSinLut[index + 1];")
    lines.append("")
    lines.append(
        "    // Perform interpolation and adjust for requested precision")
    lines.append(
        "    int64_t result = a + Primitives::Fixed64Mul(b - a, fraction, 62);")
    lines.append("    return (result >> (62 - fraction_bits)) * sign;")
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
    entries = 4096  # Default number of entries (same as original)
    fraction_bits = 62  # Default scale factor (Q1.62 format)
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

    # Only call generate_sin_lut once with all parameters
    generate_sin_lut(output_file, entries, fraction_bits)
