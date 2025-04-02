#include <iostream>
#include "Fixed64.h"
#include "Fixed64Math.h"

// Use math::fp namespace explicitly for specific symbols
using math::fp::Fixed64Math;
using math::fp::Fixed64_16;
using math::fp::Fixed64_32;

int main() {
    // Define some angles (using radians)
    Fixed64_32 angleInRadians = Fixed64_32::Deg2Rad() * Fixed64_32(45); // Convert 45 degrees to radians

    // Calculate and output the values of sin, cos, tan
    Fixed64_32 sinValue = Fixed64Math::Sin(angleInRadians);
    Fixed64_32 cosValue = Fixed64Math::Cos(angleInRadians);
    Fixed64_32 tanValue = Fixed64Math::Tan(angleInRadians);

    std::cout << "Sin(45 degrees): " << sinValue.ToString() << std::endl;
    std::cout << "Cos(45 degrees): " << cosValue.ToString() << std::endl;
    std::cout << "Tan(45 degrees): " << tanValue.ToString() << std::endl;

    // Use atan2 to calculate an angle, and convert the result back to degrees
    Fixed64_32 y = Fixed64_32::One();
    Fixed64_32 x = Fixed64_32::One();
    Fixed64_32 atan2Value = Fixed64Math::Atan2(y, x);
    Fixed64_32 angleInDegrees = atan2Value * Fixed64_32::Rad2Deg();

    std::cout << "Atan2(1,1) in degrees: " << angleInDegrees.ToString() << std::endl;

    // Use interpolation for angle interpolation
    Fixed64_32 fromAngle = Fixed64_32::Zero(); // 0 degrees
    Fixed64_32 toAngle = Fixed64_32::Deg2Rad() * Fixed64_32(90); // 90 degrees
    Fixed64_32 t = Fixed64_32(0.5); // Interpolation parameter, 0.5 for the midpoint

    // Linear interpolation between angles
    Fixed64_32 lerpAngle = Fixed64Math::Atan2(
        Fixed64Math::Sin(fromAngle) * (Fixed64_32::One() - t) + Fixed64Math::Sin(toAngle) * t,
        Fixed64Math::Cos(fromAngle) * (Fixed64_32::One() - t) + Fixed64Math::Cos(toAngle) * t
    ) * Fixed64_32::Rad2Deg();
    
    std::cout << "Interpolated angle from 0 to 90 degrees at t=0.5: " << lerpAngle.ToString() << std::endl;

    return 0;
}