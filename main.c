#include "platform.h"
#include "stdio.h"

int main()
{
	platformInitialize();
    OSXPlatformWindow* window = platformCreateWindow(800, 600, "Hello Apple", 0);
    vulkanInitialize(window);
	platformRun();

	return 0;
}
