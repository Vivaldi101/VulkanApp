#pragma once

#include <stdint.h>
#include <stdbool.h>

#define ArrayCount(a) sizeof(a) / sizeof(a[0])
#define Halt abort();

typedef uint32_t u32;
typedef int32_t s32;
typedef uint8_t byte;
typedef float f32;
typedef double f64;

typedef struct OSXPlatformWindow OSXPlatformWindow;
typedef struct VulkanContext VulkanContext;

// Platform
void platformInitialize();
OSXPlatformWindow* platformCreateWindow(u32 w, u32 h, const char* title, u32 flags);
void platformRun();

// Vulkan
void vulkanInitialize(OSXPlatformWindow* window);
void vulkanDeinitialize(VulkanContext* context);
