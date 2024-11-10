cmake_minimum_required(VERSION 3.10)
project(vulkan_app)

# Find Vulkan
find_package(Vulkan REQUIRED)

set(sources main.c vulkan_render.c)
if (APPLE)
    list(APPEND sources macos.m)
elseif()
    list(APPEND sources ios.m)  # TODO
endif()

# Add your executable
add_executable(vulkan_app ${sources})

# Include Vulkan headers
include_directories(${Vulkan_INCLUDE_DIRS})

# Link against Vulkan and MoltenVK
target_link_libraries(vulkan_app ${Vulkan_LIBRARIES})

# Link against MacOS Cocoa and frameworks
target_link_libraries(vulkan_app "-framework Cocoa")
target_link_libraries(vulkan_app "-framework CoreFoundation")

set(CMAKE_CXX_FLAGS "-g3 -fsanitize=address")
set(CMAKE_C_FLAGS "-g3 -fsanitize=address")
