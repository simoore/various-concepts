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

add_executable(KalmanFilterExtended ${CommonSources} src/kalmanfilter_ekf_student.cpp)
target_compile_definitions(KalmanFilterExtended PRIVATE _USE_MATH_DEFINES)
target_link_libraries(KalmanFilterExtended mingw32 SDL2main SDL2 SDL2_ttf Eigen3::Eigen)

add_executable(KalmanFilterUnscented ${CommonSources} src/kalmanfilter_ukf_student.cpp)
target_compile_definitions(KalmanFilterUnscented PRIVATE _USE_MATH_DEFINES)
target_link_libraries(KalmanFilterUnscented mingw32 SDL2main SDL2 SDL2_ttf Eigen3::Eigen)

add_executable(Capstone ${CommonSources} src/capstone.cpp)
target_compile_definitions(Capstone PRIVATE _USE_MATH_DEFINES)
target_link_libraries(Capstone mingw32 SDL2main SDL2 SDL2_ttf Eigen3::Eigen)

add_executable(TestLidar src/test_lidar.cpp src/utils.cpp)
target_compile_definitions(TestLidar PRIVATE _USE_MATH_DEFINES)
target_link_libraries(TestLidar Eigen3::Eigen)

# Copies the font file into the build directory
add_custom_command(TARGET KalmanFilterLinear PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:KalmanFilterLinear>)
