/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Implementation of the iOS & tvOS application delegate.
*/

#import "AAPLAppDelegate.h"
#import "AAPLViewController.h"
#import "platform.h"

@implementation AAPLAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Create the main Vulkan platform window
    OSXPlatformWindow* platformWindow = platformCreateWindow(0, 0, "Hello iOS", 0);
    VulkanContext* context = vulkanInitialize(platformWindow);

    // Set VulkanViewController as the root windows view controller
    VulkanViewController *rootView = [[VulkanViewController alloc] init];
    // Get the platform window
    self.window = [VulkanViewController platformGetWindow:platformWindow];
    // Set the root view controller
    self.window.rootViewController = rootView;

    // Display it
    [self.window makeKeyAndVisible];
    return YES;
}

@end
