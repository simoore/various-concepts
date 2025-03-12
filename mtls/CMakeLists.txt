cmake_minimum_required(VERSION 3.26)
project(MTLS LANGUAGES CXX)

include(FetchContent)

add_compile_options(-std=c++23 -Og -Wall)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.1)
FetchContent_MakeAvailable(spdlog)

function(boost_app target srcs)
    add_executable(${target} ${srcs})
    target_link_libraries(${target} PRIVATE spdlog)
endfunction()

boost_app(simpleclient src/simple_http_client.cpp)
boost_app(asyncclient src/async_http_client.cpp)
boost_app(asyncserver src/async_http_server.cpp)
