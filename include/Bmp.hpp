#pragma once

#include "Structs.hpp"

#include <cstring>
#include <memory>
#include <string>

class Image
{
public:
    inline Image() = default;
    inline Image(std::size_t width, std::size_t height)
        : w_{width}, h_{height}, canvas_{std::make_unique_for_overwrite< Pixel[] >(width * height)}
    {}

    inline void  setPixel(std::size_t x, std::size_t y, Pixel p);
    inline Pixel getPixel(std::size_t x, std::size_t y);
    inline void  saveToBmp(std::string path);

private:
    std::size_t                w_, h_;
    std::unique_ptr< Pixel[] > canvas_;
};

void Image::setPixel(std::size_t x, std::size_t y, Pixel p)
{
    canvas_[x + y * w_] = p;
}

Pixel Image::getPixel(std::size_t x, std::size_t y)
{
    return canvas_[x + y * w_];
}

void Image::saveToBmp(std::string path)
{
    // Coordinate system in left down corner, x right, y up
    FILE*       f;
    std::size_t filesize = 54 + 3 * w_ * h_; // w_ is your image width, h_ is image height, both std::size_t

    const auto img = std::make_unique_for_overwrite< unsigned char[] >(3 * w_ * h_);

    for (std::size_t i = 0; i < w_; i++)
    {
        for (std::size_t j = 0; j < h_; j++)
        {
            const auto x              = i;
            const auto y              = (h_ - 1) - j;
            Pixel      p              = getPixel(i, j);
            img[(x + y * w_) * 3 + 2] = static_cast< unsigned char >(p.r);
            img[(x + y * w_) * 3 + 1] = static_cast< unsigned char >(p.g);
            img[(x + y * w_) * 3 + 0] = static_cast< unsigned char >(p.b);
        }
    }

    unsigned char bmpfileheader[14] = {'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0};
    unsigned char bmpinfoheader[40] = {40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0};
    unsigned char bmppad[3]         = {0, 0, 0};

    const auto fsz32 = static_cast< std::uint32_t >(filesize);
    std::memcpy(bmpfileheader + 2, &fsz32, sizeof fsz32);
    const auto info_hdr_arr = std::array{static_cast< std::uint32_t >(w_), static_cast< std::uint32_t >(h_)};
    std::memcpy(bmpinfoheader + 4, info_hdr_arr.data(), sizeof info_hdr_arr);

    f = fopen(path.c_str(), "wb");
    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);
    for (std::size_t i = 0; i < h_; i++)
    {
        fwrite(img.get() + (w_ * (h_ - i - 1) * 3), 3, w_, f);
        fwrite(bmppad, 1, (4 - (w_ * 3) % 4) % 4, f);
    }

    fclose(f);
}