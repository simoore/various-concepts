cmake_minimum_required(VERSION 3.26)
cmake_policy(VERSION 3.26)
project(zmqservices LANGUAGES C CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)

add_compile_options(-Og -Wall -DZMQ_STATIC -Wno-language-extension-token)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++23>)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(CPPZMQ_BUILD_TESTS OFF CACHE BOOL "")
set(WITH_DOCS OFF CACHE BOOL "")
set(BUILD_SHARED OFF CACHE BOOL "")
set(BUILD_TESTS OFF CACHE BOOL "")

FetchContent_Declare(
    libzmq
    GIT_REPOSITORY https://github.com/zeromq/libzmq.git
    GIT_TAG v4.3.5)
FetchContent_MakeAvailable(libzmq)

FetchContent_Declare(
    cppzmq
    GIT_REPOSITORY https://github.com/zeromq/cppzmq.git
    GIT_TAG v4.10.0)
FetchContent_MakeAvailable(cppzmq)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1)
FetchContent_MakeAvailable(spdlog)

add_executable(zmqservices src/main_zeromq.cpp)
target_link_libraries(zmqservices PRIVATE cppzmq-static spdlog)
target_include_directories(zmqservices PRIVATE include)

add_executable(websocketservices src/main_websockets.cpp)
target_link_libraries(websocketservices PRIVATE Boost::beast spdlog)
target_include_directories(websocketservices PRIVATE include)
