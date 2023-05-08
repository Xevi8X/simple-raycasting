#pragma once
#include "ray.hpp"
#include <optional>
#include <Eigen/Dense>
#include "color.hpp"

class Obj
{
    public:
        Obj(){};
        ~Obj(){};
        virtual std::pair<double, std::optional<Eigen::Vector4d>> intersection(Ray ray);
        virtual Eigen::Vector4d normalVector(Eigen::Vector4d point);
};

class Sphere{
    
    public:
        Sphere(Eigen::Vector4d center, double radius, Color color);
        ~Sphere();
        std::pair<double, std::optional<Eigen::Vector4d>> intersection(Ray ray);
        Eigen::Vector4d normalVector(Eigen::Vector4d point);
        inline Color getColor() {return _color;};

    private:
        Eigen::Vector4d _center;
        double _radius;
        Color _color;
};