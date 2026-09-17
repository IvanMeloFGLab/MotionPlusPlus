cmake_minimum_required(VERSION 3.20)

project(MotionPlusPlus VERSION 1.0.0 LANGUAGES CXX)

find_package(LibMotionPlusPlus REQUIRED)
find_package(tomlplusplus REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Install path" FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(motionplusplus
    src/motionplusplus.cpp
    src/VirtualController.cpp
)

target_include_directories(motionplusplus
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        src
)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/generated/KeyCodeMap.cpp
    COMMAND python3 ${CMAKE_SOURCE_DIR}/scripts/generate_key_map.py
            ${CMAKE_BINARY_DIR}/generated/KeyCodeMap.cpp
    DEPENDS ${CMAKE_SOURCE_DIR}/scripts/generate_key_map.py
    COMMENT "Generating key code lookup table from kernel headers"
)

target_sources(motionplusplus PRIVATE ${CMAKE_BINARY_DIR}/generated/KeyCodeMap.cpp)

target_link_libraries(motionplusplus PRIVATE
    LibMotionPlusPlus::libmotionplusplus
    tomlplusplus::tomlplusplus
)

target_compile_features(motionplusplus PUBLIC cxx_std_23)

target_compile_options(motionplusplus PRIVATE
    -Wall
    -Wextra
    $<$<CONFIG:Debug>:-Wpedantic>
    $<$<CONFIG:Debug>:-Werror>
)

install(TARGETS motionplusplus
    RUNTIME DESTINATION bin
)

install(FILES conf.example.toml
    DESTINATION $ENV{HOME}/.config/motionplusplus
)

install(FILES srv/motionplusplus.service
    DESTINATION $ENV{HOME}/.config/systemd/user
)

install(CODE "execute_process(COMMAND systemctl --user daemon-reload)")

install(FILES udev/99-motionplusplus.rules
    DESTINATION /etc/udev/rules.d
    COMPONENT udev_rules
    EXCLUDE_FROM_ALL
)

