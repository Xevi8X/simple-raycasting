#pragma once
#include <Eigen/Dense>
#include <vector>
#include <string>
#include "camera.hpp"
#include "obj.hpp"
#include "bmp.hpp"
#include "light.hpp"

enum RenderMode
{
    CPU,TBB
};

class Render{
    public:
        Render(Camera& cam, std::vector<Sphere>& objects, std::vector<Light>& lights);
        void renderImage(RenderMode mode ,int width, int height, std::string path);

    private:
        Camera& camera;
        std::vector<Sphere>& objs;
        std::vector<Light>& lights;
        void renderImageCPU(int width, int height, std::string path);
        void renderImageTBB(int width, int height, std::string path);
};