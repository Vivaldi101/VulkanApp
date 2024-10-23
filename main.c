
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
} VulkanContext;

static VulkanContext VulkanInitContext(HWND windowHandle)
{
    VulkanContext result = {0};
    Pre(!result.instance);
    Pre(!result.surface);
    Pre(!result.physicalDevice);
    Pre(!result.logicalDevice);

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
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = 0;
    instanceInfo.enabledExtensionCount = extensionCount;
    instanceInfo.ppEnabledExtensionNames = extensionNames;

    if (!VK_VALID(vkCreateInstance(&instanceInfo, 0, &result.instance)))
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

    // post conditions for the context
    Post(result.instance);
    Post(result.surface);
    Post(result.physicalDevice);
    Post(result.logicalDevice);

    return result;
}

static void VulkanUpdate()
{
}

static void VulkanReset(VulkanContext* context)
{
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
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    VulkanReset(&context);

    return 0;
}