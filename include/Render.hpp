#pragma once

#include "Bmp.hpp"
#include "Obj.hpp"
#include "Structs.hpp"

#include <Eigen/Dense>
#include <tbb/tbb.h>

#include <iostream>
#include <limits>
#include <numbers>
#include <string>
#include <vector>
#include <tuple>

enum struct RenderMode
{
    CPU,
    TBB,
    SIMD
};

class Render
{
    static constexpr double ka = 0.1;
    static constexpr double kd = 0.6;
    static constexpr double ks = 0.3;
    static constexpr double m  = 30.0;

public:
    Render(Camera& cam, std::vector< Light >& lights, Obj3D** objects, size_t noOfObjects)
        : camera{cam}, objs{objects}, lights{lights}, noOfObjs{noOfObjects}
    {}
    inline void renderImage(RenderMode mode, int width, int height, std::string path);

private:
    inline static double myCos(Eigen::Vector4d a, Eigen::Vector4d b, bool cut = true);
    // inline static Color  calcColor(Eigen::Vector4d       sectionPoint,
    //                                Eigen::Vector4d       cameraPos,
    //                                Sphere*               nearestSphere,
    //                                std::vector< Light >& lights);
    inline static Color
    calcColor(Eigen::Vector4d sectionPoint, Eigen::Vector4d cameraPos, Obj3D* Obj, std::vector< Light >& lights);
    inline void renderImageCPU(int width, int height, std::string path);
    inline void renderImageTBB(int width, int height, std::string path);

    template<int noOfSpheres, int batch_size>
    inline void renderImageSIMDSpheres(int width, int height, std::string path);

    Camera&               camera;
    Obj3D**               objs;
    size_t                noOfObjs;
    std::vector< Light >& lights;

    const Color skyColor = {135, 206, 235};
};

void Render::renderImage(RenderMode mode, int width, int height, std::string path)
{
    switch (mode)
    {
    case RenderMode::CPU:
        renderImageCPU(width, height, path);
        break;

    case RenderMode::TBB:
        renderImageTBB(width, height, path);
        break;

    case RenderMode::SIMD:
        renderImageSIMDSpheres<256,4>(width, height, path);
        break;

    default:
        std::cerr << "Not implemented yet" << std::endl;
        break;
    }
}

double Render::myCos(Eigen::Vector4d a, Eigen::Vector4d b, bool cut)
{
    double cos = a.dot(b) / a.norm() / b.norm();
    if (cut && cos < 0.0)
        cos = 0.0;
    return cos;
}

// Color Render::calcColor(Eigen::Vector4d       sectionPoint,
//                         Eigen::Vector4d       cameraPos,
//                         Sphere*               nearestSphere,
//                         std::vector< Light >& lights)
// {
//     Color           c = ka * nearestSphere->getColor(sectionPoint);
//     Eigen::Vector4d N = nearestSphere->normalVector(sectionPoint);
//     for (auto light : lights)
//     {
//         Eigen::Vector4d L      = (light.pos - sectionPoint).normalized();
//         Eigen::Vector4d R      = 2 * myCos(N, L, false) * N - L;
//         double          first  = kd * myCos(N, L, true);
//         Eigen::Vector4d obs    = (cameraPos - sectionPoint).normalized();
//         double          second = ks * std::pow(myCos(obs, R, true), m);
//         c                      = c + (first + second) * light.color * nearestSphere->getColor(sectionPoint);
//     }
//     return c;
// }

Color Render::calcColor(Eigen::Vector4d       sectionPoint,
                        Eigen::Vector4d       cameraPos,
                        Obj3D*                obj,
                        std::vector< Light >& lights)
{
    // std::cout << sectionPoint.transpose() << std::endl;
    Color           c = ka * obj->getColor(sectionPoint);
    Eigen::Vector4d N = obj->normalVector(sectionPoint);
    for (auto light : lights)
    {
        Eigen::Vector4d L      = (light.pos - sectionPoint).normalized();
        Eigen::Vector4d R      = 2 * myCos(N, L, false) * N - L;
        double          first  = kd * myCos(N, L, true);
        Eigen::Vector4d obs    = (cameraPos - sectionPoint).normalized();
        double          second = ks * std::pow(myCos(obs, R, true), m);
        c                      = c + (first + second) * light.color * obj->getColor(sectionPoint);
    }
    return c;
}

