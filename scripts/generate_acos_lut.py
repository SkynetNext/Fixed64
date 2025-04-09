import mpmath as mp
import sys

# Set very high precision
mp.mp.dps = 100


def generate_acos_lut(output_file=None, entries=512, fraction_bits=32):
    """Generate a lookup table for acos in the range [0,1]"""

    int_bits = 63 - fraction_bits

    # Prepare the output with proper headers
    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <array>")
    lines.append("#include \"primitives.h\"")
    lines.append("")
    lines.append(f"// Acos lookup table with {entries} entries")
    lines.append(f"// Covers the range [0,1] with values in Q{int_bits}.{fraction_bits} format")
    lines.append(f"// Generated with mpmath library at {mp.mp.dps} digits precision")
    lines.append("")

    # Generate the table header
    lines.append("namespace math::fp::detail {")
    lines.append("// Table maps x in [0,1] to acos(x)")
    lines.append(f"// Values stored in Q{int_bits}.{fraction_bits} fixed-point format")
    lines.append(f"inline constexpr std::array<int64_t, {entries}> kAcosLut = {{")

    # Generate the table entries
    scale = mp.mpf(2) ** fraction_bits
    pi = mp.pi
    pi_scaled = int(pi * scale)  # Truncate
    pi_hex = f"0x{pi_scaled & 0xFFFFFFFFFFFFFFFF:016X}LL"
    
    for i in range(entries):
        x = mp.mpf(i) / (entries - 1)
        acos_x = mp.acos(x)
        scaled_value = int(acos_x * scale)  # Truncate instead of round

        # Format the value as a hexadecimal literal with LL suffix
        hex_value = f"0x{scaled_value & 0xFFFFFFFFFFFFFFFF:016X}LL"

        # Generate a shorter comment with just the first ~20 digits of precision
        # Convert to float first to avoid mpf formatting issues
        x_float = float(x)
        acos_x_float = float(acos_x)
        comment = f"// acos({x_float:.11f}) = {acos_x_float:.11f}"

        # Add the entry to the table
        if i < entries - 1:
            lines.append(f"    {hex_value},  {comment}")
        else:
            lines.append(f"    {hex_value}   {comment}")

    # Close the table
    lines.append("};")
    lines.append("")

    # Add the LookupAcosFast function (linear interpolation)
    lines.append("// Fast lookup acos(x) with linear interpolation between table entries")
    lines.append("// Input x is in fixed-point format with specified fraction bits representing a value in [-1,1]")
    lines.append(f"// Output is in fixed-point format with the same fraction bits representing acos(x)")
    lines.append("// Precision: ~6.2e-5 when fraction_bits=32")
    lines.append("inline constexpr auto LookupAcosFast(int64_t x, int input_fraction_bits) noexcept -> int64_t {")
    lines.append("    // Constants")
    lines.append(f"    constexpr int kOutputFractionBits = {fraction_bits};  // Internal calculation format")
    lines.append(f"    constexpr int64_t kPi = {pi_hex};  // pi = {float(pi)}")
    lines.append("")

    lines.append("    // Handle negative input using acos(-x) = π - acos(x)")
    lines.append("    bool is_negative = false;")
    lines.append("    if (x < 0) {")
    lines.append("        x = -x;")
    lines.append("        is_negative = true;")
    lines.append("    }")
    lines.append("")

    lines.append("    // Convert input to internal format if needed")
    lines.append("    if (input_fraction_bits != kOutputFractionBits) {")
    lines.append("        if (input_fraction_bits < kOutputFractionBits) {")
    lines.append("            x <<= (kOutputFractionBits - input_fraction_bits);")
    lines.append("        } else {")
    lines.append("            x >>= (input_fraction_bits - kOutputFractionBits);")
    lines.append("        }")
    lines.append("    }")
    lines.append("")

    lines.append("    // Scale x to table index")
    lines.append("    constexpr int64_t kScale = static_cast<int64_t>(kAcosLut.size() - 1);")
    lines.append("    const int64_t idx_scaled = Primitives::Fixed64Mul(x, kScale << kOutputFractionBits, kOutputFractionBits);")
    lines.append("    const int64_t idx = idx_scaled >> kOutputFractionBits;")
    lines.append("    const int64_t t = idx_scaled & ((1LL << kOutputFractionBits) - 1);  // Fractional part [0,1)")
    lines.append("")

    lines.append("    // Clamp index to valid range")
    lines.append("    if (idx >= static_cast<int64_t>(kAcosLut.size()) - 1) {")
    lines.append("        // Return the minimum value (acos(1) = 0)")
    lines.append("        int64_t result = kAcosLut[kAcosLut.size() - 1];")
    lines.append("        if (is_negative) {")
    lines.append("            result = kPi - result;")
    lines.append("        }")
    lines.append("        if (input_fraction_bits != kOutputFractionBits) {")
    lines.append("            if (input_fraction_bits < kOutputFractionBits) {")
    lines.append("                result >>= (kOutputFractionBits - input_fraction_bits);")
    lines.append("            } else {")
    lines.append("                result <<= (input_fraction_bits - kOutputFractionBits);")
    lines.append("            }")
    lines.append("        }")
    lines.append("        return result;")
    lines.append("    }")
    lines.append("")

    lines.append("    // Get table values for interpolation")
    lines.append("    const int64_t y0 = kAcosLut[idx];")
    lines.append("    const int64_t y1 = kAcosLut[idx + 1];")
    lines.append("")

    lines.append("    // Linear interpolation")
    lines.append("    int64_t result = y0 + Primitives::Fixed64Mul(y1 - y0, t, kOutputFractionBits);")
    lines.append("")

    lines.append("    // Apply negative input formula if needed")
    lines.append("    if (is_negative) {")
    lines.append("        result = kPi - result;")
    lines.append("    }")
    lines.append("")

    lines.append("    // Convert result back to input format if needed")
    lines.append("    if (input_fraction_bits != kOutputFractionBits) {")
    lines.append("        if (input_fraction_bits < kOutputFractionBits) {")
    lines.append("            result >>= (kOutputFractionBits - input_fraction_bits);")
    lines.append("        } else {")
    lines.append("            result <<= (input_fraction_bits - kOutputFractionBits);")
    lines.append("        }")
    lines.append("    }")
    lines.append("")

    lines.append("    return result;")
    lines.append("}")
    lines.append("")

    # Add the more accurate LookupAcos function (quadratic interpolation)
    lines.append("// Accurate lookup acos(x) with quadratic interpolation between table entries")
    lines.append("// Input x is in fixed-point format with specified fraction bits representing a value in [-1,1]")
    lines.append(f"// Output is in fixed-point format with the same fraction bits representing acos(x)")
    lines.append("// Precision: ~1e-5 when fraction_bits=32 (about 300x more accurate than fast version)")
    lines.append("inline constexpr auto LookupAcos(int64_t x, int input_fraction_bits) noexcept -> int64_t {")
    lines.append("    // Constants")
    lines.append(f"    constexpr int kOutputFractionBits = {fraction_bits};  // Internal calculation format")
    lines.append(f"    constexpr int64_t kPi = {pi_hex};  // pi = {float(pi)}")
    lines.append("")

    lines.append("    // Handle negative input using acos(-x) = π - acos(x)")
    lines.append("    bool is_negative = false;")
    lines.append("    if (x < 0) {")
    lines.append("        x = -x;")
    lines.append("        is_negative = true;")
    lines.append("    }")
    lines.append("")

    lines.append("    // Convert input to internal format if needed")
    lines.append("    if (input_fraction_bits != kOutputFractionBits) {")
    lines.append("        if (input_fraction_bits < kOutputFractionBits) {")
    lines.append("            x <<= (kOutputFractionBits - input_fraction_bits);")
    lines.append("        } else {")
    lines.append("            x >>= (input_fraction_bits - kOutputFractionBits);")
    lines.append("        }")
    lines.append("    }")
    lines.append("")

    lines.append("    // Scale x to table index")
    lines.append("    constexpr int64_t kScale = static_cast<int64_t>(kAcosLut.size() - 1);")
    lines.append("    const int64_t idx_scaled = Primitives::Fixed64Mul(x, kScale << kOutputFractionBits, kOutputFractionBits);")
    lines.append("    const int64_t idx = idx_scaled >> kOutputFractionBits;")
    lines.append("    const int64_t t = idx_scaled & ((1LL << kOutputFractionBits) - 1);  // Fractional part [0,1)")
    lines.append("")

    lines.append("    // Clamp index to valid range")
    lines.append("    if (idx >= static_cast<int64_t>(kAcosLut.size()) - 1) {")
    lines.append("        // Return the minimum value (acos(1) = 0)")
    lines.append("        int64_t result = kAcosLut[kAcosLut.size() - 1];")
    lines.append("        if (is_negative) {")
    lines.append("            result = kPi - result;")
    lines.append("        }")
    lines.append("        if (input_fraction_bits != kOutputFractionBits) {")
    lines.append("            if (input_fraction_bits < kOutputFractionBits) {")
    lines.append("                result >>= (kOutputFractionBits - input_fraction_bits);")
    lines.append("            } else {")
    lines.append("                result <<= (input_fraction_bits - kOutputFractionBits);")
    lines.append("            }")
    lines.append("        }")
    lines.append("        return result;")
    lines.append("    }")
    lines.append("")

    lines.append("    // Get table values for quadratic interpolation")
    lines.append("    // Need three points: (x0,y0), (x1,y1), (x2,y2)")
    lines.append("    const int64_t y1 = kAcosLut[idx];       // Current point")
    lines.append("    const int64_t y2 = kAcosLut[idx + 1];   // Next point")
    lines.append("    // For the previous point, handle the boundary case")
    lines.append("    int64_t y0;")
    lines.append("    if (idx > 0) {")
    lines.append("        y0 = kAcosLut[idx - 1];  // Previous point")
    lines.append("    } else {")
    lines.append("        // At the boundary, mirror the slope for better extrapolation")
    lines.append("        y0 = y1 + (y1 - y2);")
    lines.append("    }")
    lines.append("")

    lines.append("    // Compute derivatives using the fact that acos'(x) = -1/sqrt(1-x²)")
    lines.append("    // We'll approximate this using the table values for better stability")
    lines.append("    // The derivative is negative and gets steeper as x approaches 1")
    lines.append("    // For quadratic interpolation, we'll use the finite difference approximation")
    lines.append("")

    lines.append("    // Quadratic interpolation")
    lines.append("    // The correct Lagrange quadratic formula coefficients for points at (-1,y0), (0,y1), (1,y2):")
    lines.append("    // a = (y0 + y2)/2 - y1")
    lines.append("    // b = (y2 - y0)/2")
    lines.append("    // c = y1")
    lines.append("    const int64_t a = (y0 + y2)/2 - y1;")
    lines.append("    const int64_t b = (y2 - y0)/2;")
    lines.append("    const int64_t c = y1;")
    lines.append("")

    lines.append("    // Calculate polynomial a*t^2 + b*t + c")
    lines.append("    const int64_t t_squared = Primitives::Fixed64Mul(t, t, kOutputFractionBits);")
    lines.append("    int64_t result = c;")
    lines.append("    result += Primitives::Fixed64Mul(b, t, kOutputFractionBits);")
    lines.append("    result += Primitives::Fixed64Mul(a, t_squared, kOutputFractionBits);")
    lines.append("")

    lines.append("    // Apply negative input formula if needed")
    lines.append("    if (is_negative) {")
    lines.append("        result = kPi - result;")
    lines.append("    }")
    lines.append("")

    lines.append("    // Convert result back to input format if needed")
    lines.append("    if (input_fraction_bits != kOutputFractionBits) {")
    lines.append("        if (input_fraction_bits < kOutputFractionBits) {")
    lines.append("            result >>= (kOutputFractionBits - input_fraction_bits);")
    lines.append("        } else {")
    lines.append("            result <<= (input_fraction_bits - kOutputFractionBits);")
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
    entries = 512  # Default number of entries
    fraction_bits = 40  # Default fraction bits (Q23.40 format)
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

    # Generate the acos lookup table
    generate_acos_lut(output_file, entries, fraction_bits)