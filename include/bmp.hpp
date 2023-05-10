#pragma once
#include <string>
#include <tuple>
#include "pixel.hpp"


class Image
{
    public:
        Image(int width, int height);
        ~Image();

        std::tuple<int,int> shape();
        void setPixel(int x, int y, Pixel p);
        Pixel getPixel(int x, int y);
        void saveToBmp(std::string path);

    private:
        int w,h;
        Pixel* canva;
};