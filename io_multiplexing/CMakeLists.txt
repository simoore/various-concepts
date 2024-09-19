cmake_minimum_required (VERSION 3.22)
project (IoMultiplexing LANGUAGES CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)

add_executable(Poll)
target_sources(Poll PRIVATE src/poll.cpp)
target_compile_options(Poll PRIVATE -Wall -Wextra)
