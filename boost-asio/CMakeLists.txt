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
    target_link_libraries(${target} PRIVATE spdlog)
    if (WIN32)
        target_include_directories(${target} PRIVATE $ENV{BOOST_ROOT})
        target_compile_definitions(${target} PRIVATE _WIN32_WINNT=0x0A00)
    endif()
endfunction()

boost_app(asyncserver src/async_echo_server.cpp)
boost_app(coroserver src/coroutines_echo_server.cpp)
boost_app(simpletimer src/simple_timer.cpp)
boost_app(syncserver src/sync_echo_server.cpp)
boost_app(syncclient src/sync_echo_client.cpp)
boost_app(mtlsclient src/mtls_client.cpp)
target_link_libraries(mtlsclient PRIVATE ssl crypto)
boost_app(mtlsserver src/mtls_server.cpp)
target_link_libraries(mtlsserver PRIVATE ssl crypto)
