#pragma once
#include <eigen3/Eigen/Dense>


struct Ray
{
    inline Ray(Eigen::Vector4d _point,Eigen::Vector4d _dir): point{_point}, dir{_dir} {}
    Eigen::Vector4d point;
    Eigen::Vector4d dir;
};