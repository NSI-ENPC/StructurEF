cmake_minimum_required(VERSION 3.10)
project(StructEF)

find_package(Eigen3 REQUIRED)

add_executable(StructEF mailleur.cpp header.h)
target_link_libraries (StructEF Eigen3::Eigen)
