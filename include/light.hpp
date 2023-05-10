#pragma once
#include <Eigen/Dense>
#include "color.hpp"

struct Light
{
    inline Light(Eigen::Vector4d _pos, Color c): pos{_pos}, color{c} {}
    Eigen::Vector4d pos;
    Color color;
};
