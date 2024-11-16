#include "FixedMath.h"
#include <iostream>

int main() {
    // 定义一些角度（使用弧度）
    Fixed64 angleInRadians = FixedMath::Deg2Rad * Fixed64::parseLong(45); // 将45度转换为弧度

    // 计算并输出sin, cos, tan的值
    Fixed64 sinValue = FixedMath::Sin(angleInRadians);
    Fixed64 cosValue = FixedMath::Cos(angleInRadians);
    Fixed64 tanValue = FixedMath::Tan(angleInRadians);

    std::cout << "Sin(45 degrees): " << sinValue << std::endl;
    std::cout << "Cos(45 degrees): " << cosValue << std::endl;
    std::cout << "Tan(45 degrees): " << tanValue << std::endl;

    // 使用atan2来计算一个角度，并将结果转换回度
    Fixed64 y = Fixed64Const::One;
    Fixed64 x = Fixed64Const::One;
    Fixed64 atan2Value = FixedMath::Atan2(y, x);
    Fixed64 angleInDegrees = atan2Value * FixedMath::Rad2Deg;

    std::cout << "Atan2(1,1) in degrees: " << angleInDegrees << std::endl;

    // 使用LerpAngle来进行角度插值
    Fixed64 fromAngle = Fixed64Const::Zero; // 0度
    Fixed64 toAngle = FixedMath::Deg2Rad * Fixed64::parseLong(90); // 90度
    Fixed64 t = Fixed64::parseLong(0.5); // 插值参数，0.5表示中间值

    Fixed64 lerpAngle = FixedMath::LerpAngle(fromAngle, toAngle, t) * FixedMath::Rad2Deg;
    std::cout << "LerpAngle from 0 to 90 degrees at t=0.5: " << lerpAngle << std::endl;

    return 0;
}