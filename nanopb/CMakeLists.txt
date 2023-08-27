cmake_minimum_required(VERSION 3.25)
project(NanoPbApp LANGUAGES C CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)

###############################################################################
# Third Party Libraries
###############################################################################

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
    nanopb
    GIT_REPOSITORY https://github.com/nanopb/nanopb.git
    GIT_TAG 0.4.7)
FetchContent_MakeAvailable(nanopb)

###############################################################################
# NanoPB Code Generation
###############################################################################

SET(ProtoGenerator ${CMAKE_BINARY_DIR}/_deps/nanopb-src/generator/nanopb_generator.py)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/proto/messages.pb.c ${CMAKE_SOURCE_DIR}/proto/messages.pb.h
    COMMAND python ${ProtoGenerator} --cpp-descriptors -I${CMAKE_SOURCE_DIR}/proto messages.proto
    DEPENDS ${CMAKE_SOURCE_DIR}/proto/messages.proto
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/proto
    VERBATIM)

###############################################################################
# NanoTbApp Executable
###############################################################################

add_executable(NanoPbApp src/main.cpp ${CMAKE_SOURCE_DIR}/proto/messages.pb.c)
target_link_libraries(NanoPbApp PRIVATE protobuf-nanopb-static)
target_include_directories(NanoPbApp PRIVATE proto)
target_link_options(NanoPbApp PRIVATE -static-libasan -static-libgcc -static-libstdc++)