#include <iostream>
#include <Eigen/Dense>
#include <vector>
#include "obj.hpp"
#include "camera.hpp"
#include "render.hpp"



int main()
{
	Eigen::Vector4d pos(-5.0,-5.0,5.0,1.0);
	Eigen::Vector4d screen(-4.0,-4.0,4.0,1.0);
	Eigen::Vector4d up(0.0,0.0,1.0,0.0);

	Camera cam(pos,screen,up,80.0);

	std::vector<Sphere> objs;
	Eigen::Vector4d center(0.0,1,0.0,1.0);
	objs.push_back(Sphere(center,1.0, Color(255,0,0)));
	Eigen::Vector4d center2(0.0,-0.5,0.0,1.0);
	objs.push_back(Sphere(center2,1.0,Color(0,255,0)));
	Eigen::Vector4d center3(0.0,0.0,0.5,1.0);
	objs.push_back(Sphere(center3,1.0,Color(0,0,255)));


	Render render(cam,objs);

	render.renderImage(1920,1080,"test.bmp");
}

