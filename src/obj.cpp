#include <optional>
#include <Eigen/Dense>
#include <iostream>
#include "obj.hpp"
#include "ray.hpp"
#include "color.hpp"



Sphere::Sphere(Eigen::Vector4d center, double radius, Color color):_center{center}, _radius{radius}, _color{color}
{
}

Sphere::~Sphere()
{
}

std::pair<double, std::optional<Eigen::Vector4d>> Sphere::intersection(Ray ray)
{
    auto p = ray.point;
    auto v = ray.dir;
    Eigen::Vector4d diff = p - _center;
    //std::cout << diff<< std::endl;
    double a = v.squaredNorm();
    double b = 2*diff.dot(v);
    double c = diff.squaredNorm() -_radius*_radius;
    double delta = b*b-4*a*c;
    if(delta < 0.0) return std::make_pair(0.0, std::nullopt);

    double delta_sqrt = std::sqrt(delta);
    double s1 = (-b-delta_sqrt)/(2*a);
    double s2 = (-b+delta_sqrt)/(2*a);

    double s = std::min(s1,s2);
    if(s < 0.0) return std::make_pair(0.0, std::nullopt);

    return std::make_pair(s,p + s*v);
}

Eigen::Vector4d Sphere::normalVector(Eigen::Vector4d point)
{
    return (point-_center).normalized();
}