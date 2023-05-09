#include "render.hpp"
#include <iostream>
#include <numbers>
#include <Eigen/Dense>
#include <limits>
#include <vector>
#include "camera.hpp"
#include "obj.hpp"
#include "bmp.hpp"
#include "light.hpp"
#include <tuple>
#include <tbb/parallel_for.h>
#include <tbb/global_control.h>

constexpr double ka = 0.1;
constexpr double kd = 0.6;
constexpr double ks = 0.3;
constexpr double m = 8.0;

Render::Render(Camera& cam, std::vector<Sphere> &objects, std::vector<Light> &lights) : camera{cam}, objs{objects}, lights{lights}
{
}

void Render::renderImage(RenderMode mode, int width, int height, std::string path)
{
    switch(mode)
    {
        case RenderMode::CPU:
            renderImageCPU(width,height,path);
        break;

        case RenderMode::TBB:
            renderImageTBB(width,height,path);
        break;

        default:
            std::cerr << "Not implemented yet" << std::endl;
        break;
    }
}

double myCos(Eigen::Vector4d a, Eigen::Vector4d b, bool cut = true)
{
    double cos = a.dot(b) / a.norm() / b.norm();
    if (cut && cos < 0.0)
        cos = 0.0;
    return cos;
}

Color calcColor(Eigen::Vector4d sectionPoint, Eigen::Vector4d cameraPos, Sphere *nearestSphere, std::vector<Light> &lights)
{
    Color c = ka * nearestSphere->getColor();
    Eigen::Vector4d N = nearestSphere->normalVector(sectionPoint);
    for (auto light : lights)
    {
        Eigen::Vector4d L = (light.pos - sectionPoint).normalized();
        Eigen::Vector4d R = 2 * myCos(N, L, false) * N - L;
        double first = kd * myCos(N, L, true);
        Eigen::Vector4d obs = (cameraPos - sectionPoint).normalized();
        double second = ks * std::pow(myCos(obs, R, true), m);
        c = c + (first + second) * light.color * nearestSphere->getColor();
    }
    return c;
}

void Render::renderImageCPU(int width, int height, std::string path)
{
    Image img(width, height);

    Eigen::Vector4d screenUp, screenRight;

    Eigen::Vector4d centralRay = camera.screenCenter - camera.pos;

    screenRight = centralRay.cross3(camera.up).normalized();
    screenUp = screenRight.cross3(centralRay).normalized();
    // std::cout << screenRight << "\n" << screenUp << "\n";

    double fov = camera.fov * (std::numbers::pi / 180);
    double step = std::tan(fov / 2) * centralRay.norm() / (width / 2);

    for (int i = 0; i < width; i++)
        for (int j = 0; j < height; j++)
        {
            int x = i - width / 2;
            int y = j - height / 2;

            Eigen::Vector4d pointOnScreen = camera.screenCenter + x * step * screenRight + y * step * screenUp;
            Ray ray(camera.pos, (pointOnScreen - camera.pos).normalized());

            double z_buffor = std::numeric_limits<double>::max();
            Sphere *nearestSphere = nullptr;
            Eigen::Vector4d sectionPoint;
            for (size_t i = 0; i < objs.size(); i++)
            {
                auto res = objs[i].intersection(ray);
                if (res.second.has_value())
                {
                    if (res.first < z_buffor)
                    {
                        z_buffor = res.first;
                        nearestSphere = &(objs[i]);
                        sectionPoint = res.second.value();
                    }
                }
            }

            if (nearestSphere != nullptr)
            {
                Color c = calcColor(sectionPoint, camera.pos, nearestSphere, lights);
                img.setPixel(i, j, Pixel(c));
            }
        }

    img.saveToBmp(path);
}

void Render::renderImageTBB(int width, int height, std::string path)
{
    Image img(width, height);

    Eigen::Vector4d screenUp, screenRight;

    Eigen::Vector4d centralRay = camera.screenCenter - camera.pos;

    screenRight = centralRay.cross3(camera.up).normalized();
    screenUp = screenRight.cross3(centralRay).normalized();
    // std::cout << screenRight << "\n" << screenUp << "\n";

    double fov = camera.fov * (std::numbers::pi / 180);
    double step = std::tan(fov / 2) * centralRay.norm() / (width / 2);

    tbb::parallel_for(tbb::blocked_range<int>(0, width), [&](tbb::blocked_range<int> r)
    {
        for (int i = r.begin(); i < r.end(); ++i)
            tbb::parallel_for(tbb::blocked_range<int>(0, height), [&](tbb::blocked_range<int> r2)
            {
                for (int j = r2.begin(); j < r2.end(); ++j)
                {
                    int x = i - width / 2;
                    int y = j - height / 2;

                    Eigen::Vector4d pointOnScreen = camera.screenCenter + x * step * screenRight + y * step * screenUp;
                    Ray ray(camera.pos, (pointOnScreen - camera.pos).normalized());

                    double z_buffor = std::numeric_limits<double>::max();
                    Sphere *nearestSphere = nullptr;
                    Eigen::Vector4d sectionPoint;
                    for (size_t i = 0; i < objs.size(); i++)
                    {
                        auto res = objs[i].intersection(ray);
                        if (res.second.has_value())
                        {
                            if (res.first < z_buffor)
                            {
                                z_buffor = res.first;
                                nearestSphere = &(objs[i]);
                                sectionPoint = res.second.value();
                            }
                        }
                    }

                    if (nearestSphere != nullptr)
                    {
                        Color c = calcColor(sectionPoint, camera.pos, nearestSphere, lights);
                        img.setPixel(i, j, Pixel(c));
                    }
                }
            }
        );
    });

        img.saveToBmp(path);
}
