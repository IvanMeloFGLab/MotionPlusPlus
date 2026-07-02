cmake_minimum_required(VERSION 3.20)

project(MotionPlusPlus)

set(CMAKE_CXX_COMPILER g++-14)
set(CMAKE_CXX_STANDARD 23)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEVDEV REQUIRED IMPORTED_TARGET libevdev)

add_executable(motionplusplus
    src/main.cpp
    src/DeviceManager.cpp
    src/DeviceConnection.cpp
)

target_include_directories(motionplusplus PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(motionplusplus PRIVATE PkgConfig::LIBEVDEV)

target_compile_options(motionplusplus PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)