void Render::renderImageCPU(int width, int height, std::string path)
{
    Image           img(width, height);
    Eigen::Vector4d screenUp, screenRight;
    Eigen::Vector4d centralRay = camera.screenCenter - camera.pos;
    screenRight                = centralRay.cross3(camera.up).normalized();
    screenUp                   = screenRight.cross3(centralRay).normalized();
    double fov                 = camera.fov * (std::numbers::pi / 180);
    double step                = std::tan(fov / 2) * centralRay.norm() / (width / 2);

    for (int i = 0; i < width; i++)
        for (int j = 0; j < height; j++)
        {
            int             x             = i - width / 2;
            int             y             = j - height / 2;
            Eigen::Vector4d pointOnScreen = camera.screenCenter + x * step * screenRight + y * step * screenUp;
            Ray             ray(camera.pos, (pointOnScreen - camera.pos).normalized());

            double          z_buffor        = std::numeric_limits< double >::max();
            int             nearestObjIndex = -1;
            Eigen::Vector4d sectionPoint;
            for (size_t i = 0; i < noOfObjs; i++)
            {
                auto res = (objs[i])->intersection(ray);
                if (res.second.has_value())
                {
                    if (res.first < z_buffor)
                    {
                        z_buffor        = res.first;
                        nearestObjIndex = i;
                        sectionPoint    = res.second.value();
                    }
                }
            }
            if (nearestObjIndex >= 0)
            {
                Color c = calcColor(sectionPoint, camera.pos, objs[nearestObjIndex], lights);
                img.setPixel(i, j, Pixel(c));
            }
            else
            {
                img.setPixel(i, j, skyColor);
            }
        }

    img.saveToBmp(path);
}

void Render::renderImageTBB(int width, int height, std::string path)
{
    Image           img(width, height);
    Eigen::Vector4d screenUp, screenRight;
    Eigen::Vector4d centralRay = camera.screenCenter - camera.pos;
    screenRight                = centralRay.cross3(camera.up).normalized();
    screenUp                   = screenRight.cross3(centralRay).normalized();
    double fov                 = camera.fov * (std::numbers::pi / 180);
    double step                = std::tan(fov / 2) * centralRay.norm() / (width / 2);

    tbb::parallel_for(tbb::blocked_range< int >(0, width), [&](tbb::blocked_range< int > r) {
        for (int i = r.begin(); i < r.end(); ++i)
            tbb::parallel_for(tbb::blocked_range< int >(0, height), [&](tbb::blocked_range< int > r2) {
                for (int j = r2.begin(); j < r2.end(); ++j)
                {
                    int             x             = i - width / 2;
                    int             y             = j - height / 2;
                    Eigen::Vector4d pointOnScreen = camera.screenCenter + x * step * screenRight + y * step * screenUp;
                    Ray             ray(camera.pos, (pointOnScreen - camera.pos).normalized());

                    double          z_buffor        = std::numeric_limits< double >::max();
                    int             nearestObjIndex = -1;
                    Eigen::Vector4d sectionPoint;
                    for (size_t i = 0; i < noOfObjs; i++)
                    {
                        auto res = (objs[i])->intersection(ray);
                        if (res.second.has_value())
                        {
                            if (res.first < z_buffor)
                            {
                                z_buffor        = res.first;
                                nearestObjIndex = i;
                                sectionPoint    = res.second.value();
                            }
                        }
                    }
                    if (nearestObjIndex >= 0)
                    {
                        Color c = calcColor(sectionPoint, camera.pos, objs[nearestObjIndex], lights);
                        img.setPixel(i, j, Pixel(c));
                    }
                    else
                    {
                        img.setPixel(i, j, skyColor);
                    }
                }
            });
    });

    img.saveToBmp(path);
}

