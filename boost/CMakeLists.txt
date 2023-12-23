cmake_minimum_required(VERSION 3.26)
project(CppConcepts LANGUAGES CXX)

add_compile_options(-std=c++23 -Og -Wall)

add_executable(simpletimer src/simple_timer.cpp)
target_link_libraries(simpletimer ws2_32)

add_executable(syncserver src/sync_echo_server.cpp)
target_link_libraries(syncserver ws2_32)

add_executable(syncclient src/sync_echo_client.cpp)
target_link_libraries(syncclient ws2_32)
