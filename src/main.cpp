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
	Eigen::Vector4d center(0.0,2,0.0,1.0);
	objs.push_back(Sphere(center,2.0, Color(255,0,0)));
	Eigen::Vector4d center2(0.0,-2,0.0,1.0);
	objs.push_back(Sphere(center2,2.0,Color(0,255,0)));
	Eigen::Vector4d center3(0.0,0.0,1,1.0);
	objs.push_back(Sphere(center3,2.0,Color(0,0,255)));

	std::vector<Light> lights;
	Eigen::Vector4d l1(0.0,0.0,5.0,1.0);
	lights.push_back(Light(l1, Color(255,255,255)));
	Eigen::Vector4d l2(-5.0,0.0,0.0,1.0);
	lights.push_back(Light(l2, Color(255,255,255)));


	Render render(cam,objs,lights);

	render.renderImage(1920,1080,"test.bmp");
}

