cmake_minimum_required(VERSION 3.26)
project(CppConcepts LANGUAGES CXX)

add_compile_options(-std=c++23 -Og -Wall)

add_executable(simpletimer src/simple_timer.cpp)
target_link_libraries(simpletimer wsock32)
