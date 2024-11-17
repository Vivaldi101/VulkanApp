//
//  MetalRenderer.mm
//  vulkan_app
//
//  Created by Joni on 16.11.2024.
//

#import "MetalRenderer.h"

@implementation MetalRenderer {
    id<MTLDevice> _device;
    id<MTLCommandQueue> _commandQueue;
}

- (instancetype)initWithView:(MTKView *)view {
    self = [super init];
    if (self) {
        _device = view.device;
        _commandQueue = [_device newCommandQueue];
    }
    return self;
}

// Called whenever the drawable size changes
- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
    int foo = 42;
    int foo1 = 42;
    int foo2 = 42;
    // Handle resizing if necessary
}

// Called every frame
- (void)drawInMTKView:(MTKView *)view {
    @autoreleasepool {
        id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
        MTLRenderPassDescriptor *passDescriptor = view.currentRenderPassDescriptor;

        if (passDescriptor == nil) {
            return;
        }
        
        id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
        [encoder endEncoding];
        
        [commandBuffer presentDrawable:view.currentDrawable];

        [commandBuffer commit];
    }
}

@end
