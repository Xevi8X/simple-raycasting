#pragma once
#include <cstdint>

struct Color
{
    Color(){};
    Color(uint8_t _r,uint8_t _g,uint8_t _b): r{_r}, g{_g},b{_b} {}
    ~Color(){}
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};