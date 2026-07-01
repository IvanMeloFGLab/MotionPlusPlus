cmake_minimum_required(VERSION 3.20)

project(MotionPlusPlus)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEVDEV REQUIRED IMPORTED_TARGET libevdev)

add_executable(MotionPlusPlus
    src/main.cpp
    src/DeviceManager.cpp
)

target_include_directories(MotionPlusPlus PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(MotionPlusPlus PRIVATE PkgConfig::LIBEVDEV)

target_compile_options(MotionPlusPlus PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)
