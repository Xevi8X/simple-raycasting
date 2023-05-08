#include <Eigen/Dense>
#include <vector>
#include <string>
#include "camera.hpp"
#include "obj.hpp"
#include "bmp.hpp"

class Render{
    public:
        Render(Camera cam, std::vector<Sphere>& objects);
        void renderImage(int width, int height, std::string path);
    private:
        Camera camera;
        std::vector<Sphere>& objs;
};