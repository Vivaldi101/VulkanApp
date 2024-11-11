#include "platform.h"
#include "stdio.h"

int main()
{
	platformInitialize();
    OSXPlatformWindow* window = platformCreateWindow(800, 600, "Hello Apple", 0);
    VulkanContext context = vulkanInitialize(window);
	platformRun(&context);

	return 0;
}
