//
//  MetalRenderer.h
//  vulkan_app
//
//  Created by Joni on 16.11.2024.
//

#import <MetalKit/MetalKit.h>

// === MetalRenderer Class ===
@interface MetalRenderer : NSObject <MTKViewDelegate>
- (instancetype)initWithView:(MTKView *)view;
@end
