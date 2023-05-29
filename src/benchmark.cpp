#include "benchmark/benchmark.h"
#include "Obj.hpp"
#include "Render.hpp"

#include <Eigen/Dense>

#include <filesystem>
#include <iostream>
#include <random>
#include <vector>

constexpr size_t noOfSpheres = 1024;
std::vector< Light > lights;
Camera cam;
Obj3D* objs[noOfSpheres];

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

Render prapareSpheres()
{
    Eigen::Vector4d pos(-100.0, -100.0, 30.0, 1.0);
    Eigen::Vector4d screen(-90.0, -90.0, 25.0, 1.0);
    Eigen::Vector4d up(0.0, 0.0, 1.0, 0.0);

    cam = Camera(pos, screen, up, 80.0);

    lights.clear();
    Eigen::Vector4d      l1(-50.0, -50.0, 50.0, 1.0);
    lights.push_back(Light(l1, Color(255, 255, 255)));
    Eigen::Vector4d      l2(50.0, -50.0, 50.0, 1.0);
    lights.push_back(Light(l2, Color(255, 255, 255)));
    Eigen::Vector4d      l3(-50.0, 50.0, 50.0, 1.0);
    lights.push_back(Light(l3, Color(255, 255, 255)));
    Eigen::Vector4d      l4(50.0, 50.0, 50.0, 1.0);
    lights.push_back(Light(l4, Color(255, 255, 255)));

    generateSpheres(objs, noOfSpheres);
    auto r = Render(cam, lights, objs, noOfSpheres);
    r.prepare(1920,1080);
    return r;
}

static void BM_Prepare(benchmark::State& state) 
{
    for (auto _ : state) 
    {
        auto render =  prapareSpheres();
    }
}

static void BM_Serial(benchmark::State& state) 
{
    auto render =  prapareSpheres();

    for (auto _ : state) 
    {
        render.renderImage(RenderMode::CPU);
    }
}

static void BM_TBB(benchmark::State& state) 
{
    auto render =  prapareSpheres();

    for (auto _ : state) 
    {
        render.renderImage(RenderMode::TBB);
    }
}

static void BM_Simd(benchmark::State& state) {
    int size = state.range(0);
    
    auto render =  prapareSpheres();

    for (auto _ : state) 
    {
        render.renderImage(RenderMode::SIMD,size);
    }
}

static void BM_Save(benchmark::State& state) {
    int size = state.range(0);
    
    auto render =  prapareSpheres();
    render.renderImage(RenderMode::SIMD,8);

    for (auto _ : state) 
    {
        render.saveTo("benchmark.bmp");
    }
}


BENCHMARK(BM_Prepare)->UseRealTime()->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Serial)->UseRealTime()->Unit(benchmark::kMillisecond);
BENCHMARK(BM_TBB)->Iterations(3)->UseRealTime()->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Simd)->Iterations(3)->UseRealTime()->Unit(benchmark::kMillisecond)->RangeMultiplier(2)->Range(2, 128);
BENCHMARK(BM_Save)->UseRealTime()->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();