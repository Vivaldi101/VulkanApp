#import	<Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import	<UIKit/UIKit.h>

#define VK_USE_PLATFORM_METAL_EXT
#import <vulkan/vulkan.h>

#import "platform.h"
#import "AAPLAppDelegate.h"
#import "AAPLViewController.h"

#if 0

@interface ApplicationDelegate : NSObject<NSApplicationDelegate>
@end

@implementation ApplicationDelegate
-(void)applicationDidFinishLaunching:(NSNotification*)notification
{
	@autoreleasepool
	{
        NSEvent* event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                            location:NSMakePoint(0,0)
                                       modifierFlags:0
                                           timestamp:0
                                        windowNumber:0
                                             context:nil
                                             subtype:0
                                               data1:0
                                               data2:0];

            [NSApp postEvent:event atStart:YES];
	}

    [NSApp stop:nil];
	NSLog(@"App launched!");
}
@end

@interface WindowDelegate : NSObject<NSWindowDelegate>
{
	OSXPlatformWindow* window;
}
@end

@interface ContentView : NSView
{
	OSXPlatformWindow* window;
}
-(instancetype)initWithState:(void*)platformWindow;
@end

@implementation ContentView
-(instancetype)initWithState:(void*)platformWindow
{
	self = [super init];

	if (self != nil)
		window = platformWindow;

	return self;
}
@end

void OSXUpdate()
{
	@autoreleasepool
	{
		for (;;)
		{
            NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                    untilDate:[NSDate distantPast]
                                    inMode:NSDefaultRunLoopMode
                                    dequeue:YES];
            if (!event)
                break;
            [NSApp sendEvent:event];
		}
	}
}

typedef struct OSXPlatformState
{
	OSXPlatformWindow* window;
	ApplicationDelegate* app;
    u32 windowCount;
	BOOL doClose;
} OSXPlatformState;

static OSXPlatformState platform;

@implementation WindowDelegate
-(instancetype)initWithState:(OSXPlatformWindow*)platformWindow
{
	self = [super init];

	if (self != nil)
		window = platformWindow;

	return self;
}

-(BOOL)windowShouldClose:(NSWindow*)sender
{
	platform.doClose = YES;
    free(platform.window);
	return YES;
}
@end

void platformInitialize()
{
	[NSApplication sharedApplication];
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

	platform.app = [[ApplicationDelegate alloc] init];
    platform.doClose = false;

	if (!platform.app)
		abort();

	[NSApp setDelegate:platform.app];
	[NSApp run];
	[NSApp finishLaunching];

	[NSApp activateIgnoringOtherApps:YES];
}

void platformRun(VulkanContext* context)
{
    while (!platform.doClose)
    {
        OSXUpdate();
        VulkanRender(context);
    }
}


#endif

typedef struct OSXPlatformWindow
{
    UIWindow* window;
    CAMetalLayer* layer;
} OSXPlatformWindow;

OSXPlatformWindow* platformCreateWindow(u32 w, u32 h, const char* title, u32 flags)
{
    OSXPlatformWindow* platformWindow = calloc(1, sizeof(OSXPlatformWindow));
    if (!platformWindow)
        abort();

    CGRect frame = {{0,0}, {w,h}};
    if (w == 0 || h == 0)
        frame = [UIScreen mainScreen].bounds;

    // Create the main iOS window
    platformWindow->window = [[UIWindow alloc] initWithFrame:frame];
    if (!platformWindow->window)
        abort();

    // Create the main backing layer for window
    CAMetalLayer *metalLayer = (CAMetalLayer *)platformWindow->window.layer;
    if (!metalLayer)
        abort();

    platformWindow->layer = metalLayer;

#if 0
    //For Metal
    platformWindow->windowDelegate = [[WindowDelegate alloc] init];
    if (!platformWindow->windowDelegate)
        abort();
    platformWindow->view = [[ContentView alloc] init];
    if (!platformWindow->view)
        abort();

    [platformWindow->view setWantsLayer:YES];
    platformWindow->window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0, w,h)
                                               styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskResizable|NSWindowStyleMaskMiniaturizable
                                               backing:NSBackingStoreBuffered
                                               defer:NO];
    if (!platformWindow->window)
        abort();

    // Window properties
    [platformWindow->window setDelegate:platformWindow->windowDelegate];
    [platformWindow->window setContentView:platformWindow->view];
    [platformWindow->window setTitle:@(title)];
    [platformWindow->window makeFirstResponder:platformWindow->view];
    [platformWindow->window makeKeyAndOrderFront:nil];
    [platformWindow->window center];
    [platformWindow->window orderFrontRegardless];

    platformWindow->layer = [CAMetalLayer layer];
    if (!platformWindow->layer)
        abort();

    //[platformWindow->view setLayer:platformWindow->layer];

    //platform.window = platformWindow;
