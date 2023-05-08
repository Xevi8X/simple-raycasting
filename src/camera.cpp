#include <Eigen/Dense>
#include "camera.hpp"

Camera::Camera(Eigen::Vector4d _pos, Eigen::Vector4d _screenCenter, Eigen::Vector4d _up, double _fov):
pos{_pos}, screenCenter{_screenCenter}, up{_up}, fov{_fov}
{
}