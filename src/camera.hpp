#pragma once
#include <Eigen/Dense>

struct Camera
{
    Camera(Eigen::Vector4d _pos, Eigen::Vector4d _screenCenter, Eigen::Vector4d _up, double _fov);
    Eigen::Vector4d pos;
    Eigen::Vector4d screenCenter;
    Eigen::Vector4d up;
    double fov;
};