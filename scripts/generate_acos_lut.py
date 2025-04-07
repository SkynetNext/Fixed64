import mpmath as mp
import sys

# Set very high precision
mp.mp.dps = 100


def generate_acos_lut(output_file=None, entries=4096, fraction_bits=62):
    """Generate a lookup table for acos in the range [-1,1]"""

    # Prepare the output with proper headers
    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <array>")
    lines.append("#include \"primitives.h\"")
    lines.append("")
    lines.append(f"// Acos lookup table with {entries} entries")
    lines.append(
        f"// Covers the range [-1,1] with values scaled by 2^{fraction_bits}")
    lines.append(
        f"// Generated with mpmath library at {mp.mp.dps} digits precision")
    lines.append("")

    # Generate the table header
    lines.append("namespace math::fp::detail {")
    lines.append("// Table maps x in [-1,1] to acos(x)")
    lines.append(f"// Values scaled by 2^{fraction_bits}")
    lines.append(
        f"inline constexpr std::array<int64_t, {entries + 1}> kAcosLut = {{")

    # Generate the table entries in Q1.62 format
    scale = mp.mpf(2) ** fraction_bits

    # For acos, we map the range [-1, 1] to table entries
    for i in range(entries + 1):
        x = mp.mpf(2) * (mp.mpf(i) / entries) - \
            mp.mpf(1)  # Maps [0,entries] to [-1,1]
        acos_x = mp.acos(x)
        scaled_value = int(acos_x * scale)

        # Format the value as a hexadecimal literal with LL suffix
        hex_value = f"0x{scaled_value & 0xFFFFFFFFFFFFFFFF:016X}LL"
        if scaled_value < 0:
            hex_value = f"-0x{-scaled_value & 0xFFFFFFFFFFFFFFFF:016X}LL"

        # Generate a comment showing the floating point representation and Q1.62 value
        x_float = float(x)
        acos_x_float = float(acos_x)
        q_value = scaled_value / (2**fraction_bits)

        comment = f"// acos({x_float:.6f}) = {acos_x_float:.6f} (Q1.62: {q_value:.20f})"

        # Add the entry with comment
        if i < entries:
            lines.append(f"    {hex_value}, {comment}")
        else:
            lines.append(f"    {hex_value}  {comment}")

    lines.append("};")
    lines.append("")

    # Generate the LookupAcos function
    lines.append(
        "// Lookup acos(x) with linear interpolation between table entries")
    lines.append(
        "inline constexpr auto LookupAcos(int64_t x, int fraction_bits) noexcept -> int64_t {")
    lines.append("    // Constants")
    lines.append(
        f"    constexpr int64_t kOne = 1LL << 62;  // 1.0 in Q1.62 format")
    lines.append(
        "    constexpr int kTablePrecision = 12;                  // 4096 entries = 2^12")
    lines.append(
        f"    constexpr int kIndexShift = 50;   // 62 - 12 = 50")
    lines.append("")
    lines.append("    // Clamp input to [-1, 1] range")
    lines.append("    if (x >= kOne)")
    lines.append("        return 0;")
    lines.append("    if (x <= -kOne)")
    lines.append(
        f"        return 0xC90FDAA22168C234LL;  // pi in Q1.62 format")
    lines.append("")
    lines.append("    // Map from [-1, 1] to [0, entries]")
    lines.append(
        "    int64_t normalized = ((x + kOne) * (kAcosLut.size() - 1)) >> (62 + 1);")
    lines.append("")
    lines.append("    // Calculate lookup table index and fractional part")
    lines.append(
        "    int index = static_cast<int>(normalized >> kIndexShift);")
    lines.append(
        "    int64_t fraction = (normalized & ((1LL << kIndexShift) - 1)) << kTablePrecision;")
    lines.append("")
    lines.append("    // Linear interpolation")
    lines.append("    int64_t a = kAcosLut[index];")
    lines.append("    int64_t b = kAcosLut[index + 1];")
    lines.append("")
    lines.append(
        "    // Perform interpolation and adjust for requested precision")
    lines.append(
        f"    int64_t result = a + Primitives::Fixed64Mul(b - a, fraction, 62);")
    lines.append(f"    return (result >> ({62} - fraction_bits));")
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

    # Only call generate_acos_lut once with all parameters
    generate_acos_lut(output_file, entries, fraction_bits)
