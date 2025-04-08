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
    lines.append(f"// Covers the range [0,pi/2] with values in Q{int_bits}.{fraction_bits} format")
    lines.append(f"// Generated with mpmath library at {mp.mp.dps} digits precision")
    lines.append("")

    # Generate the table header
    lines.append("namespace math::fp::detail {")
    lines.append("// Table maps x in [0,pi/2] to tan(x)")
    lines.append(f"// Values stored in Q{int_bits}.{fraction_bits} fixed-point format")
    lines.append(f"inline constexpr std::array<int64_t, {lut_size}> kTanLut = {{")

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

        comment = f"// tan({angle_float:.11f}) = {tan_x_float:.11f}"

        # Add the entry with comment
        if i < lut_size - 1:
            lines.append(f"    {hex_value}, {comment}")
        else:
            lines.append(f"    {hex_value}  {comment}")

    lines.append("};")
    lines.append("")

    # Generate constants for the Tan lookup function
    lines.append("// Lookup tan(x) with linear interpolation between table entries")
    lines.append("// Input x is in fixed-point format with specified fraction bits representing angle in radians")
    lines.append(f"// Output is in Q{int_bits}.{fraction_bits} fixed-point format representing tan(x)")
    lines.append("inline constexpr auto LookupTan(int64_t x, int input_fraction_bits) noexcept -> int64_t {")
    lines.append("    // Constants")
    
    # Calculate constants in Q23.40 format with truncation
    pi = mp.pi
    pi_over_2 = pi / 2
    pi_scaled = int(pi * scale)  # Truncate
    pi_over_2_scaled = int(pi_over_2 * scale)  # Truncate
    lut_interval_scaled = int((lut_size - 1) / float(pi_over_2) * scale)  # Truncate
    
    pi_hex = f"0x{pi_scaled & 0xFFFFFFFFFFFFFFFF:016X}LL"
    pi_over_2_hex = f"0x{pi_over_2_scaled & 0xFFFFFFFFFFFFFFFF:016X}LL"
    lut_interval_hex = f"0x{lut_interval_scaled & 0xFFFFFFFFFFFFFFFF:016X}LL"
    
    lines.append(f"    constexpr int64_t kPi = {pi_hex};  // pi = {float(pi)}")
    lines.append(f"    constexpr int64_t kPiOver2 = {pi_over_2_hex};  // pi/2 = {float(pi_over_2)}")
    lines.append(f"    constexpr int64_t kLutInterval = {lut_interval_hex};  // LUT conversion factor")
    lines.append("    constexpr int64_t kMaxValue = INT64_MAX;  // Maximum representable value")
    lines.append(f"    constexpr int kOutputFractionBits = {fraction_bits};  // Output format: Q{int_bits}.{fraction_bits}")
    lines.append("")
    
    lines.append("    // Convert input to Q23.40 format if needed")
    lines.append("    if (input_fraction_bits != kOutputFractionBits) {")
    lines.append("        if (input_fraction_bits < kOutputFractionBits) {")
    lines.append("            x <<= (kOutputFractionBits - input_fraction_bits);")
    lines.append("        } else {")
    lines.append("            x >>= (input_fraction_bits - kOutputFractionBits);")
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
    lines.append("    int64_t raw_index = Primitives::Fixed64Mul(x, kLutInterval, kOutputFractionBits);")
    lines.append("    size_t index = static_cast<size_t>(raw_index >> kOutputFractionBits);")
    lines.append("    int64_t fraction = raw_index & ((1LL << kOutputFractionBits) - 1);")
    lines.append("")
    
    lines.append("    // 5. Handle index at or beyond the end of the table")
    lines.append("    if (index >= kTanLut.size() - 1) {")
    lines.append("        return flip ? -kMaxValue : kMaxValue;")
    lines.append("    }")
    lines.append("")
    
    lines.append("    // 6. Linear interpolation")
    lines.append("    int64_t a = kTanLut[index];")
    lines.append("    int64_t b = kTanLut[index + 1];")
    lines.append("    int64_t delta = Primitives::Fixed64Mul(fraction, b - a, kOutputFractionBits);")
    lines.append("    int64_t interpolated_value = a + delta;")
    lines.append("")
    
    lines.append("    // 7. Apply sign flip if necessary")
    lines.append("    int64_t result = flip ? -interpolated_value : interpolated_value;")
    lines.append("")
    
    lines.append("    // 8. Convert result back to original input format if needed")
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
        with open(output_file, "w") as f:
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