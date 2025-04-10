import mpmath as mp
import sys

# Set very high precision
mp.mp.dps = 100


def generate_acos_lut(output_file=None, entries=512, fraction_bits=32):
    """Generate a lookup table for acos in the range [0,1] using Chebyshev nodes"""

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
    lines.append(f"// Using Chebyshev nodes for optimal approximation accuracy")
    lines.append("")

    # Generate the table header
    lines.append("namespace math::fp::detail {")
    lines.append("// Table maps x in [0,1] to acos(x) using Chebyshev nodes distribution")
    lines.append(f"// Values stored in Q{int_bits}.{fraction_bits} fixed-point format")
    lines.append(f"inline constexpr std::array<int64_t, {entries}> kAcosLut = {{")

    # Generate the array of x values using Chebyshev nodes
    x_values = []
    for i in range(entries):
        # Chebyshev nodes formula: cos(π(2i+1)/(2n))
        theta = mp.pi * (2*i + 1) / (2*entries)
        # Nodes are in [-1,1], map to [0,1]
        x = (mp.cos(theta) + 1) / 2
        x_values.append(x)
    
    # Sort the x values from 1 to 0 (reverse order) so acos goes from 0 to pi/2
    # This matches the behavior of the original table
    x_values.reverse()

    # Generate the table entries
    scale = mp.mpf(2) ** fraction_bits
    pi = mp.pi
    pi_scaled = int(pi * scale)  # Truncate
    pi_hex = f"0x{pi_scaled & 0xFFFFFFFFFFFFFFFF:016X}LL"
    
    for i, x in enumerate(x_values):
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

    # Store the x values for lookup
    lines.append(f"// X values corresponding to the Chebyshev nodes")
    lines.append(f"inline constexpr std::array<int64_t, {entries}> kAcosXValues = {{")
    
    for i, x in enumerate(x_values):
        scaled_x = int(x * scale)  # Truncate
        hex_x = f"0x{scaled_x & 0xFFFFFFFFFFFFFFFF:016X}LL"
        
        if i < entries - 1:
            lines.append(f"    {hex_x},  // {float(x):.11f}")
        else:
            lines.append(f"    {hex_x}   // {float(x):.11f}")
    
    lines.append("};")
    lines.append("")

    # Corrected helper function for binary search
    lines.append("// Helper function for binary search in the LUT")
    lines.append("// X values are ordered from lowest (0.0) to highest (1.0)")
    lines.append("inline constexpr auto FindAcosIndex(int64_t x, int output_fraction_bits) noexcept -> int {")
    lines.append("    // Handle out-of-range values")
    lines.append("    const int table_size = kAcosXValues.size();")
    lines.append("    const int64_t one_in_fp = int64_t(1) << output_fraction_bits;")
    lines.append("    ")
    lines.append("    // x ≥ 1.0: Return last index (acos(1) = 0)")
    lines.append("    if (x >= one_in_fp) {")
    lines.append("        return table_size - 1;")
    lines.append("    }")
    lines.append("    ")
    lines.append("    // x ≤ 0.0: Return index 0 (acos(0) = π/2)")
    lines.append("    if (x <= 0) {")
    lines.append("        return 0;")
    lines.append("    }")
    lines.append("    ")
    lines.append("    // Binary search - for x values ordered from smallest to largest")
    lines.append("    int left = 0;")
    lines.append("    int right = table_size - 1;")
    lines.append("    ")
    lines.append("    while (left <= right) {")
    lines.append("        int mid = left + (right - left) / 2;")
    lines.append("        if (kAcosXValues[mid] < x) {")
    lines.append("            // Value is smaller (to the left in normal order)")
    lines.append("            left = mid + 1;")
    lines.append("        } else if (kAcosXValues[mid] > x) {")
    lines.append("            // Value is larger (to the right in normal order)")
    lines.append("            right = mid - 1;")
    lines.append("        } else {")
    lines.append("            // Exact match")
    lines.append("            return mid;")
    lines.append("        }")
    lines.append("    }")
    lines.append("    ")
    lines.append("    // No exact match - right is now the index of the element just below x")
    lines.append("    return right;")
    lines.append("}")
    lines.append("")

    # Add the LookupAcosFast function (binary search + linear interpolation)
    lines.append("// Fast lookup acos(x) with binary search and linear interpolation")
    lines.append("// Input x is in fixed-point format with specified fraction bits representing a value in [-1,1]")
    lines.append(f"// Output is in fixed-point format with the same fraction bits representing acos(x)")
    lines.append("// Precision is improved over uniform sampling due to Chebyshev node distribution")
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

    lines.append("    // Handle extreme values directly")
    lines.append("    const int64_t one_in_fp = int64_t(1) << kOutputFractionBits;")
    lines.append("    if (x >= one_in_fp) {")
    lines.append("        // |x| ≥ 1.0, return 0 for positive x or π for negative x")
    lines.append("        int64_t result = is_negative ? kPi : 0;")
    lines.append("        ")
    lines.append("        // Convert result to input format if needed")
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

    # Replace the binary search with the common function
    lines.append("    // Find the index for interpolation using binary search")
    lines.append("    const int idx = FindAcosIndex(x, kOutputFractionBits);")
    lines.append("")

    lines.append("    // Handle exact match")
    lines.append("    if (kAcosXValues[idx] == x) {")
    lines.append("        int64_t result = kAcosLut[idx];")
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

    lines.append("    // Get the adjacent indices for interpolation")
    lines.append("    const int idx_next = idx + 1;")
    lines.append("    if (idx_next >= static_cast<int>(kAcosXValues.size())) {")
    lines.append("        // We're at the last point, return it directly")
    lines.append("        int64_t result = kAcosLut[idx];")
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

    lines.append("    // Linear interpolation")
    lines.append("    const int64_t x0 = kAcosXValues[idx];")
    lines.append("    const int64_t x1 = kAcosXValues[idx_next];")
    lines.append("    const int64_t y0 = kAcosLut[idx];")
    lines.append("    const int64_t y1 = kAcosLut[idx_next];")
    lines.append("")
    
    lines.append("    // Calculate the interpolation parameter t")
    lines.append("    // t = (x - x0) / (x1 - x0)")
    lines.append("    int64_t t = Primitives::Fixed64Div(x - x0, x1 - x0, kOutputFractionBits);")
    lines.append("")
    
    lines.append("    // Linear interpolation: y = y0 + t * (y1 - y0)")
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

    # Add the more accurate LookupAcos function (binary search + quadratic interpolation)
    lines.append("// Accurate lookup acos(x) with binary search and quadratic interpolation")
    lines.append("// Input x is in fixed-point format with specified fraction bits representing a value in [-1,1]")
    lines.append(f"// Output is in fixed-point format with the same fraction bits representing acos(x)")
    lines.append("// Precision is significantly improved due to Chebyshev node distribution and quadratic interpolation")
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

    lines.append("    // Handle extreme values directly")
    lines.append("    const int64_t one_in_fp = int64_t(1) << kOutputFractionBits;")
    lines.append("    if (x >= one_in_fp) {")
    lines.append("        // |x| ≥ 1.0, return 0 for positive x or π for negative x")
    lines.append("        int64_t result = is_negative ? kPi : 0;")
    lines.append("        ")
    lines.append("        // Convert result to input format if needed")
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

    # Replace the binary search with the common function
    lines.append("    // Find the index for interpolation using binary search")
    lines.append("    const int idx = FindAcosIndex(x, kOutputFractionBits);")
    lines.append("")

    lines.append("    // Handle exact match")
    lines.append("    if (kAcosXValues[idx] == x) {")
    lines.append("        int64_t result = kAcosLut[idx];")
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
    lines.append("    // Need three points for quadratic interpolation")
    lines.append("    int64_t y1 = kAcosLut[idx];       // Current point")
    lines.append("    // For right point (idx+1)")
    lines.append("    int64_t y2;")
    lines.append("    if (idx < static_cast<int>(kAcosLut.size()) - 1) {")
    lines.append("        y2 = kAcosLut[idx + 1];   // Next point")
    lines.append("    } else {")
    lines.append("        // At rightmost boundary, extrapolate")
    lines.append("        y2 = y1 * 2 - kAcosLut[idx - 1];  // Mirror the slope")
    lines.append("    }")
    lines.append("    // For left point (idx-1)")
    lines.append("    int64_t y0;")
    lines.append("    if (idx > 0) {")
    lines.append("        y0 = kAcosLut[idx - 1];  // Previous point")
    lines.append("    } else {")
    lines.append("        // At leftmost boundary, extrapolate")
    lines.append("        y0 = y1 + (y1 - y2);  // Mirror the slope for better extrapolation")
    lines.append("    }")
    lines.append("")

    lines.append("    // Get x values for interpolation")
    lines.append("    int64_t x1 = kAcosXValues[idx];")
    lines.append("    int64_t x0 = (idx > 0) ? kAcosXValues[idx - 1] : x1 + (x1 - kAcosXValues[idx + 1]);")
    lines.append("    int64_t x2 = (idx < static_cast<int>(kAcosLut.size()) - 1) ? kAcosXValues[idx + 1] : x1 - (kAcosXValues[idx - 1] - x1);")
    lines.append("")

    lines.append("    // Compute Lagrange basis polynomials")
    lines.append("    // L0(x) = ((x-x1)*(x-x2))/((x0-x1)*(x0-x2))")
    lines.append("    // L1(x) = ((x-x0)*(x-x2))/((x1-x0)*(x1-x2))")
    lines.append("    // L2(x) = ((x-x0)*(x-x1))/((x2-x0)*(x2-x1))")
    lines.append("    ")
    lines.append("    // For fixed-point arithmetic, we'll calculate these step by step")
    lines.append("    const int64_t x_minus_x0 = x - x0;")
    lines.append("    const int64_t x_minus_x1 = x - x1;")
    lines.append("    const int64_t x_minus_x2 = x - x2;")
    lines.append("    ")
    lines.append("    const int64_t x0_minus_x1 = x0 - x1;")
    lines.append("    const int64_t x0_minus_x2 = x0 - x2;")
    lines.append("    const int64_t x1_minus_x0 = x1 - x0;")
    lines.append("    const int64_t x1_minus_x2 = x1 - x2;")
    lines.append("    const int64_t x2_minus_x0 = x2 - x0;")
    lines.append("    const int64_t x2_minus_x1 = x2 - x1;")
    lines.append("    ")
    lines.append("    // Calculate L0, L1, L2 with careful fixed-point operations")
    lines.append("    int64_t L0_num = Primitives::Fixed64Mul(x_minus_x1, x_minus_x2, kOutputFractionBits);")
    lines.append("    int64_t L0_den = Primitives::Fixed64Mul(x0_minus_x1, x0_minus_x2, kOutputFractionBits);")
    lines.append("    int64_t L0 = Primitives::Fixed64Div(L0_num, L0_den, kOutputFractionBits);")
    lines.append("    ")
    lines.append("    int64_t L1_num = Primitives::Fixed64Mul(x_minus_x0, x_minus_x2, kOutputFractionBits);")
    lines.append("    int64_t L1_den = Primitives::Fixed64Mul(x1_minus_x0, x1_minus_x2, kOutputFractionBits);")
    lines.append("    int64_t L1 = Primitives::Fixed64Div(L1_num, L1_den, kOutputFractionBits);")
    lines.append("    ")
    lines.append("    int64_t L2_num = Primitives::Fixed64Mul(x_minus_x0, x_minus_x1, kOutputFractionBits);")
    lines.append("    int64_t L2_den = Primitives::Fixed64Mul(x2_minus_x0, x2_minus_x1, kOutputFractionBits);")
    lines.append("    int64_t L2 = Primitives::Fixed64Div(L2_num, L2_den, kOutputFractionBits);")
    lines.append("    ")

    lines.append("    // Compute the quadratic interpolation")
    lines.append("    int64_t result = Primitives::Fixed64Mul(y0, L0, kOutputFractionBits) +")
    lines.append("                     Primitives::Fixed64Mul(y1, L1, kOutputFractionBits) +")
    lines.append("                     Primitives::Fixed64Mul(y2, L2, kOutputFractionBits);")
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