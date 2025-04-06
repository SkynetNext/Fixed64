import mpmath as mp
import sys

# Set very high precision
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
    lines.append(f"// Covers the range [0,1] with values scaled by 2^{scale_bits}")
    lines.append(f"// Generated with mpmath library at {mp.mp.dps} digits precision")
    lines.append("")
    
    # Generate the table header
    lines.append(f"namespace math::fp::detail {{")
    lines.append(f"// Table maps x ∈ [0,1] to atan(x)")
    lines.append(f"// Values scaled by 2^{scale_bits}")
    lines.append(f"constexpr std::array<int64_t, {entries+1}> AtanLUT = {{")
    
    # Scale factor
    scale = mp.mpf(2) ** scale_bits
    
    # Generate table entries
    for i in range(entries + 1):  # +1 for the endpoint
        x = mp.mpf(i) / mp.mpf(entries)
        atan_x = mp.atan(x)
        scaled_value = int(atan_x * scale)
        
        # Format as hex for compactness and readability
        hex_val = f"0x{scaled_value & ((1 << 64) - 1):016x}LL"
        
        # Add comment with original values for verification
        comment = f"// atan({x}) = {atan_x}"
        
        # Add the entry to lines
        if i < entries:
            lines.append(f"    {hex_val},  {comment}")
        else:
            # Last entry without comma
            lines.append(f"    {hex_val}   {comment}")
    
    # Close the table
    lines.append("};")
    
    # Add utility function for linear interpolation lookup
    lines.append("""
// Lookup atan(x) with linear interpolation between table entries
inline constexpr int64_t LookupAtan(int64_t x, int precision) {
    // Ensure x is in [0,1] range scaled by 2^precision
    if (x <= 0)
        return 0;
    
    const int64_t one = static_cast<int64_t>(1) << precision;
    if (x >= one)
        return AtanLUT[AtanLUT.size() - 1] >> (63 - precision);
    
    // Scale x to table index
    const int64_t scale = static_cast<int64_t>(AtanLUT.size() - 1);
    const int64_t idx_scaled = (x * scale) >> precision;
    const int index = static_cast<int>(idx_scaled);
    
    // Get table values for interpolation
    const int64_t y0 = AtanLUT[index];
    const int64_t y1 = AtanLUT[index + 1];
    
    // Calculate fractional part for interpolation (keeping precision)
    const int64_t frac = (x * scale) - (idx_scaled << precision);
    
    // Linear interpolation
    int64_t result = y0 + ((y1 - y0) * frac >> precision);
    
    // Return scaled to requested precision
    return result >> (63 - precision);
}
""")
    
    lines.append("} // namespace math::fp::detail")
    
    # Output to file or stdout
    if output_file:
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write('\n'.join(lines))
        print(f"Table written to {output_file}")
    else:
        print('\n'.join(lines))

if __name__ == "__main__":
    entries = 512  # Default number of entries (increased from 256)
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