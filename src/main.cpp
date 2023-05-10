#include "Obj.hpp"
#include "Render.hpp"

#include <Eigen/Dense>
#include <cxxopts.hpp>

#include <filesystem>
#include <iostream>
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
    return p;
}

int main(int argc, char** argv)
{
    Params param = parseArgs(argc, argv);

    Eigen::Vector4d pos(-5.0, -5.0, 5.0, 1.0);
    Eigen::Vector4d screen(-4.0, -4.0, 4.0, 1.0);
    Eigen::Vector4d up(0.0, 0.0, 1.0, 0.0);

    Camera cam(pos, screen, up, 80.0);

    std::vector< Sphere > objs;
    Eigen::Vector4d       center(0.0, 2, 0.0, 1.0);
    objs.push_back(Sphere(center, 2.0, Color(255, 0, 0)));
    Eigen::Vector4d center2(0.0, -2, 0.0, 1.0);
    objs.push_back(Sphere(center2, 2.0, Color(0, 255, 0)));
    Eigen::Vector4d center3(0.0, 0.0, 1, 1.0);
    objs.push_back(Sphere(center3, 2.0, Color(0, 0, 255)));

    std::vector< Light > lights;
    Eigen::Vector4d      l1(0.0, 0.0, 5.0, 1.0);
    lights.push_back(Light(l1, Color(255, 255, 255)));
    Eigen::Vector4d l2(-5.0, 0.0, 0.0, 1.0);
    lights.push_back(Light(l2, Color(255, 255, 255)));
    Eigen::Vector4d l3(.0, 0. - 5, 0.0, 1.0);
    lights.push_back(Light(l3, Color(255, 255, 255)));

    Render render(cam, objs, lights);

    if (param.presentation)
    {
        fs::remove_all("show");
        if (!fs::create_directory("show"))
            std::cerr << "Can not create communication folder";
        for (size_t i = 0; i < 20; i++)
        {
            double fi        = 2.0 + i / 5.0;
            cam.pos          = Eigen::Vector4d(10.0 * cos(fi), 10.0 * sin(fi), 5.0, 1.0);
            cam.screenCenter = Eigen::Vector4d(9.0 * cos(fi), 9.0 * sin(fi), 4.5, 1.0);
            render.renderImage(param.mode, param.width, param.height, "show/" + std::to_string(i) + ".bmp");
        }
    }
    else
    {
        render.renderImage(param.mode, param.width, param.height, param.path);
    }
}
