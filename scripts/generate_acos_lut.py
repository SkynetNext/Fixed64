import numpy as np
import math
import sys

def generate_acos_lut(output_file="acos_lut.h"):
    """Generate arccosine lookup table with high precision segmented approach"""
    
    # Fixed-point precision constants
    P = 32  # 32 fractional bits
    ONE = 1 << P  # 1.0 in Q32 format
    PI = int(math.pi * ONE)  # π in Q32 format
    
    # Initialize arrays for storing lookup table values
    lut = []
    
    # Region 1: 0.0-0.8 uniform distribution (512 points)
    num_points1 = 512
    for i in range(num_points1 + 1):
        x = 0.8 * i / num_points1
        y = math.acos(x)
        lut.append(int(y * ONE))
    
    # Region 2: 0.8-0.95 Hermite interpolation (64 segments)
    num_segments = 64
    step = 0.15 / num_segments
    
    for seg in range(num_segments + 1):
        x0 = 0.8 + seg * step
        y0 = math.acos(x0)
        dy_dx = -(1.0 / math.sqrt(1.0 - x0*x0))
        
        lut.append(int(x0 * ONE))
        lut.append(int(y0 * ONE))
        lut.append(int(dy_dx * ONE))
    
    # Region 3: 0.95-0.99 denser uniform distribution (512 points)
    num_points3 = 512
    for i in range(num_points3 + 1):
        x = 0.95 + 0.04 * i / num_points3
        y = math.acos(x)
        lut.append(int(y * ONE))
    
    # Region 4: 0.99-0.999 even denser (512 points)
    num_points4 = 512
    for i in range(num_points4 + 1):
        x = 0.99 + 0.009 * i / num_points4
        y = math.acos(x)
        lut.append(int(y * ONE))
    
    # Region 5: 0.999-1.0 densest (256 points)
    num_points5 = 256
    for i in range(num_points5 + 1):
        x = 0.999 + 0.001 * i / num_points5
        y = math.acos(x) if x < 1.0 else 0.0
        lut.append(int(y * ONE))
    
    # Write to header file
    with open(output_file, "w") as f:
        f.write("#pragma once\n\n")
        f.write("#include <array>\n")
        f.write("#include <cstdint>\n")
        f.write("#include <algorithm>\n")
        f.write("#include \"primitives.h\"\n\n")
        f.write("namespace math::fp::detail {\n\n")
        
        # Write table as std::array
        f.write(f"// Arccosine lookup table with {len(lut)} entries using multi-region approach\n")
        f.write("// Region 1: 0.0-0.8 uniform (512+1 points)\n")
        f.write("// Region 2: 0.8-0.95 Hermite interpolation (64 segments = 195 points)\n")
        f.write("// Region 3: 0.95-0.99 denser uniform (512+1 points)\n")
        f.write("// Region 4: 0.99-0.999 even denser (512+1 points)\n")
        f.write("// Region 5: 0.999-1.0 densest (256+1 points)\n")
        f.write(f"// Fixed-point format: Q{64-P}.{P}\n")
        f.write(f"inline constexpr std::array<int64_t, {len(lut)}> AcosLut = {{\n    ")
        
        # Format the values in rows of 4
        for i, val in enumerate(lut):
            if i > 0 and i % 4 == 0:
                f.write("\n    ")
            f.write(f"{val}LL")
            if i < len(lut) - 1:
                f.write(", ")
        
        f.write("\n};\n\n")
        
        # Write fixed-point constants
        f.write("// Fixed-point constants\n")
        f.write("constexpr int FRACTION_BITS = 32;\n")
        f.write(f"constexpr int64_t ONE = 1LL << FRACTION_BITS;\n")
        f.write(f"constexpr int64_t PI = {PI}LL;  // π in Q{64-P}.{P} format (π × 2^{P})\n\n")
        
        # Write the LookupAcos function directly based on the C++ example
        f.write("/**\n")
        f.write(" * @brief Calculate arccosine value with multi-region interpolation\n")
        f.write(" * @param x Fixed-point value in [-1,1] range with input_fraction_bits precision\n")
        f.write(" * @param input_fraction_bits Precision (fractional bits) of the input value\n")
        f.write(" * @return Fixed-point arccosine value with input_fraction_bits precision in [0, π] range\n")
        f.write(" */\n")
        f.write("inline int64_t constexpr LookupAcos(int64_t x, int input_fraction_bits) noexcept {\n")
        f.write("    // Region boundary constants\n")
        f.write("    constexpr int64_t THRESHOLD_0_8 = ONE * 4LL / 5LL;         // 0.8\n")
        f.write("    constexpr int64_t THRESHOLD_0_95 = ONE * 19LL / 20LL;      // 0.95\n")
        f.write("    constexpr int64_t THRESHOLD_0_99 = ONE * 99LL / 100LL;     // 0.99\n")
        f.write("    constexpr int64_t THRESHOLD_0_999 = ONE * 999LL / 1000LL;  // 0.999\n")
        f.write("    constexpr int64_t THRESHOLD_SMALL = ONE - (ONE >> 16);     // 0.999984741211\n\n")
        
        f.write("    // Adjust input to internal precision\n")
        f.write("    int64_t scaled_x;\n")
        f.write("    if (input_fraction_bits > FRACTION_BITS) {\n")
        f.write("        scaled_x = x >> (input_fraction_bits - FRACTION_BITS);\n")
        f.write("    } else if (input_fraction_bits < FRACTION_BITS) {\n")
        f.write("        scaled_x = x << (FRACTION_BITS - input_fraction_bits);\n")
        f.write("    } else {\n")
        f.write("        scaled_x = x;\n")
        f.write("    }\n\n")
        
        f.write("    // Boundary check: ensure input is in [-ONE, ONE] range\n")
        f.write("    if (scaled_x >= ONE) {\n")
        f.write("        int64_t result = 0;\n")
        f.write("        // Adjust output precision\n")
        f.write("        if (input_fraction_bits > FRACTION_BITS) {\n")
        f.write("            result = result << (input_fraction_bits - FRACTION_BITS);\n")
        f.write("        } else if (input_fraction_bits < FRACTION_BITS) {\n")
        f.write("            result = result >> (FRACTION_BITS - input_fraction_bits);\n")
        f.write("        }\n")
        f.write("        return result;\n")
        f.write("    }\n")
        f.write("    if (scaled_x <= -ONE) {\n")
        f.write("        int64_t result = PI;\n")
        f.write("        // Adjust output precision\n")
        f.write("        if (input_fraction_bits > FRACTION_BITS) {\n")
        f.write("            result = result << (input_fraction_bits - FRACTION_BITS);\n")
        f.write("        } else if (input_fraction_bits < FRACTION_BITS) {\n")
        f.write("            result = result >> (FRACTION_BITS - input_fraction_bits);\n")
        f.write("        }\n")
        f.write("        return result;\n")
        f.write("    }\n\n")
        
        f.write("    bool is_negative = scaled_x < 0;\n")
        f.write("    scaled_x = is_negative ? -scaled_x : scaled_x;\n\n")
        
        f.write("    // Handle extremely small angles: x > 0.999984741211, use sqrt(2(1-x)) approximation\n")
        f.write("    if (scaled_x > THRESHOLD_SMALL) {\n")
        f.write("        int64_t epsilon = ONE - scaled_x;\n")
        f.write("        int64_t sqrt_input = (epsilon << 1);\n")
        f.write("        int64_t result = Primitives::Fixed64SqrtFast(sqrt_input, FRACTION_BITS);\n")
        f.write("        \n")
        f.write("        // Adjust for negative input\n")
        f.write("        if (is_negative) {\n")
        f.write("            result = PI - result;\n")
        f.write("        }\n")
        f.write("        \n")
        f.write("        // Adjust output precision\n")
        f.write("        if (input_fraction_bits > FRACTION_BITS) {\n")
        f.write("            result = result << (input_fraction_bits - FRACTION_BITS);\n")
        f.write("        } else if (input_fraction_bits < FRACTION_BITS) {\n")
        f.write("            result = result >> (FRACTION_BITS - input_fraction_bits);\n")
        f.write("        }\n")
        f.write("        return result;\n")
        f.write("    }\n\n")
        
        f.write("    int64_t result;\n")
        f.write("    // Region 1: [0, 0.8], use 512-point uniform interpolation\n")
        f.write("    if (scaled_x < THRESHOLD_0_8) {\n")
        f.write("        int index = (scaled_x << 9) / THRESHOLD_0_8;  // x * 512 / (0.8 * ONE)\n")
        f.write("        index = std::min(index, 511);\n\n")
        
        f.write("        // Calculate interpolation\n")
        f.write("        int64_t x0 = (index * THRESHOLD_0_8) >> 9;  // index * 0.8 * ONE / 512\n")
        f.write("        int64_t dx = scaled_x - x0;\n")
        f.write("        constexpr int64_t delta = THRESHOLD_0_8 >> 9;  // 0.8 * ONE / 512\n")
        f.write("        result = AcosLut[index] + ((AcosLut[index + 1] - AcosLut[index]) * dx) / delta;\n")
        f.write("    }\n")
        
        f.write("    // Region 2: [0.8, 0.95], use 64-segment Hermite interpolation\n")
        f.write("    else if (scaled_x < THRESHOLD_0_95) {\n")
        f.write("        int seg = ((scaled_x - THRESHOLD_0_8) * 64LL) / (ONE * 3LL / 20LL);  // (x - 0.8) / (0.15/64)\n")
        f.write("        seg = std::min(seg, 63);\n\n")
        
        f.write("        int base_idx = 513 + seg * 3;\n")
        f.write("        int64_t x0 = AcosLut[base_idx];\n")
        f.write("        int64_t y0 = AcosLut[base_idx + 1];\n")
        f.write("        int64_t dydx = AcosLut[base_idx + 2];\n\n")
        
        f.write("        int64_t dx = scaled_x - x0;\n")
        f.write("        result = y0 + ((dydx * dx) >> FRACTION_BITS);\n")
        f.write("    }\n")
        
        f.write("    // Region 3: [0.95, 0.99], use 512-point linear interpolation\n")
        f.write("    else if (scaled_x < THRESHOLD_0_99) {\n")
        f.write("        int base_idx = 513 + 195;              // 512 + 1 + 65*3\n")
        f.write("        int64_t rel_x = scaled_x - THRESHOLD_0_95;  // x - 0.95\n")
        f.write("        int64_t scale = (ONE * 4LL / 100LL);   // 0.04 * ONE\n\n")
        
        f.write("        int index = (rel_x * 512LL) / scale;\n")
        f.write("        index = std::min(index, 511);\n\n")
        
        f.write("        int idx = base_idx + index;\n")
        f.write("        int64_t x1 = THRESHOLD_0_95 + (scale * index) / 512LL;\n")
        f.write("        int64_t x2 = THRESHOLD_0_95 + (scale * (index + 1)) / 512LL;\n\n")
        
        f.write("        int64_t alpha = ((scaled_x - x1) << FRACTION_BITS) / (x2 - x1);\n")
        f.write("        result = ((AcosLut[idx] * (ONE - alpha)) + (AcosLut[idx + 1] * alpha)) >> FRACTION_BITS;\n")
        f.write("    }\n")
        
        f.write("    // Region 4: [0.99, 0.999], use 512-point linear interpolation\n")
        f.write("    else if (scaled_x < THRESHOLD_0_999) {\n")
        f.write("        int base_idx = 513 + 195 + 513;\n")
        f.write("        int64_t rel_x = scaled_x - THRESHOLD_0_99;  // x - 0.99\n")
        f.write("        int64_t scale = (ONE * 9LL / 1000LL);  // 0.009 * ONE\n\n")
        
        f.write("        int index = (rel_x * 512LL) / scale;\n")
        f.write("        index = std::min(index, 511);\n\n")
        
        f.write("        int idx = base_idx + index;\n")
        f.write("        int64_t x1 = THRESHOLD_0_99 + (scale * index) / 512LL;\n")
        f.write("        int64_t x2 = THRESHOLD_0_99 + (scale * (index + 1)) / 512LL;\n\n")
        
        f.write("        int64_t alpha = ((scaled_x - x1) << FRACTION_BITS) / (x2 - x1);\n")
        f.write("        result = ((AcosLut[idx] * (ONE - alpha)) + (AcosLut[idx + 1] * alpha)) >> FRACTION_BITS;\n")
        f.write("    }\n")
        
        f.write("    // Region 5: [0.999, 1.0), use 256-point linear interpolation\n")
        f.write("    else {\n")
        f.write("        int base_idx = 513 + 195 + 513 + 513;\n")
        f.write("        int64_t rel_x = scaled_x - THRESHOLD_0_999;  // x - 0.999\n")
        f.write("        int64_t scale = ONE / 1000LL;           // 0.001 * ONE\n\n")
        
        f.write("        int index = (rel_x * 256LL) / scale;\n")
        f.write("        index = std::min(index, 255);\n\n")
        
        f.write("        int idx = base_idx + index;\n")
        f.write("        int64_t x1 = THRESHOLD_0_999 + (scale * index) / 256LL;\n")
        f.write("        int64_t x2 = THRESHOLD_0_999 + (scale * (index + 1)) / 256LL;\n\n")
        
        f.write("        int64_t alpha = ((scaled_x - x1) << FRACTION_BITS) / (x2 - x1);\n")
        f.write("        result = ((AcosLut[idx] * (ONE - alpha)) + (AcosLut[idx + 1] * alpha)) >> FRACTION_BITS;\n")
        f.write("    }\n\n")
        
        f.write("    // Adjust for negative input\n")
        f.write("    if (is_negative) {\n")
        f.write("        result = PI - result;\n")
        f.write("    }\n\n")
        
        f.write("    // Adjust output precision\n")
        f.write("    if (input_fraction_bits > FRACTION_BITS) {\n")
        f.write("        result = result << (input_fraction_bits - FRACTION_BITS);\n")
        f.write("    } else if (input_fraction_bits < FRACTION_BITS) {\n")
        f.write("        result = result >> (FRACTION_BITS - input_fraction_bits);\n")
        f.write("    }\n\n")
        
        f.write("    return result;\n")
        f.write("}\n\n")
        
        f.write("} // namespace math::fp::detail\n")
    
    print(f"Generated acos lookup table with {len(lut)} entries in {output_file}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        generate_acos_lut(sys.argv[1])
    else:
        generate_acos_lut()