#endif
    return platformWindow;
}

static void VulkanCreateSyncObjects(VulkanContext* context)
{
    Pre(context);
    VkSemaphoreCreateInfo semaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceInfo = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };

    if (!VK_VALID(vkCreateSemaphore(context->logicalDevice, &semaphoreInfo, 0, &context->semaphoreImageAvailable)))
        Post(0);
    if (!VK_VALID(vkCreateSemaphore(context->logicalDevice, &semaphoreInfo, 0, &context->semaphoreRenderFinished)))
        Post(0);
    if (!VK_VALID(vkCreateFence(context->logicalDevice, &fenceInfo, 0, &context->fenceFrame)))
        Post(0);
}

typedef struct QueueFamilyIndices
{
    u32 graphicsFamily;
    u32 presentFamily;
    bool isValid;
} QueueFamilyIndices;

static bool VulkanAreExtensionsSupported(VkPhysicalDevice device)
{
    return true;
}

static QueueFamilyIndices VulkanFindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices = {};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);

    VkQueueFamilyProperties* queueFamilies = allocate(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for (u32 i = 0; i < queueFamilyCount; ++i)
    {
        VkQueueFamilyProperties queueFamily = queueFamilies[i];
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport)
        {
            indices.presentFamily = i;
            indices.isValid = true;
        }
        if (indices.isValid)
            break;
        i++;
    }

    // TODO: Currently just assume that graphics and present queues belong to same family
    Post(indices.graphicsFamily == indices.presentFamily);

    return indices;
}

