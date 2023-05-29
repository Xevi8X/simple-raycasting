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
#include <chrono>

constexpr int sphereNo = 1024;

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
        : camera{cam}, objs{objects}, lights{lights}, noOfObjs{noOfObjects} {}
    void prepare(int width, int height);
    void renderImage(RenderMode mode, int batch_size = 8);
    void saveTo(std::string path) {img.saveToBmp(path);}

private:
    inline static double myCos(Eigen::Vector4d a, Eigen::Vector4d b, bool cut = true);
    // inline static Color  calcColor(Eigen::Vector4d       sectionPoint,
    //                                Eigen::Vector4d       cameraPos,
    //                                Sphere*               nearestSphere,
    //                                std::vector< Light >& lights);
    inline static Color
    calcColor(Eigen::Vector4d sectionPoint, Eigen::Vector4d cameraPos, Obj3D* Obj, std::vector< Light >& lights);
    void renderImageCPU();
    void renderImageTBB();

    template<int noOfSpheres, int batch_size>
    void renderImageSIMDSpheres();

    Camera&               camera;
    Obj3D**               objs;
    const size_t                noOfObjs;
    std::vector< Light >& lights;
    Image img;
    int width, height;

    const Color skyColor = {135, 206, 235};
    Eigen::Matrix<double,4,sphereNo> centers;
    Eigen::Vector<double,sphereNo> radius2;

};

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

void Render::prepare(int width_, int height_)
{
    width = width_;
    height = height_;
    img = std::move(Image(width, height));
    std::tie(centers, radius2) = prepareSpheresMatrix<sphereNo>(objs);
}

void Render::renderImage(RenderMode mode, int batch_size)
{
    switch (mode)
    {
    case RenderMode::CPU:
        renderImageCPU();
        break;

    case RenderMode::TBB:
        renderImageTBB();
        break;

    case RenderMode::SIMD:
        switch (batch_size)
        {
        case 1:
            renderImageSIMDSpheres<sphereNo,1>();
            break;
        case 2:
            renderImageSIMDSpheres<sphereNo,2>();
            break;
        case 4:
            renderImageSIMDSpheres<sphereNo,4>();
            break;
        case 8:
            renderImageSIMDSpheres<sphereNo,8>();
            break;
        case 16:
            renderImageSIMDSpheres<sphereNo,16>();
            break;
        case 32:
            renderImageSIMDSpheres<sphereNo,32>();
            break;
        case 64:
            renderImageSIMDSpheres<sphereNo,64>();
            break;
        case 128:
            renderImageSIMDSpheres<sphereNo,128>();
            break;
        default:
            std::cerr << "Invalid batch size" << std::endl;
            return;
        }
        break;

    default:
        std::cerr << "Not implemented yet" << std::endl;
        return;
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

void Render::renderImageCPU()
{
    Eigen::Vector4d screenUp, screenRight;
    Eigen::Vector4d centralRay = camera.screenCenter - camera.pos;
    screenRight                = centralRay.cross3(camera.up).normalized();
    screenUp                   = screenRight.cross3(centralRay).normalized();
    double fov                 = camera.fov * (std::numbers::pi / 180);
    double step                = std::tan(fov / 2) * centralRay.norm() / (width / 2);

    //auto t_start = std::chrono::high_resolution_clock::now();
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
    //auto t_end = std::chrono::high_resolution_clock::now();
    //std::cout << "Time CPU: " << std::chrono::duration<double, std::milli>(t_end-t_start).count() << "ms" << std::endl;

    //img.saveToBmp(path);
}

void Render::renderImageTBB()
{
    Eigen::Vector4d screenUp, screenRight;
    Eigen::Vector4d centralRay = camera.screenCenter - camera.pos;
    screenRight                = centralRay.cross3(camera.up).normalized();
    screenUp                   = screenRight.cross3(centralRay).normalized();
    double fov                 = camera.fov * (std::numbers::pi / 180);
    double step                = std::tan(fov / 2) * centralRay.norm() / (width / 2);

    //auto t_start = std::chrono::high_resolution_clock::now();
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
    //auto t_end = std::chrono::high_resolution_clock::now();
    //std::cout << "Time TBB: " << std::chrono::duration<double, std::milli>(t_end-t_start).count() << "ms" << std::endl;

    //img.saveToBmp(path);
}



template<int batch_size>
inline std::tuple< double, std::optional< Eigen::Vector4d >, int> batchIntersection(Ray ray,Eigen::Matrix<double,4,batch_size>& centers, Eigen::Vector<double,batch_size>& radius2 )
{
    static const Eigen::Vector<double,batch_size> zeros = Eigen::Vector<double,batch_size>::Zero();
    static const Eigen::Vector<double,batch_size> inf = Eigen::Vector<double,batch_size>::Constant(std::numeric_limits< double >::max());

    centers.colwise() -= ray.point;     //centers is -diff
    Eigen::Vector<double,batch_size> a;
    double a_scalar = ray.dir.squaredNorm();
    a.setConstant(a_scalar); 
    Eigen::Vector<double,batch_size> c = centers.colwise().squaredNorm().transpose() - radius2;
    Eigen::Vector<double,batch_size> b = -2* (centers.transpose()*ray.dir);
    Eigen::Vector<double,batch_size> delta = b.cwiseProduct(b) - 4 * a.cwiseProduct(c);

    // Eigen::Vector<double,batch_size> mask;
    // for (size_t i = 0; i < batch_size; i++)
    // {
    //     mask(i) = delta(i) < 0 ? std::numeric_limits< double >::max() : 0;
    // }

    Eigen::Vector<double,batch_size> mask = (delta.array() < 0.0).select(inf,zeros);

    Eigen::Vector<double,batch_size> delta_sqrt = delta.cwiseAbs().cwiseSqrt();
    Eigen::Vector<double,batch_size> s1 = (-b - delta_sqrt) / (2* a_scalar);
    Eigen::Vector<double,batch_size> s2 = (-b + delta_sqrt) / (2* a_scalar);
    Eigen::Vector<double,batch_size> s = s1.cwiseMin(s2);
    s = s.cwiseMax(mask);

    Eigen::Index minIndex;
    double min = s.minCoeff(&minIndex);
    return {min, ray.point + s(minIndex) * ray.dir,minIndex};
}

template<int noOfSpheres, int batch_size>
void Render::renderImageSIMDSpheres()
{
    Eigen::Vector4d screenUp, screenRight;
    Eigen::Vector4d centralRay = camera.screenCenter - camera.pos;
    screenRight                = centralRay.cross3(camera.up).normalized();
    screenUp                   = screenRight.cross3(centralRay).normalized();
    double fov                 = camera.fov * (std::numbers::pi / 180);
    double step                = std::tan(fov / 2) * centralRay.norm() / (width / 2);

    // Eigen::Matrix<double,4,noOfSpheres> centers;
    // Eigen::Vector<double,noOfSpheres> radius2;
    // auto && [centers, radius2] = prepareSpheresMatrix<noOfSpheres>(objs);

    //auto t_start = std::chrono::high_resolution_clock::now();
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
                        //std::cout << std::to_string(std::get<0>(res)) + "\n";
                        if (std::get<0>(res) < z_buffor)
                        {
                            z_buffor        = std::get<0>(res);
                            nearestObjIndex = batch_size * k + std::get<2>(res);
                            sectionPoint    = std::get<1>(res).value();
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
    //auto t_end = std::chrono::high_resolution_clock::now();
    //std::cout << "Time SIMD: " << std::chrono::duration<double, std::milli>(t_end-t_start).count() << "ms" << std::endl;

    //img.saveToBmp(path);
}
