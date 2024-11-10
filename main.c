#include "platform.h"
#include "stdio.h"

int main()
{
	initialize();
    createWindow(800, 600, "Hello Apple", 0);
	run();

	return 0;
}
