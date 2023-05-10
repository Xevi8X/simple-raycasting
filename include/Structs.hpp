#pragma once

#include "Eigen/Dense"

#include <cstdint>

struct Color
{
    std::uint8_t r{}, g{}, b{};
};

struct Pixel
{
    Pixel() = default;
    Pixel(const Color& color) : r{color.r}, g{color.g}, b{color.b} {}

    std::uint8_t r{255}, g{255}, b{255};
};

inline Color operator+(const Color& c1, const Color& c2)
{
    constexpr auto comp_sum = [](std::uint8_t comp1, std::uint8_t comp2) {
        return static_cast< std::uint8_t >(std::clamp(comp1 + comp2, 0, 255));
    };
    return {comp_sum(c1.r, c2.r), comp_sum(c1.g, c2.g), comp_sum(c1.b, c2.b)};
}

inline Color operator*(double a, const Color& c)
{
    const auto mult_by_a = [a](std::uint8_t value) {
        return static_cast< std::uint8_t >(std::clamp(value * a, 0., 255.));
    };
    return {mult_by_a(c.r), mult_by_a(c.g), mult_by_a(c.b)};
}

inline Color operator*(const Color& c1, const Color& c2)
{
    constexpr auto comp_prod = [](std::uint8_t comp1, std::uint8_t comp2) {
        const auto c1_d = static_cast< double >(comp1) / 255.;
        return static_cast< std::uint8_t >(std::clamp(c1_d * comp2, 0., 255.));
    };
    return {comp_prod(c1.r, c2.r), comp_prod(c1.g, c2.g), comp_prod(c1.b, c2.b)};
}

struct Camera
{
    Eigen::Vector4d pos;
    Eigen::Vector4d screenCenter;
    Eigen::Vector4d up;
    double          fov;
};

struct Light
{
    Eigen::Vector4d pos;
    Color           color;
};

struct Ray
{
    Eigen::Vector4d point;
    Eigen::Vector4d dir;
};
