cmake_minimum_required(VERSION 3.20)

project(MotionPlusPlus VERSION 0.1.0 LANGUAGES CXX)

find_package(LibMotionPlusPlus REQUIRED)

add_executable(motionplusplus
    src/motionplusplus.cpp
)

target_include_directories(motionplusplus
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        src
)

target_link_libraries(motionplusplus PRIVATE LibMotionPlusPlus::libmotionplusplus)

target_compile_features(motionplusplus PUBLIC cxx_std_23)

target_compile_options(motionplusplus PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

set_target_properties(motionplusplus PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
