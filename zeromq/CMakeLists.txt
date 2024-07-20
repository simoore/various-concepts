cmake_minimum_required(VERSION 3.26)
project(zmqservices LANGUAGES C CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(BUILD_SHARED_LIBS OFF)

add_compile_options(-Og -Wall -DZMQ_STATIC -Wno-language-extension-token)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1)
FetchContent_MakeAvailable(spdlog)

add_executable(zmqservices src/main.cpp)
target_link_libraries(zmqservices PRIVATE libzmq spdlog)
target_include_directories(zmqservices PRIVATE include)
