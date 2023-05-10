#pragma once
#include <cstdint>

class Color
{
    public:
        Color(){};
        Color(uint8_t _r,uint8_t _g,uint8_t _b): r{_r}, g{_g},b{_b} {}
        ~Color(){}
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;

        inline Color operator+(const Color& other) const {
            return Color(std::min(255,r+other.r),std::min(255,g+other.g),std::min(255,b+other.b));
        }

        friend Color operator*(double a, Color c)
        {
            return Color(
                std::min(255,(int)(a*c.r)),
                std::min(255,(int)(a*c.g)),
                std::min(255,(int)(a*c.b))
                );
        }

        friend Color operator*(Color b, Color c)
        {
            return Color(
                std::min(255,(int)(b.r/255.0*c.r)),
                std::min(255,(int)(b.g/255.0*c.g)),
                std::min(255,(int)(b.b/255.0*c.b))
            );
        }

};

