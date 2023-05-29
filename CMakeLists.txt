cmake_minimum_required(VERSION 3.8)
project(ray CXX)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(cxxopts REQUIRED)
find_package(TBB REQUIRED)
find_package(benchmark REQUIRED)


add_executable(ray src/main.cpp)
set_property(TARGET ray PROPERTY CXX_STANDARD 20)
target_include_directories(ray PUBLIC include)
target_compile_features(ray PUBLIC cxx_std_20)
target_link_libraries(ray cxxopts::cxxopts Eigen3::Eigen TBB::tbb benchmark::benchmark)

add_executable(benchmark src/benchmark.cpp)
set_property(TARGET benchmark PROPERTY CXX_STANDARD 20)
target_include_directories(benchmark PUBLIC include)
target_compile_features(benchmark PUBLIC cxx_std_20)
target_link_libraries(benchmark Eigen3::Eigen TBB::tbb benchmark::benchmark)