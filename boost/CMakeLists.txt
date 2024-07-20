cmake_minimum_required(VERSION 3.26)
project(CppConcepts LANGUAGES CXX)

include(FetchContent)

add_compile_options(-std=c++23 -Og -Wall)

set(FETCHCONTENT_QUIET OFF)

function(FetchBoostLib Name)
    FetchContent_Declare(
        Boost_${Name}
        GIT_REPOSITORY https://github.com/boostorg/${Name}.git
        OVERRIDE_FIND_PACKAGE
        GIT_TAG boost-1.85.0
        GIT_PROGRESS ON
        GIT_SHALLOW ON
    )
    FetchContent_MakeAvailable(Boost_${Name})
endfunction()

set(BOOST_INCLUDE_LIBRARIES 
    asio
    assert
    beast
    bind
    config
    container
    core
    detail
    endian
    intrusive
    io
    logic
    move
    mp11
    optional
    predef
    preprocessor
    smart_ptr
    static_assert
    static_string
    throw_exception 
    type_traits 
    utility)

foreach(name ${BOOST_INCLUDE_LIBRARIES})
    FetchBoostLib(${name})
endforeach()
# ws2_32

add_executable(simpletimer src/simple_timer.cpp)
target_link_libraries(simpletimer)

add_executable(syncserver src/sync_echo_server.cpp)
target_link_libraries(syncserver)

add_executable(syncclient src/sync_echo_client.cpp)
target_link_libraries(syncclient)

add_executable(syncclient src/sync_echo_client.cpp)
target_link_libraries(syncclient)

add_executable(websockets src/websockets.cpp)
target_link_libraries(websockets PRIVATE boost::beat)
