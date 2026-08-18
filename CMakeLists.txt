cmake_minimum_required(VERSION 3.20)

project(MotionPlusPlus VERSION 0.1.0 LANGUAGES CXX)

find_package(LibMotionPlusPlus REQUIRED)
find_package(tomlplusplus REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Install path" FORCE)
endif()

option(INSTALL_UDEV_RULES "Install udev rule for virtual devices access" OFF)

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
    -Wpedantic
    -Werror
)

install(TARGETS motionplusplus
    RUNTIME DESTINATION bin
)

if(INSTALL_UDEV_RULES)
    install(FILES udev/99-motionplusplus.rules DESTINATION /etc/udev/rules.d)
    install(CODE "execute_process(COMMAND udevadm control --reload-rules)")
endif()

install(FILES conf.example.toml
    DESTINATION $ENV{HOME}/.config/motionplusplus
)

install(FILES srv/motionplusplus.service
    DESTINATION $ENV{HOME}/.config/systemd/user
)