template<int noOfSpheres>
std::tuple<Eigen::Matrix<double,4,noOfSpheres>,Eigen::Vector<double,noOfSpheres>> prepareSpheresMatrix(Obj3D **objs)
{
    Eigen::Matrix<double,4,noOfSpheres> centers;
    Eigen::Vector<double,noOfSpheres> radius2;
    for (size_t i = 0; i < noOfSpheres; i++)
    {
        radius2(i) = std::pow((dynamic_cast<Sphere*>(objs[i]))->getRadius(),2);
        centers.col(i) = dynamic_cast<Sphere*>(objs[i])->getCenter();
    }
    return {centers,radius2};
}

template<int batch_size>
inline std::tuple< double, std::optional< Eigen::Vector4d >, int> batchIntersection(Ray ray,Eigen::Matrix<double,4,batch_size>& centers, Eigen::Vector<double,batch_size>& radius2 )
{

}

template<int noOfSpheres, int batch_size>
void Render::renderImageSIMDSpheres(int width, int height, std::string path)
{
    Image           img(width, height);
    Eigen::Vector4d screenUp, screenRight;
    Eigen::Vector4d centralRay = camera.screenCenter - camera.pos;
    screenRight                = centralRay.cross3(camera.up).normalized();
    screenUp                   = screenRight.cross3(centralRay).normalized();
    double fov                 = camera.fov * (std::numbers::pi / 180);
    double step                = std::tan(fov / 2) * centralRay.norm() / (width / 2);

    // Eigen::Matrix<double,4,noOfSpheres> centers;
    // Eigen::Vector<double,noOfSpheres> radius2;
    auto && [centers, radius2] = prepareSpheresMatrix<noOfSpheres>(objs);

    tbb::parallel_for(tbb::blocked_range< int >(0, width), [&](tbb::blocked_range< int > r) {
        for (int i = r.begin(); i < r.end(); ++i)
            tbb::parallel_for(tbb::blocked_range< int >(0, height), [&](tbb::blocked_range< int > r2) {
                for (int j = r2.begin(); j < r2.end(); ++j)
                {
                    int             x             = i - width / 2;
                    int             y             = j - height / 2;
                    Eigen::Vector4d pointOnScreen = camera.screenCenter + x * step * screenRight + y * step * screenUp;
                    Ray             ray(camera.pos, (pointOnScreen - camera.pos).normalized());
                    double          z_buffor        = std::numeric_limits< double >::max();
                    int             nearestObjIndex = -1;
                    Eigen::Vector4d sectionPoint;
                    // BATCH SPLITING BEGIN
                    for (int k = 0; k < noOfSpheres/batch_size; k++)
                    {
                        Eigen::Matrix<double,4,batch_size> local_centers = centers.block(0,k*batch_size,4,batch_size);
                        Eigen::Vector<double,batch_size> local_radius2 = radius2.segment(k*batch_size,batch_size);
                        auto res = batchIntersection(ray,local_centers,local_radius2);
                        if (std::get<1>(res).has_value())
                        {
                            if (std::get<0>(res) < z_buffor)
                            {
                                z_buffor        = std::get<0>(res);
                                nearestObjIndex = batch_size * k + std::get<2>(res);
                                sectionPoint    = std::get<1>(res).value();
                            }
                        }

                    }
                    
                    // BATCH SPLITING END
                    if (nearestObjIndex >= 0)
                    {
                        Color c = calcColor(sectionPoint, camera.pos, objs[nearestObjIndex], lights);
                        img.setPixel(i, j, Pixel(c));
                    }
                    else
                    {
                        img.setPixel(i, j, skyColor);
                    }
                }
            });
    });

    img.saveToBmp(path);
}
