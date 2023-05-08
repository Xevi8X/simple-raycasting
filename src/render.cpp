#include "render.hpp"
#include <iostream>
#include <numbers>
#include <Eigen/Dense>
#include <limits>
#include <vector>
#include "camera.hpp"
#include "obj.hpp"
#include "bmp.hpp"
#include<tuple>


Render::Render(Camera cam, std::vector<Sphere>& objects):
camera{cam}, objs{objects}
{
}

void Render::renderImage(int width, int height, std::string path)
{
    Image img(width,height);

    Eigen::Vector4d screenUp,screenRight;

    Eigen::Vector4d centralRay = camera.screenCenter-camera.pos;

    screenRight = centralRay.cross3(camera.up).normalized();
    screenUp = screenRight.cross3(centralRay).normalized();
    //std::cout << screenRight << "\n" << screenUp << "\n";

    double fov = camera.fov * (std::numbers::pi / 180);
    double step = std::tan(fov/2)*centralRay.norm()/(width/2);

    for(int i = 0; i < width; i++)
    for(int j = 0; j < height; j++)
    {
        int x = i - width/2;
        int y = j - height/2;
        
        Eigen::Vector4d pointOnScreen = camera.screenCenter + x*step*screenRight + y*step*screenUp;
        Ray ray(camera.pos,(pointOnScreen-camera.pos).normalized());

        double z_buffor = std::numeric_limits<double>::max();
        Sphere* nearestSphere = nullptr;
        Eigen::Vector4d sectionPoint;
        for(size_t i = 0; i < objs.size(); i++)
        {
            auto res = objs[i].intersection(ray);
            if(res.second.has_value())
            {
                if(res.first < z_buffor)
                {
                    z_buffor = res.first;
                    nearestSphere = &(objs[i]);
                    sectionPoint = res.second.value();    
                }
            }
            else{
            }
        }

        if(nearestSphere != nullptr)
        {
            img.setPixel(i,j,Pixel(nearestSphere->getColor()));
        }

    }

    img.saveToBmp(path);
}
