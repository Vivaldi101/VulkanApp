#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR

#include <Windows.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>

typedef uint32_t u32;

#ifndef VK_VALID
#define VK_VALID(v) (v) == VK_SUCCESS
#endif

#define ArrayCount(a) sizeof(a) / sizeof(a[0])
#define Halt abort();

#define Pre(a) if(!(a)) Halt
#define Post(a) if(!(a)) Halt
#define Invariant(a) if(!(a)) Halt
#define Implies(a, b) (!(a) || (b))
#define Iff(a, b) ((a) == (b))

// TODO:
// Use the push buffer style macros
static void* Allocate(size_t size)
{
	return _malloca(size);
}

static LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(wnd, msg, wparam, lparam);
}

#define FRAMEBUFFER_COUNT 2		// This is dumb - we need to query the swapchain count from vulkan
typedef struct VulkanContext
{
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VkSwapchainKHR swapChain;
    VkCommandBuffer drawCmdBuffer;
    VkRenderPass renderPass;
    VkFramebuffer frameBuffers[FRAMEBUFFER_COUNT];	// Normal double buffering
    VkQueue queue;
    u32    surfaceWidth;
    u32    surfaceHeight;
    u32    swapChainCount;
} VulkanContext;

// Function to dynamically load vkCreateDebugUtilsMessengerEXT
static VkResult VulkanCreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (!func)
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

static void DebugMessage(const char* format, ...)
{
    char temp[1024];
    va_list args;
    va_start(args, format);
    wvsprintfA(temp, format, args);
    va_end(args);
    OutputDebugStringA(temp);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    DebugMessage("Validation layer: %s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

typedef struct QueueFamilyIndices 
{
    u32 graphicsFamily;
    u32 presentFamily;
    bool isValid;
} QueueFamilyIndices;

static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) 
{
    QueueFamilyIndices indices = {};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);

    VkQueueFamilyProperties* queueFamilies = Allocate(queueFamilyCount * sizeof(VkQueueFamilyProperties));
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

    return indices;
}

static bool VulkanIsDeviceCompatible(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	const QueueFamilyIndices result = findQueueFamilies(device, surface);
    return result.isValid;
}

