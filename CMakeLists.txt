cmake_minimum_required(VERSION 3.8)
project(ray CXX)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB SOURCES ${SOURCE_DIR}/*.cpp)

add_executable(ray ${SOURCES})
set_property(TARGET ray PROPERTY CXX_STANDARD 20)
target_include_directories(ray PUBLIC include)
target_compile_features(ray PUBLIC cxx_std_20)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(cxxopts REQUIRED)
find_package(TBB REQUIRED)
target_link_libraries(ray cxxopts::cxxopts Eigen3::Eigen TBB::tbb)
