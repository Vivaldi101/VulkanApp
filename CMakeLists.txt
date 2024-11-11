cmake_minimum_required(VERSION 3.10)
project(vulkan_app)

# Find Vulkan
find_package(Vulkan REQUIRED)

set(sources main.c vulkan_render.c)

# Differentiate between iOS and Mac
if (APPLE)
    list(APPEND sources macos.m)
endif()

# Add your executable
add_executable(vulkan_app ${sources})

# Include Vulkan headers
include_directories(${Vulkan_INCLUDE_DIRS})

# Link against Vulkan and MoltenVK
target_link_libraries(vulkan_app ${Vulkan_LIBRARIES})

# Metal and OSX stuff
target_link_libraries(vulkan_app
        "-framework Metal"
        "-framework MetalKit"
        "-framework AppKit"
        "-framework Foundation"
        "-framework QuartzCore"
        )

set(CMAKE_CXX_FLAGS "-std=c2x -g3 -fsanitize=address")
set(CMAKE_C_FLAGS "-std=c2x -g3 -fsanitize=address")
