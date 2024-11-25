/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Implementation of the cross-platform view controller.
*/

#import "AAPLViewController.h"
#import "AAPLRenderer.h"
#import "platform.h"

#import "ios.m" // Maybe invert this and include the view controller in ios.m

@implementation VulkanViewController
{
}
@synthesize vulkanView;
#if 0
    // For Metal
    MTKView *_view;
    AAPLRenderer *_renderer;
#endif

+ (UIWindow*)platformGetWindow:(OSXPlatformWindow*)platformWindow
{
    return platformWindow->window;
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
}

- (void)renderFrame
{
    [self.vulkanView renderFrame];
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Sanity check
    if (self.platformWindow)
    {
        vulkanView = [[VulkanView alloc] initWithFrame:self.view.bounds];
        vulkanView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;

        [self.view addSubview:vulkanView];
    }
    // Set up a display link to call renderFrame at the screen refresh rate (typically 60Hz)
    self.displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(renderFrame)];
    [self.displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];

#if 0
    // For Metal
    _view = (MTKView *)self.view;
    _view.enableSetNeedsDisplay = YES;
    _view.device = MTLCreateSystemDefaultDevice();
    _view.clearColor = MTLClearColorMake(1.0, 0.0, 1.0, 1.0);
    _view.preferredFramesPerSecond = 60;
    _view.paused = NO;

    _renderer = [[AAPLRenderer alloc] initWithMetalKitView:_view];

    if(!_renderer)
    {
        NSLog(@"Renderer initialization failed");
        return;
    }

    // Initialize the renderer with the view size.
    [_renderer mtkView:_view drawableSizeWillChange:_view.drawableSize];

    _view.delegate = _renderer;
#endif
}

@end
