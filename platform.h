#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define VK_USE_PLATFORM_METAL_EXT
#include <vulkan/vulkan.h>

#define ArrayCount(a) sizeof(a) / sizeof(a[0])
#define Halt abort();

#define VK_VALID(v) ((v) == VK_SUCCESS)
#define VK_VALID_HANDLE(v) ((v) != VK_NULL_HANDLE)

#define Pre(a) if(!(a)) Halt
#define Post(a) if(!(a)) Halt
#define Invariant(a) if(!(a)) Halt
#define Implies(a, b) (!(a) || (b))
#define Iff(a, b) ((a) == (b))

typedef uint32_t u32;
typedef int32_t s32;
typedef uint8_t byte;
typedef float f32;
typedef double f64;

typedef struct OSXPlatformWindow OSXPlatformWindow;
// +1 more for min images used for double buffering to avoid implementation stalls
#define VULKAN_IMAGE_COUNT 3		// TODO: This is dumb - we need to query the swapchain count from vulkan
typedef struct VulkanContext
{
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VkSwapchainKHR swapChain;
    VkCommandBuffer drawCmdBuffer;
    VkRenderPass renderPass;
    VkFramebuffer frameBuffers[VULKAN_IMAGE_COUNT];
    VkImage images[VULKAN_IMAGE_COUNT];
    VkImageView imageViews[VULKAN_IMAGE_COUNT];
    VkQueue queue;
    VkFormat format;
    VkSemaphore semaphoreImageAvailable;
    VkSemaphore semaphoreRenderFinished;
    VkFence fenceFrame;
    u32    surfaceWidth;
    u32    surfaceHeight;
    u32    swapChainCount;
} VulkanContext;

void* allocate(size_t size);

// Platform
void platformInitialize();
OSXPlatformWindow* platformCreateWindow(u32 w, u32 h, const char* title, u32 flags);
void platformRun(VulkanContext* context);

// Vulkan
VulkanContext vulkanInitialize(OSXPlatformWindow* platformWindow);
void vulkanDeinitialize(VulkanContext* context);
VulkanContext OSXVulkanInitialize(OSXPlatformWindow* platformWindow);
void VulkanRender(VulkanContext* context);
