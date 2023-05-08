#pragma once
#include <cstdint>
#include "color.hpp"

struct Pixel
{
    inline Pixel(){};
    inline Pixel(uint8_t _r,uint8_t _g,uint8_t _b): r{_r}, g{_g},b{_b} {}
    inline Pixel(const Color& color): Pixel(color.r,color.g,color.b) {}
    ~Pixel(){}
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    //uint8_t a; //alignment?
};
