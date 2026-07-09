cmake_minimum_required(VERSION 3.20)

project(LibMotionPlusPlus VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_COMPILER g++-14)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build shared instead of static libs" OFF)
option(MOTIONPLUSPLUS_BUILD_EXAMPLES "Build example programs" ON)
option(INSTALL_UDEV_RULES "Install udev rule for controllers device access" OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEVDEV REQUIRED IMPORTED_TARGET libevdev)

add_library(libmotionplusplus
    src/DeviceManager.cpp
    src/DeviceConnection.cpp
    src/Controller.cpp
    src/WiiMote.cpp
    src/ControllerManager.cpp
)

add_library(LibMotionPlusPlus::libmotionplusplus ALIAS libmotionplusplus)

target_include_directories(libmotionplusplus
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        src
)

target_link_libraries(libmotionplusplus PUBLIC PkgConfig::LIBEVDEV)

target_compile_features(libmotionplusplus PUBLIC cxx_std_23)

target_compile_options(libmotionplusplus PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

set_target_properties(libmotionplusplus PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

if (MOTIONPLUSPLUS_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

# --- Install: library, headers, and CMake package config ---
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS libmotionplusplus
    EXPORT LibMotionPlusPlusTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/libmotionplusplus
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT LibMotionPlusPlusTargets
    FILE LibMotionPlusPlusTargets.cmake
    NAMESPACE LibMotionPlusPlus::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LibMotionPlusPlus
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/LibMotionPlusPlusConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LibMotionPlusPlusConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LibMotionPlusPlus
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/LibMotionPlusPlusConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/LibMotionPlusPlusConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/LibMotionPlusPlusConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LibMotionPlusPlus
)

if(INSTALL_UDEV_RULES)
    install(FILES udev/99-libmotionplusplus.rules DESTINATION /etc/udev/rules.d)
    install(CODE "execute_process(COMMAND udevadm control --reload-rules)")
endif()
