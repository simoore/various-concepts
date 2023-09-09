cmake_minimum_required(VERSION 3.25)
project(CapnpApp LANGUAGES C CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)

###############################################################################
# Third Party Libraries
###############################################################################

include(ExternalProject)

ExternalProject_Add(
    c-capnproto
    GIT_REPOSITORY https://github.com/opensourcerouting/c-capnproto
    GIT_TAG 632f0d73a1f4a03026b5e4727386b9fe3ec6e00e
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND meson setup build
    BUILD_COMMAND meson compile -C build
    INSTALL_COMMAND "")

add_library(CCapnp INTERFACE)
target_include_directories(CCapnp INTERFACE ${CMAKE_BINARY_DIR}/c-capnproto-prefix/src/c-capnproto/lib)
add_dependencies(CCapnp c-capnproto)

###############################################################################
# NanoPB Code Generation
###############################################################################

SET(CapnpPlugin ${CMAKE_BINARY_DIR}/c-capnproto-prefix/src/c-capnproto/build/capnpc-c)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/capnp/addressbook.capnp.c ${CMAKE_SOURCE_DIR}/capnp/addressbook.capnp.h
    COMMAND capnp compile -o ${CapnpPlugin} ${CMAKE_SOURCE_DIR}/capnp/addressbook.capnp
    DEPENDS ${CMAKE_SOURCE_DIR}/capnp/addressbook.capnp
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/capnp
    VERBATIM)

###############################################################################
# NanoTbApp Executable
###############################################################################

add_executable(CapnpApp src/main.cpp ${CMAKE_SOURCE_DIR}/capnp/addressbook.capnp.c)
target_link_libraries(CapnpApp PRIVATE CCapnp)
