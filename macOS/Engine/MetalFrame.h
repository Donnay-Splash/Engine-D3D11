#ifndef ENGINE_MACOS_METAL_FRAME_H
#define ENGINE_MACOS_METAL_FRAME_H

#import <MetalKit/MetalKit.h>

struct MetalFrame
{
    __strong id<CAMetalDrawable> drawable;
    __strong MTLRenderPassDescriptor *renderPass;
};

#endif
