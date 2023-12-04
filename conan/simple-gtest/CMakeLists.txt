cmake_minimum_required(VERSION 3.23)
project(SimpleGTest CXX)

find_package(GTest REQUIRED)

add_executable(simple-gtest src/main.cpp)
target_link_libraries(simple-gtest gtest::gtest)
target_link_options(simple-gtest PRIVATE -static-libasan -static-libgcc -static-libstdc++)