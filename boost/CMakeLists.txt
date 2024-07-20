cmake_minimum_required(VERSION 3.26)
project(CppConcepts LANGUAGES CXX)

include(FetchContent)

add_compile_options(-std=c++23 -Og -Wall)

add_executable(simpletimer src/simple_timer.cpp)
target_link_libraries(simpletimer PRIVATE ws2_32)

add_executable(syncserver src/sync_echo_server.cpp)
target_link_libraries(syncserver PRIVATE ws2_32)

add_executable(syncclient src/sync_echo_client.cpp)
target_link_libraries(syncclient PRIVATE ws2_32)

add_executable(websockets src/websockets.cpp)
target_link_libraries(websockets PRIVATE ws2_32)
