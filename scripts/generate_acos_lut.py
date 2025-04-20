import numpy as np
import math
import sys

def generate_acos_lut(output_file="acos_lut.h"):
    """Generate arccosine lookup table with high precision segmented approach"""
    
    # Fixed-point precision constants
    P = 32  # 32 fractional bits
    ONE = 1 << P  # 1.0 in Q32 format
    PI = int(math.pi * ONE)  # π in Q32 format
    
    # Define region size constants early
    kRegion1Size = 257  # 256 + 1
    kRegion2Size = 258  # (128 + 1) * 2
    kRegion3Size = 257  # 256 + 1
    kRegion4Size = 257  # 256 + 1
    kRegion5Size = 257  # 256 + 1
    
    # Initialize arrays for storing lookup table values
    lut = []
    dydx_lut = []  # New array for derivatives in region 2
    
    # Region 1: 0.0-0.8 uniform distribution (256 points)
    num_points1 = 256
    for i in range(num_points1 + 1):
        x = 0.8 * i / num_points1
        y = math.acos(x)
        lut.append(int(y * ONE))
    
    # Region 2: 0.8-0.93 Hermite interpolation (128 segments)
    num_segments = 128
    step = 0.13 / num_segments
    
    for seg in range(num_segments + 1):
        x0 = 0.8 + seg * step
        y0 = math.acos(x0)
        dy_dx = -(1.0 / math.sqrt(1.0 - x0*x0))
        
        lut.append(int(x0 * ONE))
        lut.append(int(y0 * ONE))
        dydx_lut.append(int(dy_dx * ONE))  # Store derivatives in separate array
    
    # Region 3: 0.93-0.99 denser uniform distribution (256 points)
    num_points3 = 256
    for i in range(num_points3 + 1):
        x = 0.93 + 0.06 * i / num_points3
        y = math.acos(x)
        lut.append(int(y * ONE))
    
    # Region 4: 0.99-0.999 even denser (256 points)
    num_points4 = 256
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
        f.write("// Region 1: 0.0-0.8 uniform (256+1 points)\n")
        f.write("// Region 2: 0.8-0.93 Hermite interpolation (128 segments = 258 points, with derivatives in separate array)\n")
        f.write("// Region 3: 0.93-0.99 denser uniform (256+1 points)\n")
        f.write("// Region 4: 0.99-0.999 even denser (256+1 points)\n")
        f.write("// Region 5: 0.999-1.0 densest (256+1 points)\n")
        f.write(f"// Fixed-point format: Q{63-P}.{P}\n")
        f.write(f"inline constexpr std::array<int64_t, {len(lut)}> AcosLut = {{\n    ")
        
        # Write the values with each entry on its own line, including region markers
        for i, val in enumerate(lut):
            # Add region marker comments
            if i == 0:
                f.write("// Region 1: 0.0-0.8 uniform (256+1 points)\n")
            elif i == kRegion1Size:
                f.write("// Region 2: 0.8-0.93 Hermite interpolation (128 segments = 258 points)\n")
            elif i == kRegion1Size + kRegion2Size:
                f.write("// Region 3: 0.93-0.99 denser uniform (256+1 points)\n")
            elif i == kRegion1Size + kRegion2Size + kRegion3Size:
                f.write("// Region 4: 0.99-0.999 even denser (256+1 points)\n")
            elif i == kRegion1Size + kRegion2Size + kRegion3Size + kRegion4Size:
                f.write("// Region 5: 0.999-1.0 densest (256+1 points)\n")
            
            # For Region 1 (0.0-0.8)
            if i <= num_points1:
                x = 0.8 * i / num_points1
                y = math.acos(x)
                f.write(f"{val}LL, // acos({x:.10f}) = {y:.10f}")
            
            # For Region 2 (0.8-0.93 Hermite interpolation)
            elif i < kRegion1Size + kRegion2Size:
                # Region 2 alternates between x and y values
                rel_i = i - kRegion1Size
                if rel_i % 2 == 0:  # x value
                    seg = rel_i // 2
                    x = 0.8 + seg * step
                    f.write(f"{val}LL, // x = {x:.10f}")
                else:  # y value
                    seg = (rel_i - 1) // 2
                    x = 0.8 + seg * step
                    y = math.acos(x)
                    f.write(f"{val}LL, // acos({x:.10f}) = {y:.10f}")
            
            # For Region 3 (0.93-0.99)
            elif i < kRegion1Size + kRegion2Size + kRegion3Size:
                rel_i = i - (kRegion1Size + kRegion2Size)
                x = 0.93 + 0.06 * rel_i / num_points3
                y = math.acos(x)
                f.write(f"{val}LL, // acos({x:.10f}) = {y:.10f}")
            
            # For Region 4 (0.99-0.999)
            elif i < kRegion1Size + kRegion2Size + kRegion3Size + kRegion4Size:
                rel_i = i - (kRegion1Size + kRegion2Size + kRegion3Size)
                x = 0.99 + 0.009 * rel_i / num_points4
                y = math.acos(x)
                f.write(f"{val}LL, // acos({x:.10f}) = {y:.10f}")
            
            # For Region 5 (0.999-1.0)
            else:
                rel_i = i - (kRegion1Size + kRegion2Size + kRegion3Size + kRegion4Size)
                x = 0.999 + 0.001 * rel_i / num_points5
                y = math.acos(x) if x < 1.0 else 0.0
                f.write(f"{val}LL, // acos({x:.10f}) = {y:.10f}")
            
            # Add a newline after each entry
            if i < len(lut) - 1:
                f.write("\n")
        
        f.write("\n};\n\n")
        
        # Write the derivatives lookup table with comments
        f.write(f"// Derivatives for Region 2 (0.8-0.93)\n")
        f.write(f"inline constexpr std::array<int64_t, {len(dydx_lut)}> AcosDyDxLut = {{\n    ")
        
        # Write the AcosDyDxLut table with each entry on its own line
        for i, val in enumerate(dydx_lut):
            x = 0.8 + i * step
            dy_dx = -(1.0 / math.sqrt(1.0 - x*x))
            f.write(f"{val}LL, // d(acos)/dx at x={x:.10f} = {dy_dx:.10f}")
            
            if i < len(dydx_lut) - 1:
                f.write("\n")
        
        f.write("\n};\n\n")
        
        # Write the LookupAcos function directly based on the C++ example
        f.write("/**\n")
        f.write(" * @brief Calculate arccosine value with multi-region interpolation\n")
        f.write(" * @param x Fixed-point value in [-1,1] range with input_fraction_bits precision\n")
        f.write(" * @param input_fraction_bits Precision (fractional bits) of the input value\n")
        f.write(" * @return Fixed-point arccosine value with input_fraction_bits precision in [0, pi] range\n")
        f.write(" */\n")
        f.write("inline int64_t LookupAcos(int64_t x, int input_fraction_bits) noexcept {\n")
        f.write("    // Fixed-point constants\n")
        f.write("    constexpr int kFractionBits = 32;\n")
        f.write(f"    constexpr int64_t kOne = 1LL << kFractionBits;\n")
        f.write(f"    constexpr int64_t kPi = {PI}LL;  // pi in Q{64-P}.{P} format (pi * 2^{P})\n\n")
    
        
        f.write("    // Region boundary constants\n")
        f.write("    constexpr int64_t kThreshold_0_8 = kOne * 4LL / 5LL;         // 0.8\n")
        f.write("    constexpr int64_t kThreshold_0_93 = kOne * 93LL / 100LL;      // 0.93\n")
        f.write("    constexpr int64_t kThreshold_0_99 = kOne * 99LL / 100LL;     // 0.99\n")
        f.write("    constexpr int64_t kThreshold_0_999 = kOne * 999LL / 1000LL;  // 0.999\n")
        f.write("    constexpr int64_t kThresholdSmall = kOne - (kOne >> 16);     // 0.999984741211\n\n")
        
        f.write("    // Region size constants\n")
        f.write("    constexpr int kRegion1Size = 257;  // 256 + 1\n")
        f.write("    constexpr int kRegion2Size = 258;  // (128 + 1) * 2 (x and y values only)\n")
        f.write("    constexpr int kRegion3Size = 257;  // 256 + 1\n")
        f.write("    constexpr int kRegion4Size = 257;  // 256 + 1\n\n")

        f.write("    // Pre-computed multipliers for optimized index calculation\n")
        f.write("    constexpr int64_t kInvThreshold_0_8 = (1LL << (kFractionBits + 8)) / (kOne * 4LL / 5LL);\n")
        f.write("    constexpr int64_t kInvRange_2 = (1LL << kFractionBits) * 128LL / (kOne * 13LL / 100LL);\n")
        f.write("    constexpr int64_t kInvScale_3 = (1LL << (kFractionBits + 8)) / (kOne * 6LL / 100LL);\n")
        f.write("    constexpr int64_t kInvScale_4 = (1LL << (kFractionBits + 8)) / (kOne * 9LL / 1000LL);\n")
        f.write("    constexpr int64_t kInvScale_5 = (1LL << (kFractionBits + 8)) / (kOne / 1000LL);\n\n")
        
        f.write("    // Adjust input to internal precision\n")
        f.write("    int64_t scaled_x;\n")
        f.write("    if (input_fraction_bits > kFractionBits) {\n")
        f.write("        scaled_x = x >> (input_fraction_bits - kFractionBits);\n")
        f.write("    } else if (input_fraction_bits < kFractionBits) {\n")
        f.write("        scaled_x = x << (kFractionBits - input_fraction_bits);\n")
        f.write("    } else {\n")
        f.write("        scaled_x = x;\n")
        f.write("    }\n\n")
        
        f.write("    // Boundary check: ensure input is in [-kOne, kOne] range\n")
        f.write("    if (scaled_x >= kOne) {\n")
        f.write("        int64_t result = 0;\n")
        f.write("        // Adjust output precision\n")
        f.write("        if (input_fraction_bits > kFractionBits) {\n")
        f.write("            result = result << (input_fraction_bits - kFractionBits);\n")
        f.write("        } else if (input_fraction_bits < kFractionBits) {\n")
        f.write("            result = result >> (kFractionBits - input_fraction_bits);\n")
        f.write("        }\n")
        f.write("        return result;\n")
        f.write("    }\n")
        f.write("    if (scaled_x <= -kOne) {\n")
        f.write("        int64_t result = kPi;\n")
        f.write("        // Adjust output precision\n")
        f.write("        if (input_fraction_bits > kFractionBits) {\n")
        f.write("            result = result << (input_fraction_bits - kFractionBits);\n")
        f.write("        } else if (input_fraction_bits < kFractionBits) {\n")
        f.write("            result = result >> (kFractionBits - input_fraction_bits);\n")
        f.write("        }\n")
        f.write("        return result;\n")
        f.write("    }\n\n")
        
        f.write("    bool is_negative = scaled_x < 0;\n")
        f.write("    scaled_x = is_negative ? -scaled_x : scaled_x;\n\n")
        
        f.write("    // Handle extremely small angles: x > 0.999984741211, use sqrt(2(1-x)) approximation\n")
        f.write("    if (scaled_x > kThresholdSmall) {\n")
        f.write("        int64_t epsilon = kOne - scaled_x;\n")
        f.write("        int64_t sqrt_input = (epsilon << 1);\n")
        f.write("        int64_t result = Primitives::Fixed64SqrtFast(sqrt_input, kFractionBits);\n")
        f.write("        \n")
        f.write("        // Adjust for negative input\n")
        f.write("        if (is_negative) {\n")
        f.write("            result = kPi - result;\n")
        f.write("        }\n")
        f.write("        \n")
        f.write("        // Adjust output precision\n")
        f.write("        if (input_fraction_bits > kFractionBits) {\n")
        f.write("            result = result << (input_fraction_bits - kFractionBits);\n")
        f.write("        } else if (input_fraction_bits < kFractionBits) {\n")
        f.write("            result = result >> (kFractionBits - input_fraction_bits);\n")
        f.write("        }\n")
        f.write("        return result;\n")
        f.write("    }\n\n")
        
        f.write("    int64_t result;\n")
        f.write("    // Region 1: [0, 0.8], use 256-point uniform interpolation\n")
        f.write("    if (scaled_x < kThreshold_0_8) {\n")
        f.write("        constexpr int kShift = 8;  // log2(256)\n")
        f.write("        // Optimized index calculation: multiply by pre-computed inverse instead of dividing\n")
        f.write("        int index = (scaled_x * kInvThreshold_0_8) >> kFractionBits;  // x * 256 / (0.8 * kOne)\n")
        f.write("        \n")
        f.write("        // Calculate interpolation\n")
        f.write("        int64_t x0 = (index * kThreshold_0_8) >> kShift;  // index * 0.8 * kOne / 256\n")
        f.write("        int64_t dx = scaled_x - x0;\n")
        f.write("        constexpr int64_t kDelta = kThreshold_0_8 >> kShift;  // 0.8 * kOne / 256\n")
        f.write("        result = AcosLut[index] + ((AcosLut[index + 1] - AcosLut[index]) * dx) / kDelta;\n")
        f.write("    }\n")
        
        f.write("    // Region 2: [0.8, 0.93], use 128-segment Hermite interpolation\n")
        f.write("    else if (scaled_x < kThreshold_0_93) {\n")
        f.write("        // Optimized segment calculation: multiply by pre-computed inverse instead of dividing\n")
        f.write("        int seg = ((scaled_x - kThreshold_0_8) * kInvRange_2) >> kFractionBits;  // (x - 0.8) / (0.13/128)\n")
        f.write("        \n")
        f.write("        constexpr int kPointsPerSegment = 2;  // Only x and y in main array (derivative in separate array)\n")
        f.write("        int base_idx = kRegion1Size + seg * kPointsPerSegment;\n")
        f.write("        int64_t x0 = AcosLut[base_idx];\n")
        f.write("        int64_t y0 = AcosLut[base_idx + 1];\n")
        f.write("        int64_t dydx = AcosDyDxLut[seg];  // Use derivative from separate array\n\n")
        
        f.write("        int64_t dx = scaled_x - x0;\n")
        f.write("        result = y0 + ((dydx * dx) >> kFractionBits);\n")
        f.write("    }\n")
        
        f.write("    // Region 3: [0.93, 0.99], use 256-point linear interpolation\n")
        f.write("    else if (scaled_x < kThreshold_0_99) {\n")
        f.write("        constexpr int base_idx = kRegion1Size + kRegion2Size;\n")
        f.write("        int64_t rel_x = scaled_x - kThreshold_0_93;  // x - 0.93\n")
        f.write("        constexpr int64_t kScale = kOne * 6LL / 100LL;   // 0.06 * kOne\n\n")
        f.write("        constexpr int kShift = 8;  // log2(256)\n")
        f.write("        // Optimized index calculation: multiply by pre-computed inverse instead of dividing\n")
        f.write("        int index = (rel_x * kInvScale_3) >> kFractionBits;  // rel_x * 256 / (0.06 * kOne)\n")
        f.write("        \n")
        f.write("        int idx = base_idx + index;\n")
        f.write("        int64_t x1 = kThreshold_0_93 + ((kScale * index) >> kShift);  // 0.93 + (0.06 * index / 256)\n")
        f.write("        int64_t x2 = kThreshold_0_93 + ((kScale * (index + 1)) >> kShift);\n\n")
        
        f.write("        int64_t alpha = ((scaled_x - x1) << kFractionBits) / (x2 - x1);\n")
        f.write("        result = ((AcosLut[idx] * (kOne - alpha)) + (AcosLut[idx + 1] * alpha)) >> kFractionBits;\n")
        f.write("    }\n")
        
        f.write("    // Region 4: [0.99, 0.999], use 256-point linear interpolation\n")
        f.write("    else if (scaled_x < kThreshold_0_999) {\n")
        f.write("        constexpr int base_idx = kRegion1Size + kRegion2Size + kRegion3Size;\n")
        f.write("        int64_t rel_x = scaled_x - kThreshold_0_99;  // x - 0.99\n")
        f.write("        constexpr int64_t kScale = kOne * 9LL / 1000LL;  // 0.009 * kOne\n\n")
        f.write("        constexpr int kShift = 8;  // log2(256)\n")
        f.write("        // Optimized index calculation: multiply by pre-computed inverse instead of dividing\n")
        f.write("        int index = (rel_x * kInvScale_4) >> kFractionBits;  // rel_x * 256 / (0.009 * kOne)\n")
        f.write("        \n")
        f.write("        int idx = base_idx + index;\n")
        f.write("        int64_t x1 = kThreshold_0_99 + ((kScale * index) >> kShift);  // 0.99 + (0.009 * index / 256)\n")
        f.write("        int64_t x2 = kThreshold_0_99 + ((kScale * (index + 1)) >> kShift);\n\n")
        
        f.write("        int64_t alpha = ((scaled_x - x1) << kFractionBits) / (x2 - x1);\n")
        f.write("        result = ((AcosLut[idx] * (kOne - alpha)) + (AcosLut[idx + 1] * alpha)) >> kFractionBits;\n")
        f.write("    }\n")
        
        f.write("    // Region 5: [0.999, 1.0), use 256-point linear interpolation\n")
        f.write("    else {\n")
        f.write("        constexpr int base_idx = kRegion1Size + kRegion2Size + kRegion3Size + kRegion4Size;\n")
        f.write("        int64_t rel_x = scaled_x - kThreshold_0_999;  // x - 0.999\n")
        f.write("        constexpr int64_t kScale = kOne / 1000LL;           // 0.001 * kOne\n\n")
        f.write("        constexpr int kShift = 8;  // log2(256)\n")
        f.write("        // Optimized index calculation: multiply by pre-computed inverse instead of dividing\n")
        f.write("        int index = (rel_x * kInvScale_5) >> kFractionBits;  // rel_x * 256 / (0.001 * kOne)\n")
        f.write("        \n")
        f.write("        int idx = base_idx + index;\n")
        f.write("        int64_t x1 = kThreshold_0_999 + ((kScale * index) >> kShift);  // 0.999 + (0.001 * index / 256)\n")
        f.write("        int64_t x2 = kThreshold_0_999 + ((kScale * (index + 1)) >> kShift);\n\n")
        
        f.write("        int64_t alpha = ((scaled_x - x1) << kFractionBits) / (x2 - x1);\n")
        f.write("        result = ((AcosLut[idx] * (kOne - alpha)) + (AcosLut[idx + 1] * alpha)) >> kFractionBits;\n")
        f.write("    }\n\n")
        
        f.write("    // Adjust for negative input\n")
        f.write("    if (is_negative) {\n")
        f.write("        result = kPi - result;\n")
        f.write("    }\n\n")
        
        f.write("    // Adjust output precision\n")
        f.write("    if (input_fraction_bits > kFractionBits) {\n")
        f.write("        result = result << (input_fraction_bits - kFractionBits);\n")
        f.write("    } else if (input_fraction_bits < kFractionBits) {\n")
        f.write("        result = result >> (kFractionBits - input_fraction_bits);\n")
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