static bool VulkanIsDeviceCompatible(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    const QueueFamilyIndices result = VulkanFindQueueFamilies(device, surface);
    return result.isValid && VulkanAreExtensionsSupported(device);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    //DebugMessage("Validation layer: %s\n", pCallbackData->pMessage);
    printf("Validation layer: %s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

// Function to dynamically load vkCreateDebugUtilsMessengerEXT
static VkResult vulkanCreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (!func)
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

static void checkExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

    VkExtensionProperties *extensions = malloc(extensionCount * sizeof(VkExtensionProperties));
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

    printf("Available Vulkan instance extensions:\n");
    for (uint32_t i = 0; i < extensionCount; ++i) {
        printf("\t%s\n", extensions[i].extensionName);
    }

    free(extensions);
}

VulkanContext* OSXVulkanInitialize(OSXPlatformWindow* platformWindow)
{
    checkExtensions();
    
    VulkanContext result = {0};
    u32 extensionCount = 0;
    if (!VK_VALID(vkEnumerateInstanceExtensionProperties(0, &extensionCount, 0)))
        Post(0);

    Invariant(extensionCount > 0);
    VkExtensionProperties* extensions = allocate(extensionCount * sizeof(VkExtensionProperties));
    Invariant(extensions);

    if (!VK_VALID(vkEnumerateInstanceExtensionProperties(0, &extensionCount, extensions)))
        Post(0);

    const char** extensionNames = allocate(extensionCount * sizeof(const char*));
    Invariant(extensionNames);
    for (size_t i = 0; i < extensionCount; ++i)
        extensionNames[i] = extensions[i].extensionName;

    //const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanApp";
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "No Engine";
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo instanceInfo = {0};
    const char* instanceExtensions[] = {
        "VK_EXT_debug_report"
        "VK_EXT_debug_utils"
        "VK_KHR_portability_enumeration"
        "VK_LUNARG_direct_driver_loading"
    };
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = 0;
    instanceInfo.enabledExtensionCount = ArrayCount(instanceExtensions);
    instanceInfo.ppEnabledExtensionNames = instanceExtensions;
    instanceInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    
    VkResult createInstanceResult = vkCreateInstance(&instanceInfo, 0, &result.instance);

    if (!VK_VALID(createInstanceResult))
        Post(0);

    VkDebugUtilsMessengerEXT debugMessenger;
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {0};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = vulkanDebugCallback;

    if (!VK_VALID(vulkanCreateDebugUtilsMessengerEXT(result.instance, &debugCreateInfo, 0, &debugMessenger)))
        Post(0);

    // TODO: Compress this into platform specific surface routine

    VkMetalSurfaceCreateInfoEXT metalSurfaceInfo = {0};
    metalSurfaceInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    metalSurfaceInfo.pLayer = platformWindow->layer;

    if (!VK_VALID(vkCreateMetalSurfaceEXT(result.instance, &metalSurfaceInfo, 0, &result.surface)))
        Post(0);

    u32 deviceCount = 0;
    if (!VK_VALID(vkEnumeratePhysicalDevices(result.instance, &deviceCount, 0)))
        Post(0);
    Post(deviceCount > 0);   // Just use the first device for now

    VkPhysicalDevice* devices = allocate(deviceCount * sizeof(VkPhysicalDevice));
    if (!devices)
        Post(0);

    if (!VK_VALID(vkEnumeratePhysicalDevices(result.instance, &deviceCount, devices)))
        Post(0);

    Post(VK_VALID_HANDLE(result.surface));
    for (u32 i = 0; i < deviceCount; ++i)
        if (VulkanIsDeviceCompatible(devices[i], result.surface))
        {
            result.physicalDevice = devices[i];
            break;
        }

    Post(result.physicalDevice != VK_NULL_HANDLE);

    VkDeviceQueueCreateInfo queueInfo = {0};
    const QueueFamilyIndices queueFamilies = VulkanFindQueueFamilies(result.physicalDevice, result.surface);
    Post(queueFamilies.isValid);
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = queueFamilies.graphicsFamily;
    queueInfo.queueCount = 1;
    const float queuePriorities[] = {1.0f};
    queueInfo.pQueuePriorities = queuePriorities;

    extensionCount = 0;
    if (!VK_VALID(vkEnumerateDeviceExtensionProperties(result.physicalDevice, 0, &extensionCount, 0)))
        Post(0);

    Invariant(extensionCount > 0);
    extensions = allocate(extensionCount * sizeof(VkExtensionProperties));
    Invariant(extensions);

    if (!VK_VALID(vkEnumerateDeviceExtensionProperties(result.physicalDevice, 0, &extensionCount, extensions)))
        Post(0);

    extensionNames = allocate(extensionCount * sizeof(const char*));
    Invariant(extensionNames);
    for (size_t i = 0; i < extensionCount; ++i)
        extensionNames[i] = extensions[i].extensionName;

    {
        VkDeviceCreateInfo info =
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pQueueCreateInfos = &queueInfo,
            .queueCreateInfoCount = 1,
            .enabledExtensionCount = extensionCount,
            .ppEnabledExtensionNames = extensionNames,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = 0,
        };

        VkPhysicalDeviceFeatures physicalFeatures = { 0 };
        physicalFeatures.shaderClipDistance = VK_TRUE;
        info.pEnabledFeatures = &physicalFeatures;

        if (!VK_VALID(vkCreateDevice(result.physicalDevice, &info, 0, &result.logicalDevice)))
            Post(0);
    }

    VkSurfaceCapabilitiesKHR surfaceCaps = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(result.physicalDevice, result.surface, &surfaceCaps);

    VkExtent2D surfaceExtent = surfaceCaps.currentExtent;
    result.surfaceWidth = surfaceExtent.width;
    result.surfaceHeight = surfaceExtent.height;

    {
        VkSwapchainCreateInfoKHR info =
        {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = result.surface,
            .minImageCount = VULKAN_IMAGE_COUNT,
            .imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
            .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
            .imageExtent = surfaceExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_FIFO_KHR,
            .clipped = true,
            .oldSwapchain = 0,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &queueFamilies.graphicsFamily,
        };
        if (!VK_VALID(vkCreateSwapchainKHR(result.logicalDevice, &info, 0, &result.swapChain)))
            Post(0);

        result.format = info.imageFormat;
    }

    u32 swapChainCount = 0;
    if (!VK_VALID(vkGetSwapchainImagesKHR(result.logicalDevice, result.swapChain, &swapChainCount, 0)))
        Post(0);

    if (swapChainCount != VULKAN_IMAGE_COUNT)
        Post(0);

    result.swapChainCount = swapChainCount;

    VkImage* swapChainImages = allocate(swapChainCount*sizeof(VkImage));
    if (!swapChainImages)
        Post(0);

    if (!VK_VALID(vkGetSwapchainImagesKHR(result.logicalDevice, result.swapChain, &swapChainCount, swapChainImages)))
        Post(0);

    for (u32 i = 0; i < swapChainCount; ++i)
        result.images[i] = swapChainImages[i];

    VkImageView* imageViews = allocate(swapChainCount*sizeof(VkImageView));
    if (!imageViews)
        Post(0);

    result.swapChainCount = swapChainCount;
    for (u32 i = 0; i < swapChainCount; ++i) {
        VkImageViewCreateInfo info =
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1,
            .image = swapChainImages[i]
        };

        if (!VK_VALID(vkCreateImageView(result.logicalDevice, &info, 0, &imageViews[i])))
            Post(0);
        result.imageViews[i] = imageViews[i];
    }

    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(result.physicalDevice, &queueFamilyCount, 0);
    VkQueueFamilyProperties* queueProperties = allocate(queueFamilyCount*sizeof(VkQueueFamilyProperties));
    if (!queueProperties)
        Post(0);
    Post(queueFamilyCount > 0);
    vkGetPhysicalDeviceQueueFamilyProperties(result.physicalDevice, &queueFamilyCount, queueProperties);

    vkGetDeviceQueue(result.logicalDevice, queueFamilies.presentFamily, 0, &result.queue);

    // TODO: Use the above similar to these
    // TODO: Wrap these
    VkCommandPool commandPool = 0;
    {
        VkCommandPoolCreateInfo info =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .pNext = 0,
            .queueFamilyIndex = queueFamilies.graphicsFamily,
        };
        if (!VK_VALID(vkCreateCommandPool(result.logicalDevice, &info, 0, &commandPool)))
            Post(0);
    }

    VkCommandBuffer drawCmdBuffer = 0;
    {
        VkCommandBufferAllocateInfo info =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        if (!VK_VALID(vkAllocateCommandBuffers(result.logicalDevice, &info, &drawCmdBuffer)))
            Post(0);
        result.drawCmdBuffer = drawCmdBuffer;
    }

    VkRenderPass renderPass = 0;

    VkAttachmentDescription pass[1] = {0};
    pass[0].format = VK_FORMAT_B8G8R8A8_SRGB;
    pass[0].samples = VK_SAMPLE_COUNT_1_BIT;
    pass[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    pass[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    pass[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    pass[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    pass[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    pass[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ref = {0};
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &ref;

    {
        VkRenderPassCreateInfo info =
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = pass,
            .subpassCount = 1,
            .pSubpasses = &subpass,
        };
        if (!VK_VALID(vkCreateRenderPass(result.logicalDevice, &info, 0, &renderPass)))
            Post(0);
        result.renderPass = renderPass;
    }

    VkImageView frameBufferAttachments = 0;
    VkFramebuffer* frameBuffers = allocate(swapChainCount * sizeof(VkFramebuffer));
    if (!frameBuffers)
        Post(0);
    {
        VkFramebufferCreateInfo info =
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &frameBufferAttachments,
            .width = result.surfaceWidth,
            .height = result.surfaceHeight,
            .layers = 1,
            .renderPass = renderPass,
        };
        for (u32 i = 0; i < swapChainCount; ++i) {
            frameBufferAttachments = imageViews[i];
            if (!VK_VALID(vkCreateFramebuffer(result.logicalDevice, &info, 0, &frameBuffers[i])))
                Post(0);
            result.frameBuffers[i] = frameBuffers[i];
        }
    }

    VulkanCreateSyncObjects(&result);

    // post conditions for the context
    Post(result.instance);
    Post(result.surface);
    Post(result.physicalDevice);
    Post(result.logicalDevice);
    Post(result.swapChain);
    Post(result.swapChainCount == VULKAN_IMAGE_COUNT);
    Post(result.renderPass);
    Post(result.frameBuffers);
    Post(result.queue);
    Post(result.drawCmdBuffer);
    Post(result.format);
    Post(result.images);

    Post(result.fenceFrame);
    Post(result.semaphoreImageAvailable);
    Post(result.semaphoreRenderFinished);

    return calloc(1, sizeof(result));
}