static VulkanContext VulkanInitContext(HWND windowHandle)
{
    VulkanContext result = {};
    u32 extensionCount = 0;
    if (!VK_VALID(vkEnumerateInstanceExtensionProperties(0, &extensionCount, 0)))
        Post(0);

    Invariant(extensionCount > 0);
    VkExtensionProperties* extensions = Allocate(extensionCount * sizeof(VkExtensionProperties));
    Invariant(extensions);

    if (!VK_VALID(vkEnumerateInstanceExtensionProperties(0, &extensionCount, extensions)))
        Post(0);

    const char** extensionNames = Allocate(extensionCount * sizeof(const char*));
    Invariant(extensionNames);
    for (size_t i = 0; i < extensionCount; ++i)
        extensionNames[i] = extensions[i].extensionName;

    const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanApp";
    appInfo.applicationVersion = VK_API_VERSION_1_0;
    appInfo.engineVersion = VK_API_VERSION_1_0;
    appInfo.pEngineName = "No Engine";
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceInfo = {0};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = ArrayCount(validationLayers);
    instanceInfo.ppEnabledLayerNames = validationLayers;
    instanceInfo.enabledExtensionCount = extensionCount;
    instanceInfo.ppEnabledExtensionNames = extensionNames;

    if (!VK_VALID(vkCreateInstance(&instanceInfo, 0, &result.instance)))
        Post(0);

    VkDebugUtilsMessengerEXT debugMessenger;
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {0};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = VulkanDebugCallback;

    if (!VK_VALID(VulkanCreateDebugUtilsMessengerEXT(result.instance, &debugCreateInfo, 0, &debugMessenger)))
        Post(0);

    bool isWin32Surface = false;
    for (size_t i = 0; i < extensionCount; ++i)
        if (strcmp(extensionNames[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0) {
            isWin32Surface = true;
            break;
        }

    if (!isWin32Surface)
        Post(0);

    PFN_vkCreateWin32SurfaceKHR vkWin32SurfaceFunction = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(result.instance, "vkCreateWin32SurfaceKHR");

    if (!vkWin32SurfaceFunction)
        Post(0);

    VkWin32SurfaceCreateInfoKHR win32SurfaceInfo = {0};
    win32SurfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    win32SurfaceInfo.hinstance = GetModuleHandleA(0);
    win32SurfaceInfo.hwnd = windowHandle;

    vkWin32SurfaceFunction(result.instance, &win32SurfaceInfo, 0, &result.surface);

    u32 deviceCount = 0;
    if (!VK_VALID(vkEnumeratePhysicalDevices(result.instance, &deviceCount, 0)))
        Post(0);
    Post(deviceCount > 0);   // Just use the first device for now

    VkPhysicalDevice* devices = Allocate(deviceCount * sizeof(VkPhysicalDevice));
    if (!devices)
        Post(0);

    if (!VK_VALID(vkEnumeratePhysicalDevices(result.instance, &deviceCount, devices)))
        Post(0);

    for (u32 i = 0; i < deviceCount; ++i)
		if (VulkanIsDeviceCompatible(devices[i], result.surface)) 
        {
            result.physicalDevice = devices[i];
            break;
        }

    Post(result.physicalDevice != VK_NULL_HANDLE);

    VkDeviceQueueCreateInfo queueInfo = {0};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = 0;
    queueInfo.queueCount = 1;
    const float queuePriorities[] = {1.0f};
    queueInfo.pQueuePriorities = queuePriorities;

    extensionCount = 0;
    if (!VK_VALID(vkEnumerateDeviceExtensionProperties(result.physicalDevice, 0, &extensionCount, 0)))
        Post(0);

    Invariant(extensionCount > 0);
    extensions = Allocate(extensionCount * sizeof(VkExtensionProperties));
    Invariant(extensions);

    if (!VK_VALID(vkEnumerateDeviceExtensionProperties(result.physicalDevice, 0, &extensionCount, extensions)))
        Post(0);

    extensionNames = Allocate(extensionCount * sizeof(const char*));
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
			.minImageCount = 2,
			.imageFormat = VK_FORMAT_B8G8R8A8_UNORM,
			.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
			.imageExtent = surfaceExtent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = VK_PRESENT_MODE_FIFO_KHR,
			.clipped = true,
			.oldSwapchain = 0,
        };
        if (!VK_VALID(vkCreateSwapchainKHR(result.logicalDevice, &info, 0, &result.swapChain)))
            Post(0);
    }

    u32 swapChainCount = 0;
	if (!VK_VALID(vkGetSwapchainImagesKHR(result.logicalDevice, result.swapChain, &swapChainCount, 0)))
        Post(0);

	if (swapChainCount != FRAMEBUFFER_COUNT)
		Post(0);

    VkImage* swapChainImages = Allocate(swapChainCount*sizeof(VkImage));
    if (!swapChainImages)
        Post(0);

	if (!VK_VALID(vkGetSwapchainImagesKHR(result.logicalDevice, result.swapChain, &swapChainCount, swapChainImages)))
        Post(0);

    VkImageView* imageViews = Allocate(swapChainCount*sizeof(VkImageView));
    if (!imageViews)
        Post(0);

	result.swapChainCount = swapChainCount;
    for (u32 i = 0; i < swapChainCount; ++i) {
        VkImageViewCreateInfo info = 
        {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_B8G8R8A8_UNORM,
            .components.r = VK_COMPONENT_SWIZZLE_R,
            .components.g = VK_COMPONENT_SWIZZLE_G,
            .components.b = VK_COMPONENT_SWIZZLE_B,
            .components.a = VK_COMPONENT_SWIZZLE_A,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1,
            .image = swapChainImages[i]
        };

        if (!VK_VALID(vkCreateImageView(result.logicalDevice, &info, 0, &imageViews[i])))
            Post(0);
    }

    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(result.physicalDevice, &queueFamilyCount, 0);
    VkQueueFamilyProperties* queueProperties = Allocate(queueFamilyCount*sizeof(VkQueueFamilyProperties));
    if (!queueProperties)
        Post(0);
    Post(queueFamilyCount > 0);
    vkGetPhysicalDeviceQueueFamilyProperties(result.physicalDevice, &queueFamilyCount, queueProperties);

    VkQueue presentQueue = 0;
    vkGetDeviceQueue(result.logicalDevice, 0, 0, &presentQueue);
    result.queue = presentQueue;

	// TODO: Use the above similar to these
	// TODO: Wrap these
    VkCommandPool commandPool = 0;
    {
        VkCommandPoolCreateInfo info =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = 0,
            .queueFamilyIndex = 0,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
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
    pass[0].format = VK_FORMAT_B8G8R8A8_UNORM;
    pass[0].samples = VK_SAMPLE_COUNT_1_BIT;
    pass[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    pass[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    pass[0].stencilLoadOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    pass[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    pass[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    pass[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
    VkFramebuffer* frameBuffers = Allocate(swapChainCount * sizeof(VkFramebuffer));
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

    // post conditions for the context
    Post(result.instance);
    Post(result.surface);
    Post(result.physicalDevice);
    Post(result.logicalDevice);
    Post(result.swapChain);
    Post(result.swapChainCount > 0);
    Post(result.renderPass);
    Post(result.frameBuffers);
    Post(result.queue);
    Post(result.drawCmdBuffer);

    return result;
}

static void VulkanRender(VulkanContext* context)
{
    Pre(context);
    u32 nextImageIndex = 0;

    if (!VK_VALID(vkAcquireNextImageKHR(context->logicalDevice, context->swapChain, UINT64_MAX, VK_NULL_HANDLE, VK_NULL_HANDLE, &nextImageIndex)))
        Post(0);
    {
        VkCommandBufferBeginInfo info =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        if (!VK_VALID(vkBeginCommandBuffer(context->drawCmdBuffer, &info)))
            Post(0);
    }
    {
        // temporary float that oscilates between 0 and 1
        // to gradually change the color on the screen
        static float aa = 0.0f;
        // slowly increment
        aa += 0.001f;
        // when value reaches 1.0 reset to 0
        if (aa >= 1.0) aa = 0;
        // activate render pass:
        // clear color (r,g,b,a)
        VkClearValue clearValue[] = 
        { 
            { 1.0f, aa, 1.0f, 1.0f },	// color
			{ 1.0, 0.0 }				// depth stencil
        }; 

        // define render pass structure
        VkRenderPassBeginInfo info = 
        {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = context->renderPass,
            .framebuffer = context->frameBuffers[nextImageIndex],
        };
        VkOffset2D a = { 0, 0 };
        VkExtent2D b = { context->surfaceWidth, context->surfaceHeight };
        VkRect2D c = { a,b };
        info.renderArea = c;
        info.clearValueCount = 2;
        info.pClearValues = clearValue;

		vkCmdBeginRenderPass(context->drawCmdBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        {
			// draw cmds
        }
        vkCmdEndRenderPass(context->drawCmdBuffer);
    }

    if (!VK_VALID(vkEndCommandBuffer(context->drawCmdBuffer)))
        Post(0);

    // present:
    // create a fence to inform us when the GPU
    // has finished processing our commands
	// setup the type of fence
    VkFence renderFence;
    {
        VkFenceCreateInfo info =
        {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        };
        if (!VK_VALID(vkCreateFence(context->logicalDevice, &info, 0, &renderFence)))
            Post(0);
    }
    {
        VkSubmitInfo info =
        {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = VK_NULL_HANDLE,
            .pWaitDstStageMask = 0,
            .commandBufferCount = 1,
            .pCommandBuffers = &context->drawCmdBuffer,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = VK_NULL_HANDLE,
        };
        if (!VK_VALID(vkQueueSubmit(context->queue, 1, &info, renderFence)))
			Post(0);
    }
    // wait until the GPU has finished processing the commands
    if (!VK_VALID(vkWaitForFences(context->logicalDevice, 1, &renderFence, VK_TRUE, UINT64_MAX)))
		Post(0);
    vkDestroyFence(context->logicalDevice, renderFence, 0);

    // present the image on the screen (flip the swap-chain image)
    {
        VkPresentInfoKHR info =
        {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = NULL,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = VK_NULL_HANDLE,
            .swapchainCount = 1,
            .pSwapchains = &context->swapChain,
            .pImageIndices = &nextImageIndex,
            .pResults = NULL,
        };
        if (!VK_VALID(vkQueuePresentKHR(context->queue, &info)))
			Post(0);
    }
}

static void VulkanReset(VulkanContext* context)
{
    Pre(context);
    Pre(context->instance);
    vkDestroyInstance(context->instance, 0);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmdline, int cmdshow)
{
    // register window class to have custom WindowProc callback
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"opengl_window_class";
    ATOM atom = RegisterClassExW(&wc);
    assert(atom && "Failed to register window class");

    // window properties - width, height and style
    int width = CW_USEDEFAULT;
    int height = CW_USEDEFAULT;
    DWORD exstyle = WS_EX_APPWINDOW;
    DWORD style = WS_OVERLAPPEDWINDOW;

    // uncomment in case you want fixed size window
    //style &= ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    RECT rect = { 0, 0, 800, 600 };
    AdjustWindowRectEx(&rect, style, FALSE, exstyle);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    // create window
    HWND window = CreateWindowExW(
        exstyle, wc.lpszClassName, L"Vulkan Window", style,
        0, 0, width, height,
        NULL, NULL, wc.hInstance, NULL);
    assert(window && "Failed to create window");

    RECT clientRectangle = {0};
    GetClientRect(window, &clientRectangle);

    width = clientRectangle.right - clientRectangle.left;
    height = clientRectangle.bottom - clientRectangle.top;

    VulkanContext context = VulkanInitContext(window);

    // show the window
    ShowWindow(window, SW_SHOWDEFAULT);

    // main render loop
    for (;;)
    {
        // process all incoming Windows messages
        MSG msg;
        if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        VulkanRender(&context);
    }

    VulkanReset(&context);

    return 0;
}