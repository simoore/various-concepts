cmake_minimum_required(VERSION 3.26)
project(mygrpc LANGUAGES C CXX)

include(FetchContent)

set(BUILD_SHARED_LIBS off)
add_compile_options(-Og)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++23>)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.1)
FetchContent_MakeAvailable(spdlog)

set(ABSL_ENABLE_INSTALL ON)
FetchContent_Declare(
    grpc
    GIT_REPOSITORY https://github.com/grpc/grpc.git
    GIT_TAG v1.71.0)
FetchContent_MakeAvailable(grpc)

set(grpc_cpp_plugin $<TARGET_FILE:grpc_cpp_plugin>)
set(mygrpc_proto_src ${CMAKE_CURRENT_SOURCE_DIR}/protos/mygrpc/v1/greeter.proto)
set(generated_path ${CMAKE_CURRENT_SOURCE_DIR}/generated)
set(generated_protos 
    ${generated_path}/mygrpc/v1/greeter.pb.cc
    ${generated_path}/mygrpc/v1/greeter.pb.h
    ${generated_path}/mygrpc/v1/greeter.grpc.pb.cc
    ${generated_path}/mygrpc/v1/greeter.grpc.pb.h)

add_custom_command(
    OUTPUT ${generated_protos}
    COMMAND protoc
    ARGS 
        --grpc_out ${generated_path}
        --cpp_out ${generated_path}
        -I ${CMAKE_CURRENT_SOURCE_DIR}/protos
        --plugin=protoc-gen-grpc=${grpc_cpp_plugin}
        ${mygrpc_proto_src}
    DEPENDS ${mygrpc_proto_src})

add_library(mygrpc_proto_lib STATIC ${generated_protos})
target_include_directories(mygrpc_proto_lib PUBLIC ${generated_path})
target_link_libraries(mygrpc_proto_lib PUBLIC grpc libprotobuf grpc++_reflection)

function(grpc_app target srcs)
    add_executable(${target} ${srcs})
    target_link_libraries(${target} PRIVATE spdlog mygrpc_proto_lib)
endfunction()

grpc_app(greeterserver src/greeter_server.cpp)

