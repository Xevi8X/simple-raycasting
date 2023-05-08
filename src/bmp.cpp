#include "bmp.hpp"

Image::Image(int width, int height) : w{width}, h{height}
{
    canva = new Pixel[w * h];
}

Image::~Image()
{
    delete[] canva;
}

void Image::setPixel(int x, int y, Pixel p)
{
    canva[x + y * w] = p;
}

Pixel Image::getPixel(int x, int y)
{
    return canva[x + y * w];
}

void Image::saveToBmp(std::string path)
{
    // Coordinate system in left down corner, x right, y up
    FILE *f;
    unsigned char *img = NULL;
    int filesize = 54 + 3 * w * h; // w is your image width, h is image height, both int

    img = new unsigned char[3 * w * h];
    //std::memset(img, 0, 3 * w * h);

    int x,y;

    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            x = i;
            y = (h - 1) - j;
            Pixel p = getPixel(i,j);
            img[(x + y * w) * 3 + 2] = (unsigned char)(p.r);
            img[(x + y * w) * 3 + 1] = (unsigned char)(p.g);
            img[(x + y * w) * 3 + 0] = (unsigned char)(p.b);
        }
    }

    unsigned char bmpfileheader[14] = {'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0};
    unsigned char bmpinfoheader[40] = {40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0};
    unsigned char bmppad[3] = {0, 0, 0};

    bmpfileheader[2] = (unsigned char)(filesize);
    bmpfileheader[3] = (unsigned char)(filesize >> 8);
    bmpfileheader[4] = (unsigned char)(filesize >> 16);
    bmpfileheader[5] = (unsigned char)(filesize >> 24);

    bmpinfoheader[4] = (unsigned char)(w);
    bmpinfoheader[5] = (unsigned char)(w >> 8);
    bmpinfoheader[6] = (unsigned char)(w >> 16);
    bmpinfoheader[7] = (unsigned char)(w >> 24);
    bmpinfoheader[8] = (unsigned char)(h);
    bmpinfoheader[9] = (unsigned char)(h >> 8);
    bmpinfoheader[10] = (unsigned char)(h >> 16);
    bmpinfoheader[11] = (unsigned char)(h >> 24);

    f = fopen(path.c_str(), "wb");
    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);
    for (int i = 0; i < h; i++)
    {
        fwrite(img + (w * (h - i - 1) * 3), 3, w, f);
        fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, f);
    }

    delete[] img;
    fclose(f);
}
