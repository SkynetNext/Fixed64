import numpy as np
from mpmath import mp, mpf
import matplotlib.pyplot as plt

# Set precision
mp.dps = 50  # 50 decimal places of precision

def generate_atan2_lut(table_size=256, output_file="atan2_lut.h"):
    """Generate arctangent lookup table for atan2 implementation with high precision"""
    
    # Generate input values from 0 to 1 (first octant)
    inputs = np.linspace(0, 1, table_size)
    
    # Calculate atan values with high precision (0 to pi/4)
    atan_values = []
    for x in inputs:
        atan_x = float(mp.atan(mpf(x)))
        atan_values.append(atan_x)
    
    # Convert to fixed-point representation (Q31.32 format)
    P = 32  # 32 fractional bits
    fixed_values = []
    for val in atan_values:
        fixed_val = int(val * (1 << P))
        fixed_values.append(fixed_val)
    
    # Add an extra entry that's a copy of the last item (257th entry)
    fixed_values.append(fixed_values[-1])
    
    # Write to header file
    with open(output_file, "w") as f:
        f.write("#pragma once\n\n")
        f.write("#include <array>\n")
        f.write("#include <cstdint>\n\n")
        f.write("namespace math::fp::detail {\n\n")
        
        # Write table as std::array with inline
        f.write(f"// Arctangent lookup table with {table_size + 1} entries for atan2 implementation\n")
        f.write(f"// Input range: [0, 1], Output: atan(x) in radians [0, pi/4]\n")
        f.write(f"// Fixed-point format: Q31.32\n")
        f.write(f"inline constexpr std::array<int64_t, {table_size + 1}> kAtan2LUT = {{\n")
        
        # Format the values with comments indicating actual values
        for i, val in enumerate(fixed_values):
            if i < table_size:
                x = inputs[i]
                atan_x = atan_values[i]
                f.write(f"    {val}LL,  // ratio={x:.11f}, angle={atan_x:.11f}\n")
            else:
                # Last entry (copy of the last value)
                x = 1.0
                atan_x = atan_values[-1]
                f.write(f"    {val}LL   // ratio={x:.11f}, angle={atan_x:.11f}\n")
        
        f.write("};\n\n")
        
        # Write lookup function with trailing return type
        f.write("/**\n")
        f.write(" * @brief Lookup arctangent value for atan2 implementation with linear interpolation\n")
        f.write(" * @param ratio Fixed-point ratio value (y/x or x/y) in [0,1] range\n")
        f.write(" * @param P Precision (fractional bits) of the input\n")
        f.write(" * @return Fixed-point arctangent value with P fractional bits in [0, pi/4] range\n")
        f.write(" */\n")
        f.write("inline auto LookupAtan2(int64_t ratio, int P) noexcept -> int64_t {\n")
        f.write("    // Scale input to [0, 1] range in Q31.32 format\n")
        f.write("    constexpr int kTableP = 32;\n")
        f.write("    int64_t scaled_x;\n")
        f.write("    if (P > kTableP) {\n")
        f.write("        scaled_x = ratio >> (P - kTableP);\n")
        f.write("    } else if (P < kTableP) {\n")
        f.write("        scaled_x = ratio << (kTableP - P);\n")
        f.write("    } else {\n")
        f.write("        scaled_x = ratio;\n")
        f.write("    }\n\n")
        
        f.write("    // Ensure input is in valid range\n")
        f.write("    constexpr int64_t kOne = 1LL << kTableP;\n")
        f.write("    if (scaled_x >= kOne) {\n")
        f.write("        scaled_x = kOne - 1;\n")
        f.write("    }\n\n")
        
        f.write(f"    // Calculate table index and fractional part\n")
        f.write(f"    constexpr int kTableSize = {table_size};\n")
        f.write(f"    constexpr int64_t kIndexScale = (1LL << kTableP) / (kTableSize - 1);\n")
        f.write(f"    int index = static_cast<int>(scaled_x / kIndexScale);\n")
        f.write(f"    int64_t frac = scaled_x % kIndexScale;\n\n")
        
        f.write(f"    // Perform linear interpolation\n")
        f.write(f"    int64_t y0 = kAtan2LUT[index];\n")
        f.write(f"    int64_t y1 = kAtan2LUT[index + 1];\n")
        f.write(f"    int64_t result = y0 + ((y1 - y0) * frac) / kIndexScale;\n\n")
        
        f.write(f"    // Adjust precision if needed\n")
        f.write(f"    if (P > kTableP) {{\n")
        f.write(f"        result = result << (P - kTableP);\n")
        f.write(f"    }} else if (P < kTableP) {{\n")
        f.write(f"        result = result >> (kTableP - P);\n")
        f.write(f"    }}\n\n")
        
        f.write(f"    return result;\n")
        f.write(f"}}\n\n")
        
        f.write("} // namespace math::fp::detail\n")
    
    print(f"Generated atan2 lookup table with {table_size + 1} entries in {output_file}")
    
    # Plot the table for verification
    plt.figure(figsize=(10, 6))
    plt.plot(inputs, atan_values)
    plt.title('Arctangent Lookup Table for Atan2')
    plt.xlabel('Input (ratio=y/x)')
    plt.ylabel('Angle (radians)')
    plt.grid(True)
    plt.savefig('atan2_lut.png')
    plt.show()

if __name__ == "__main__":
    generate_atan2_lut(256, "atan2_lut.h")