cmake_minimum_required(VERSION 3.2)
project(AFirstExample 
    LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE on)

include_directories(
    $ENV{DEVLIBDIR}/googletest-release-1.11.0/googlemock/include 
    $ENV{DEVLIBDIR}/googletest-release-1.11.0/googletest/include 
)
link_directories(
    $ENV{DEVLIBDIR}/googletest-release-1.11.0/build/lib
)
add_definitions(-std=c++17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")


set(sources 
   SoundexTest.cpp)
add_executable(unittests ${sources})
target_link_libraries(unittests pthread)
target_link_libraries(unittests gmock)
target_link_libraries(unittests gtest)