cmake_minimum_required(VERSION 3.16)

project(vulkan_ios_app LANGUAGES C CXX)

if(IOS AND CMAKE_VERSION VERSION_LESS 3.20)
    message(FATAL_ERROR "Configuring iOS apps requires a minimum CMake version of 3.20")
elseif(APPLE AND NOT IOS AND CMAKE_VERSION VERSION_LESS 3.17)
    message(FATAL_ERROR "Configuring Xcode for macOS requires a minimum CMake version of 3.17")
endif()

add_library(${PROJECT_NAME})
