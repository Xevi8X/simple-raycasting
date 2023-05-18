#pragma once

#include "Structs.hpp"

#include <Eigen/Dense>
#include <iostream>
#include <optional>

class Obj3D
{
public:
    virtual std::pair< double, std::optional< Eigen::Vector4d > > intersection(Ray ray)               = 0;
    virtual Eigen::Vector4d                                       normalVector(Eigen::Vector4d point) = 0;
    virtual Color                                                 getColor(Eigen::Vector4d point)     = 0;
};

class Sphere : public Obj3D
{
public:
    Sphere(const Eigen::Vector4d& center, double radius, const Color& color)
        : center_{center}, radius_{radius}, color_{color}
    {}
    inline std::pair< double, std::optional< Eigen::Vector4d > > intersection(Ray ray) override;
    inline Eigen::Vector4d                                       normalVector(Eigen::Vector4d point) override;
    inline Color getColor(Eigen::Vector4d point) override { return color_; };

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
        return std::make_pair(-1.0, std::nullopt);

    double delta_sqrt = std::sqrt(delta);
    double s1         = (-b - delta_sqrt) / (2 * a);
    double s2         = (-b + delta_sqrt) / (2 * a);

    double s = std::min(s1, s2);
    if (s < 0.0)
        return std::make_pair(-1.0, std::nullopt);

    return std::make_pair(s, p + s * v);
}

Eigen::Vector4d Sphere::normalVector(Eigen::Vector4d point)
{
    return (point - center_).normalized();
}

class Plane : public Obj3D
{
public:
    inline std::pair< double, std::optional< Eigen::Vector4d > > intersection(Ray ray) override;
    inline Eigen::Vector4d                                       normalVector(Eigen::Vector4d point) override;
    inline Color                                                 getColor(Eigen::Vector4d point) override;

private:
    Eigen::Vector4d center_;
    double          radius_;
    Color           color[2] = {Color(252, 204, 116), Color(87, 58, 46)};
};

std::pair< double, std::optional< Eigen::Vector4d > > Plane::intersection(Ray ray)
{
    Eigen::Vector4d p = ray.point;
    Eigen::Vector4d v = ray.dir;
    if (p.z() < 0.0 || v.z() >= 0)
        return std::make_pair(0.0, std::nullopt);

    double scale = p.z() / (-v.z());
    return std::make_pair(scale * p.z(), p + scale * v);
}

Eigen::Vector4d Plane::normalVector(Eigen::Vector4d point)
{
    return Eigen::Vector4d(0, 0, 1, 0);
}

Color Plane::getColor(Eigen::Vector4d point)
{
    constexpr int scale = 10;
    int           x     = static_cast< int >(point.x());
    int           y     = static_cast< int >(point.y());
    int           res   = (x >= 0 ? x : x - scale) / scale + (y > 0 ? y : y - scale) / scale;
    return res % 2 == 0 ? color[0] : color[1];
};