#import "MetalRenderer.h"

#import "MetalFrame.h"

#include <memory>

namespace engine {
namespace {

class MetalRenderer final : public detail::Renderer
{
public:
    MetalRenderer() : device_(nil), commandQueue_(nil), pipelineState_(nil), view_(nil)
    {
    }

    bool initialize(
        void *surface,
        Size size,
        const ErrorHandler &errorHandler,
        Error &error) override
    {
        (void)size;
        errorHandler_ = errorHandler;
        MTKView *view = (__bridge MTKView *)surface;
        if (view == nil) {
            error.message = "The Metal renderer received no render surface.";
            return false;
        }

        view_ = view;
        device_ = MTLCreateSystemDefaultDevice();
        if (device_ == nil) {
            error.message = "Engine requires a Mac with a usable Metal device.";
            return false;
        }
        view.device = device_;
        view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        view.clearColor = MTLClearColorMake(0.08, 0.09, 0.12, 1.0);

        commandQueue_ = [device_ newCommandQueue];
        if (commandQueue_ == nil) {
            error.message = "Failed to create a Metal command queue.";
            return false;
        }

        NSURL *libraryURL = [NSBundle.mainBundle URLForResource:@"default" withExtension:@"metallib"];
        if (libraryURL == nil) {
            error.message = "The bundled Metal shader library is missing.";
            return false;
        }
        NSError *libraryError = nil;
        id<MTLLibrary> library = [device_ newLibraryWithURL:libraryURL error:&libraryError];
        if (library == nil) {
            error.message = libraryError.localizedDescription.UTF8String ?: "Failed to load Metal shaders.";
            return false;
        }

        id<MTLFunction> vertexFunction = [library newFunctionWithName:@"triangle_vertex"];
        id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"triangle_fragment"];
        if (vertexFunction == nil || fragmentFunction == nil) {
            error.message = "The Metal shader library is missing a triangle shader function.";
            return false;
        }

        MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDescriptor.vertexFunction = vertexFunction;
        pipelineDescriptor.fragmentFunction = fragmentFunction;
        pipelineDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
        NSError *pipelineError = nil;
        pipelineState_ = [device_ newRenderPipelineStateWithDescriptor:pipelineDescriptor
                                                                 error:&pipelineError];
        if (pipelineState_ == nil) {
            error.message = pipelineError.localizedDescription.UTF8String
                ?: "Failed to create the Metal render pipeline.";
            return false;
        }
        return true;
    }

    bool render(void *opaqueFrame, Error &error) override
    {
        MetalFrame *frame = static_cast<MetalFrame *>(opaqueFrame);
        if (frame == nullptr || frame->drawable == nil || frame->renderPass == nil) {
            error.message = "The render surface supplied an invalid Metal frame.";
            return false;
        }

        id<MTLCommandBuffer> commandBuffer = [commandQueue_ commandBuffer];
        if (commandBuffer == nil) {
            error.message = "Failed to create a Metal command buffer.";
            return false;
        }
        id<MTLRenderCommandEncoder> encoder =
            [commandBuffer renderCommandEncoderWithDescriptor:frame->renderPass];
        if (encoder == nil) {
            error.message = "Failed to create a Metal render command encoder.";
            return false;
        }

        [encoder setRenderPipelineState:pipelineState_];
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
        [encoder endEncoding];
        [commandBuffer presentDrawable:frame->drawable];

        const ErrorHandler errorHandler = errorHandler_;
        [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> completedBuffer) {
            if (completedBuffer.status == MTLCommandBufferStatusError && errorHandler) {
                Error completionError;
                completionError.message = completedBuffer.error.localizedDescription.UTF8String
                    ?: "Metal failed while presenting a frame.";
                errorHandler(completionError);
            }
        }];
        [commandBuffer commit];
        return true;
    }

    bool resize(Size size, Error &error) override
    {
        (void)size;
        (void)error;
        return true;
    }

    void shutdown() noexcept override
    {
        view_.device = nil;
        pipelineState_ = nil;
        commandQueue_ = nil;
        device_ = nil;
        view_ = nil;
        errorHandler_ = ErrorHandler();
    }

private:
    __strong id<MTLDevice> device_;
    __strong id<MTLCommandQueue> commandQueue_;
    __strong id<MTLRenderPipelineState> pipelineState_;
    __weak MTKView *view_;
    ErrorHandler errorHandler_;
};

} // namespace

std::unique_ptr<detail::Renderer> createMetalRenderer()
{
    return std::unique_ptr<detail::Renderer>(new MetalRenderer());
}

} // namespace engine
