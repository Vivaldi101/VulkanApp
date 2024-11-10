#import	<Foundation/Foundation.h>
#import	<Cocoa/Cocoa.h>
#import	<QuartzCore/QuartzCore.h>

#import "platform.h"

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

typedef struct OSXPlatformWindow
{
	WindowDelegate* windowDelegate;
	ContentView* view;
	NSWindow* window;
	//CAMetalLayer* layer;
} OSXPlatformWindow;

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

void createWindow(u32 w, u32 h, const char* title, u32 flags)
{
	OSXPlatformWindow* platformWindow = malloc(sizeof(OSXPlatformWindow));
	if (!platformWindow)
		abort();

	platformWindow->windowDelegate = [[WindowDelegate alloc] initWithState:platformWindow];
    platformWindow->view = [[ContentView alloc] initWithState:platformWindow];
	platformWindow->window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0, w,h)
											   styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskResizable|NSWindowStyleMaskMiniaturizable
											   backing:NSBackingStoreBuffered
											   defer:NO];

	// Window properties
	[platformWindow->window setDelegate:platformWindow->windowDelegate];
	[platformWindow->window setContentView:platformWindow->view];
	[platformWindow->window setTitle:@(title)];
	[platformWindow->window makeKeyAndOrderFront:nil];

    platform.window = platformWindow;
}

void initialize()
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
}

void run()
{
    while (!platform.doClose)
        OSXUpdate();
}
