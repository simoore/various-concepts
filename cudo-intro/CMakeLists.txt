cmake_minimum_required(VERSION 3.26)
project(CudaIntro LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 20)

add_executable(CudaHello src/hello.cu)
add_executable(CudaProp src/properties.cu)
add_executable(CudaSum src/kernel.cu)

