import mpmath as mp
import sys

# Set high precision
mp.mp.dps = 100

def generate_cordic_table(output_file=None, iterations=32, scale_bits=63):
    """Generate CORDIC angle table for atan2 implementation"""
    
    # Prepare the output with proper headers
    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <array>")
    lines.append("")
    lines.append(f"// CORDIC angle table with {iterations} entries")
    lines.append(f"// Contains atan(2^-i) values for i=0...{iterations-1}")
    lines.append(f"// Values scaled by 2^{scale_bits}")
    lines.append(f"// Generated with mpmath library at {mp.mp.dps} digits precision")
    lines.append("")
    
    # Generate the table header
    lines.append(f"namespace math::fp {{")
    lines.append(f"namespace detail {{")
    lines.append(f"// Table contains atan(2^-i) values for CORDIC algorithm")
    lines.append(f"// Values scaled by 2^{scale_bits}")
    lines.append(f"constexpr std::array<int64_t, {iterations}> CordicTable = {{")
    
    # Scale factor
    scale = mp.mpf(2) ** scale_bits
    
    # Generate table entries
    for i in range(iterations):
        angle = mp.atan(mp.mpf(1) / mp.mpf(2**i))
        scaled_value = int(angle * scale)
        
        # Format as hex for compactness and readability
        hex_val = f"0x{scaled_value & ((1 << 64) - 1):016x}LL"
        
        # Add comment with original values for verification
        comment = f"// atan(2^-{i}) = {angle}"
        
        # Add the entry to lines
        if i < iterations - 1:
            lines.append(f"    {hex_val},  {comment}")
        else:
            # Last entry without comma
            lines.append(f"    {hex_val}   {comment}")
    
    # Close the table
    lines.append("};")
    lines.append("} // namespace detail")
    lines.append("} // namespace math::fp")
    
    # Output to file or stdout
    if output_file:
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write('\n'.join(lines))
        print(f"CORDIC table written to {output_file}")
    else:
        print('\n'.join(lines))

if __name__ == "__main__":
    iterations = 32  # Default number of iterations
    scale_bits = 63  # Default scale factor (standard for Fixed64)
    output_file = None
    
    # Parse command line arguments if provided
    if len(sys.argv) > 1:
        try:
            iterations = int(sys.argv[1])
        except ValueError:
            # Not a number, assume it's a filename
            output_file = sys.argv[1]
    
    if len(sys.argv) > 2:
        try:
            iterations = int(sys.argv[2])
        except ValueError:
            print(f"Error: Invalid number of iterations: {sys.argv[2]}")
            sys.exit(1)
    
    if len(sys.argv) > 3:
        try:
            scale_bits = int(sys.argv[3])
        except ValueError:
            print(f"Error: Invalid scale bits: {sys.argv[3]}")
            sys.exit(1)
    
    generate_cordic_table(output_file, iterations, scale_bits) 