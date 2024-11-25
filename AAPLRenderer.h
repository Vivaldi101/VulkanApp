/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Header for a platform independent renderer class, which performs Metal setup and per frame rendering.
*/

#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>
#import <MoltenVK/mvk_vulkan.h>
#import "platform.h"

#if 0
// For Metal
@interface AAPLRenderer : NSObject<MTKViewDelegate>

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView;

@end
#endif

@interface VulkanView : UIView
@property (nonatomic) VulkanContext* context;  // Strong property, ARC retains VulkanContext
- (void)renderFrame;

@end
