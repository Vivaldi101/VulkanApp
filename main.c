
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR

#include <Windows.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

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

typedef struct VulkanContext
{
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VkSwapchainKHR swapChain;
    uint32_t    surfaceWidth;
    uint32_t    surfaceHeight;
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

static VulkanContext VulkanInitContext(HWND windowHandle)
{
    VulkanContext result = {0};
    uint32_t extensionCount = 0;
    if (!VK_VALID(vkEnumerateInstanceExtensionProperties(0, &extensionCount, 0)))
        Post(0);

    Invariant(extensionCount > 0);
    VkExtensionProperties* extensions = _malloca(extensionCount * sizeof(VkExtensionProperties));
    Invariant(extensions);

    if (!VK_VALID(vkEnumerateInstanceExtensionProperties(0, &extensionCount, extensions)))
        Post(0);

    const char** extensionNames = _malloca(extensionCount * sizeof(const char*));
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

    uint32_t deviceCount = 0;
    if (!VK_VALID(vkEnumeratePhysicalDevices(result.instance, &deviceCount, 0)))
        Post(0);
    Post(deviceCount > 0);   // Just use the first device for now

    VkPhysicalDevice* devices = _malloca(deviceCount * sizeof(VkPhysicalDevice));
    if (!devices)
        Post(0);

    if (!VK_VALID(vkEnumeratePhysicalDevices(result.instance, &deviceCount, devices)))
        Post(0);

    result.physicalDevice = devices[0];

    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties = {0};
    vkGetPhysicalDeviceMemoryProperties(result.physicalDevice, &physicalDeviceMemoryProperties);

    VkDeviceQueueCreateInfo queueInfo = {0};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = 0;
    queueInfo.queueCount = 1;
    const float queuePriorities[] = {1.0f};
    queueInfo.pQueuePriorities = queuePriorities;

    VkDeviceCreateInfo deviceCreateInfo = {0};
    extensionCount = 0;
    if (!VK_VALID(vkEnumerateDeviceExtensionProperties(result.physicalDevice, 0, &extensionCount, 0)))
        Post(0);

    Invariant(extensionCount > 0);
    extensions = _malloca(extensionCount * sizeof(VkExtensionProperties));
    Invariant(extensions);

    if (!VK_VALID(vkEnumerateDeviceExtensionProperties(result.physicalDevice, 0, &extensionCount, extensions)))
        Post(0);

    extensionNames = _malloca(extensionCount * sizeof(const char*));
    Invariant(extensionNames);
    for (size_t i = 0; i < extensionCount; ++i)
        extensionNames[i] = extensions[i].extensionName;

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.enabledExtensionCount = extensionCount;
    deviceCreateInfo.ppEnabledExtensionNames = extensionNames;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = 0;

    VkPhysicalDeviceFeatures physicalFeatures = {0};
    physicalFeatures.shaderClipDistance = VK_TRUE;
    deviceCreateInfo.pEnabledFeatures = &physicalFeatures;

    if (!VK_VALID(vkCreateDevice(result.physicalDevice, &deviceCreateInfo, 0, &result.logicalDevice)))
        Post(0);

    VkSurfaceCapabilitiesKHR surfaceCaps = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(result.physicalDevice, result.surface, &surfaceCaps);

    VkExtent2D surfaceExtent = surfaceCaps.currentExtent;
    result.surfaceWidth = surfaceExtent.width;
    result.surfaceHeight = surfaceExtent.height;

    VkSwapchainCreateInfoKHR swapChainInfo = {0};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.surface = result.surface;
    swapChainInfo.minImageCount = 2;
    swapChainInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    swapChainInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    swapChainInfo.imageExtent = surfaceExtent;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapChainInfo.clipped = true;
    swapChainInfo.oldSwapchain = 0;

    if (!VK_VALID(vkCreateSwapchainKHR(result.logicalDevice, &swapChainInfo, 0, &result.swapChain)))
        Post(0);

    uint32_t swapChainCount = 0;
	if (!VK_VALID(vkGetSwapchainImagesKHR(result.logicalDevice, result.swapChain, &swapChainCount, 0)))
        Post(0);

    VkImage* swapChainImages = _malloca(swapChainCount*sizeof(VkImage));

	if (!VK_VALID(vkGetSwapchainImagesKHR(result.logicalDevice, result.swapChain, &swapChainCount, swapChainImages)))
        Post(0);

    // post conditions for the context
    Post(result.instance);
    Post(result.surface);
    Post(result.physicalDevice);
    Post(result.logicalDevice);
    Post(result.swapChain);

    return result;
}

static void VulkanUpdate(VulkanContext* context)
{
    Pre(context);
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
        VulkanUpdate(&context);
    }

    VulkanReset(&context);

    return 0;
}