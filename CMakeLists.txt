cmake_minimum_required(VERSION 3.20)

project(MotionPlusPlus VERSION 0.1.0 LANGUAGES CXX)

find_package(LibMotionPlusPlus REQUIRED)

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

target_link_libraries(motionplusplus PRIVATE LibMotionPlusPlus::libmotionplusplus)

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

install(FILES srv/motionplusplus.service
    DESTINATION $ENV{HOME}/.config/systemd/user
)

