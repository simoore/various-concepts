cmake_minimum_required(VERSION 3.26)
project(KalmanFilter LANGUAGES CXX)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

add_compile_options(-std=c++23 -Og -Wall)

SET(CommonSources 
    src/main.cpp
    src/display.cpp
    src/beacons.cpp
    src/sensors.cpp
    src/simulation.cpp
    src/utils.cpp)

add_executable(KalmanFilterLinear ${CommonSources} src/kalmanfilter_lkf_student.cpp)
target_compile_definitions(KalmanFilterLinear PRIVATE _USE_MATH_DEFINES)
target_link_libraries(KalmanFilterLinear mingw32 SDL2main SDL2 SDL2_ttf Eigen3::Eigen)

# Copies the font file into the build directory
add_custom_command(TARGET KalmanFilterLinear PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:KalmanFilterLinear>)
