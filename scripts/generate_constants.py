import mpmath as mp
import sys

# Set precision to much higher than default
mp.mp.dps = 100  # 100 decimal digits of precision

def print_constant_info(name, value):
    """Print detailed information about a constant scaled by 2^63"""
    # 2^63 for scaling
    two_63 = mp.mpf(2) ** 63
    
    # Scale by 2^63
    scaled = value * two_63
    
    # Convert to integer while preserving all significant digits
    int_scaled = int(scaled)
    
    # Get high and low 64-bit parts
    high_part = int_scaled >> 64
    low_part = int_scaled & ((1 << 64) - 1)
    
    # Format high part properly (as signed)
    if high_part < 0:
        high_part_str = f"-0x{-high_part:x}"
    else:
        high_part_str = f"0x{high_part:x}"
    
    # Print in hex with proper formatting
    print(f"// {name}: {value}")
    print(f"// {name} × 2^63 = {scaled}")
    print(f"// Full 128-bit hex: 0x{int_scaled & ((1 << 128) - 1):032x}")
    print(f"constexpr int64_t {name}_hi = {high_part_str};")
    print(f"constexpr uint64_t {name}_lo = 0x{low_part:016x}ULL;")
    
    # Add the new format for easy copy-pasting
    print(f"\n// For easy copy-paste:")
    print(f"({high_part_str}, 0x{low_part:016x}ULL, detail::nothing{{}})")
    print()

# Check if input is provided
if len(sys.argv) > 1:
    # Handle input value mode
    try:
        input_value = mp.mpf(sys.argv[1])
        print("// Custom constant with 100 digits of precision\n")
        print_constant_info("Custom", input_value)
    except Exception as e:
        print(f"Error processing input: {e}")
        print("Usage: python generate_constants.py [number]")
        sys.exit(1)
else:
    # No input - generate all constants
    constants = {
        "Pi": mp.pi,
        "TwoPi": 2 * mp.pi,
        "HalfPi": mp.pi / 2,
        "QuarterPi": mp.pi / 4,
        "InvPi": 1 / mp.pi,
        "E": mp.e,
        "Ln2": mp.log(2),
        "Log2E": mp.log(mp.e, 2),  # log base 2 of e
        "Log10Of2": mp.log10(2),
        "Deg2Rad": mp.pi / 180,
        "Rad2Deg": 180 / mp.pi
    }

    print("// Constants for Fixed64 with 100 digits of precision\n")
    for name, value in constants.items():
        print_constant_info(name, value)