cmake_minimum_required(VERSION 3.26)
project(MTLS LANGUAGES CXX)

include(FetchContent)
find_package(OpenSSL)

add_compile_options(-std=c++23 -Og -Wall)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.1)
FetchContent_MakeAvailable(spdlog)

function(boost_app target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE spdlog)
endfunction()

boost_app(cancellogic src/cancel_logic.cpp)
boost_app(simpleclient src/simple_http_client.cpp)
boost_app(tlsserver src/async_http_server_ssl.cpp src/request_handler.cpp)
target_include_directories(tlsserver PRIVATE include)
target_link_libraries(tlsserver PRIVATE OpenSSL::SSL OpenSSL::Crypto)
boost_app(tlsclient src/async_http_client_ssl.cpp)
target_link_libraries(tlsclient PRIVATE OpenSSL::SSL OpenSSL::Crypto)
