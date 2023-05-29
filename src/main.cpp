#include "Obj.hpp"
#include "Render.hpp"

#include <Eigen/Dense>
#include <cxxopts.hpp>

#include <filesystem>
#include <iostream>
#include <random>
#include <vector>

namespace fs = std::filesystem;

struct Params
{
    int         height, width;
    std::string path;
    RenderMode  mode;
    bool        presentation;
};

Params parseArgs(int argc, char** argv)
{
    Params           p;
    cxxopts::Options options("ray", "Sphere raycasting");
    options.add_options()(
        "s,shape", "Shape of output image", cxxopts::value< std::vector< int > >()->default_value("1920,1080"))(
        "f,file", "Output file name", cxxopts::value< std::string >()->default_value("scene.bmp"))(
        "c,cpu", "CPU mode", cxxopts::value< bool >()->default_value("false"))(
        "t,tbb", "TBB mode", cxxopts::value< bool >()->default_value("false"))(
        "m,simd", "SIMD mode", cxxopts::value< bool >()->default_value("false"))(
        "p,show", "Generate 20 bmp from rotating camera", cxxopts::value< bool >()->default_value("false"))(
        "h,help", "Print usage");
    auto result = options.parse(argc, argv);
    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    auto shape     = result["shape"].as< std::vector< int > >();
    p.width        = shape[0];
    p.height       = shape[1];
    p.path         = result["file"].as< std::string >();
    p.presentation = result["show"].as< bool >();

    if (result["cpu"].as< bool >())
    {
        p.mode = RenderMode::CPU;
    }
    if (result["tbb"].as< bool >())
    {
        p.mode = RenderMode::TBB;
    }
    if (result["simd"].as< bool >())
    {
        p.mode = RenderMode::SIMD;
    }
    return p;
}

void generateSpheres(Obj3D** objs, size_t noOfSpheres)
{
    std::mt19937                             mt(2023);
    std::uniform_real_distribution< double > r(0.5, 5);
    std::uniform_real_distribution< double > coord(-80, 80);
    std::uniform_int_distribution< uint8_t > color(0, 255);

    auto randR = [&r, &mt]() {
        return r(mt);
    };
    auto randCoord = [&coord, &mt]() {
        return coord(mt);
    };
    auto randColor = [&color, &mt]() {
        return Color{color(mt), color(mt), color(mt)};
    };

    for (size_t i = 0; i < noOfSpheres; i++)
    {
        double          R = randR();
        Eigen::Vector4d center(randCoord(), randCoord(), R, 1);
        objs[i] = new Sphere(center, R, randColor());
    }
}

int main(int argc, char** argv)
{
    constexpr size_t noOfSpheres = 1024;
    constexpr size_t noOfPlanes  = 0;

    Params param = parseArgs(argc, argv);

    Eigen::Vector4d pos(-100.0, -100.0, 30.0, 1.0);
    Eigen::Vector4d screen(-90.0, -90.0, 25.0, 1.0);
    Eigen::Vector4d up(0.0, 0.0, 1.0, 0.0);

    Camera cam(pos, screen, up, 80.0);

    std::vector< Light > lights;
    Eigen::Vector4d      l1(-50.0, -50.0, 50.0, 1.0);
    lights.push_back(Light(l1, Color(255, 255, 255)));
    Eigen::Vector4d      l2(50.0, -50.0, 50.0, 1.0);
    lights.push_back(Light(l2, Color(255, 255, 255)));
    Eigen::Vector4d      l3(-50.0, 50.0, 50.0, 1.0);
    lights.push_back(Light(l3, Color(255, 255, 255)));
    Eigen::Vector4d      l4(50.0, 50.0, 50.0, 1.0);
    lights.push_back(Light(l4, Color(255, 255, 255)));

    Obj3D* objs[noOfSpheres + noOfPlanes];
    generateSpheres(objs, noOfSpheres);
    objs[noOfSpheres] = new Plane();
    std::cout << "Allocation done" << std::endl;

    Render render(cam, lights, objs, noOfPlanes + noOfSpheres);

    if (param.presentation)
    {
        fs::remove_all("show");
        if (!fs::create_directory("show"))
            std::cerr << "Can not create communication folder";
        for (size_t i = 0; i < 60; i++)
        {
            double fi        = i / 5.0;
            cam.pos          = Eigen::Vector4d(100.0 * cos(fi), 100.0 * sin(fi), 30.0, 1.0);
            cam.screenCenter = Eigen::Vector4d(90.0 * cos(fi), 90.0 * sin(fi), 25.0, 1.0);
            render.prepare(param.width, param.height);
            render.renderImage(param.mode);
            render.saveTo( "show/" + std::to_string(i) + ".bmp");
        }
        if(system("ffmpeg -f image2 -i ./show/%d.bmp ./show/out.mov"))
        {
            std::cerr << "FFMPEG return " << errno << std::endl;
        }
    }
    else
    {
        render.prepare(param.width, param.height);
        render.renderImage(param.mode);
        render.saveTo(param.path);
    }

    std::cout << "Free mem" << std::endl;

    for (size_t i = 0; i < noOfSpheres + noOfPlanes; i++)
    {
        delete (objs[i]);
    }
}
