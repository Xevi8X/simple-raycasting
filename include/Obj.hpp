#pragma once

#include "Structs.hpp"

#include <Eigen/Dense>
#include <optional>

class Sphere
{
public:
    Sphere(const Eigen::Vector4d& center, double radius, const Color& color)
        : center_{center}, radius_{radius}, color_{color}
    {}
    inline std::pair< double, std::optional< Eigen::Vector4d > > intersection(Ray ray);
    inline Eigen::Vector4d                                       normalVector(Eigen::Vector4d point);
    inline Color                                                 getColor() { return color_; };

private:
    Eigen::Vector4d center_;
    double          radius_;
    Color           color_;
};

std::pair< double, std::optional< Eigen::Vector4d > > Sphere::intersection(Ray ray)
{
    auto            p    = ray.point;
    auto            v    = ray.dir;
    Eigen::Vector4d diff = p - center_;
    // std::cout << diff<< std::endl;
    double a     = v.squaredNorm();
    double b     = 2 * diff.dot(v);
    double c     = diff.squaredNorm() - radius_ * radius_;
    double delta = b * b - 4 * a * c;
    if (delta < 0.0)
        return std::make_pair(0.0, std::nullopt);

    double delta_sqrt = std::sqrt(delta);
    double s1         = (-b - delta_sqrt) / (2 * a);
    double s2         = (-b + delta_sqrt) / (2 * a);

    double s = std::min(s1, s2);
    if (s < 0.0)
        return std::make_pair(0.0, std::nullopt);

    return std::make_pair(s, p + s * v);
}

Eigen::Vector4d Sphere::normalVector(Eigen::Vector4d point)
{
    return (point - center_).normalized();
}