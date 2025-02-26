cmake_minimum_required(VERSION 3.26)
project(CppConcepts LANGUAGES CXX)

include(FetchContent)

add_compile_options(-std=c++23 -Og -Wall)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.1)
FetchContent_MakeAvailable(spdlog)

function(boost_app target srcs)
    add_executable(${target} ${srcs})
    target_include_directories(${target} PRIVATE $ENV{BOOST_ROOT} spdlog)
    target_compile_definitions(${target} PRIVATE _WIN32_WINNT=0x0A00)
endfunction()

boost_app(simpletimer src/simple_timer.cpp)
boost_app(syncserver src/sync_echo_server.cpp)
boost_app(syncclient src/sync_echo_client.cpp)
boost_app(websockets src/websockets.cpp)
