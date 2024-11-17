/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Header for the cross-platform view controller.
*/
#define TARGET_IOS

#if defined(TARGET_IOS) || defined(TARGET_TVOS)
#import <UIKit/UIKit.h>
#define PlatformViewController UIViewController
#else
#import <AppKit/AppKit.h>
#define PlatformViewController NSViewController
#endif

#import <MetalKit/MetalKit.h>

#import "AAPLRenderer.h"

@interface AAPLViewController : PlatformViewController

@end
