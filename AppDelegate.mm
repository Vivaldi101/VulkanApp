//
//  AppDelegate.mm
//  vulkan_app
//
//  Created by Joni on 16.11.2024.
//

#import "AppDelegate.h"
#import "MetalRenderer.h"

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    CGRect screenBounds = [[UIScreen mainScreen] bounds];
    self.window = [[UIWindow alloc] initWithFrame:screenBounds];
    
    // Create an MTKView and set it as the root view
    MTKView *mtkView = [[MTKView alloc] initWithFrame:screenBounds];
    mtkView.device = MTLCreateSystemDefaultDevice();
    
    if (!mtkView.device) {
        NSLog(@"Metal is not supported on this device");
        return NO;
    }
    
    // Set up the renderer and assign it as the view's delegate
    MetalRenderer *renderer = [[MetalRenderer alloc] initWithView:mtkView];
    mtkView.delegate = renderer;
    
    // Configure MTKView settings
    mtkView.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    mtkView.clearColor = MTLClearColorMake(1.0, 1.0, 0.0, 1.0);
    mtkView.preferredFramesPerSecond = 60;
    mtkView.paused = NO;

    // Set the MTKView as the root view of the window
    UIViewController *viewController = [[UIViewController alloc] init];
    viewController.view = mtkView;
    self.window.rootViewController = viewController;
    
    [self.window makeKeyAndVisible];
    return YES;
}

@end
