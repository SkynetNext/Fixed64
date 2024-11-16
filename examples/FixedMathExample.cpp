#include "FixedMath.h"
#include <iostream>

using namespace Skynet;

int main() {
    // Define some angles (using radians)
    Fixed64 angleInRadians = FixedMath::Deg2Rad * Fixed64(45); // Convert 45 degrees to radians

    // Calculate and output the values of sin, cos, tan
    Fixed64 sinValue = FixedMath::Sin(angleInRadians);
    Fixed64 cosValue = FixedMath::Cos(angleInRadians);
    Fixed64 tanValue = FixedMath::Tan(angleInRadians);

    std::cout << "Sin(45 degrees): " << sinValue << std::endl;
    std::cout << "Cos(45 degrees): " << cosValue << std::endl;
    std::cout << "Tan(45 degrees): " << tanValue << std::endl;

    // Use atan2 to calculate an angle, and convert the result back to degrees
    Fixed64 y = Fixed64Const::One;
    Fixed64 x = Fixed64Const::One;
    Fixed64 atan2Value = FixedMath::Atan2(y, x);
    Fixed64 angleInDegrees = atan2Value * FixedMath::Rad2Deg;

    std::cout << "Atan2(1,1) in degrees: " << angleInDegrees << std::endl;

    // Use LerpAngle for angle interpolation
    Fixed64 fromAngle = Fixed64Const::Zero; // 0 degrees
    Fixed64 toAngle = FixedMath::Deg2Rad * Fixed64(90); // 90 degrees
    Fixed64 t = Fixed64(0.5); // Interpolation parameter, 0.5 for the midpoint

    Fixed64 lerpAngle = FixedMath::LerpAngle(fromAngle, toAngle, t) * FixedMath::Rad2Deg;
    std::cout << "LerpAngle from 0 to 90 degrees at t=0.5: " << lerpAngle << std::endl;

    return 0;
}