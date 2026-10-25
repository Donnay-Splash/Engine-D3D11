#import "MetalDevice.h"

#import "MetalFrame.h"

#include <Core/Math.h>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <vector>

namespace Engine {
namespace {

struct RenderUniforms
{
    Maths::Matrix4 model;
    Maths::Matrix4 viewProjection;
    Maths::Vector4 diffuseColor;
};

static_assert(std::is_standard_layout<Maths::Vector3>::value, "Metal float3 data must be standard layout.");
static_assert(sizeof(Maths::Vector3) == sizeof(float) * 3, "Metal float3 data must be tightly packed.");
static_assert(std::is_standard_layout<RenderUniforms>::value, "Metal uniforms must be standard layout.");
static_assert(offsetof(RenderUniforms, viewProjection) == sizeof(float) * 16,
    "Metal view-projection uniform offset must match MSL.");
static_assert(offsetof(RenderUniforms, diffuseColor) == sizeof(float) * 32,
    "Metal material uniform offset must match MSL.");
static_assert(sizeof(RenderUniforms) == sizeof(float) * 36,
    "Metal uniform size must match the shader structure.");

class MetalDevice final : public Device
{
public:
    MetalDevice
()
        : device_(nil),
          commandQueue_(nil),
          pipelineState_(nil),
          depthState_(nil),
          depthTexture_(nil),
          view_(nil),
          size_({0, 0})
    {
    }

    bool Initialize(
        void *surface,
        Size size,
        const ErrorHandler &errorHandler,
        Error &error) override
    {
        errorHandler_ = errorHandler;
        MTKView *view = (__bridge MTKView *)surface;
        if (view == nil) {
            error.message = "The Metal renderer received no render surface.";
            return false;
        }

        size_ = size;
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
            error.message = libraryError.localizedDescription.UTF8String
                ?: "Failed to load Metal shaders.";
            return false;
        }

        id<MTLFunction> vertexFunction = [library newFunctionWithName:@"mesh_vertex"];
        id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"mesh_fragment"];
        if (vertexFunction == nil || fragmentFunction == nil) {
            error.message = "The Metal shader library is missing a mesh shader function.";
            return false;
        }

        return Resize(size, error);
    }

    bool Resize(Size size, Error &error) override
    {
        size_ = size;
        if (size.width == 0 || size.height == 0) {
            depthTexture_ = nil;
            return true;
        }

        MTLTextureDescriptor *descriptor = [MTLTextureDescriptor
            texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                          width:size.width
                                         height:size.height
                                      mipmapped:NO];
        descriptor.storageMode = MTLStorageModePrivate;
        descriptor.usage = MTLTextureUsageRenderTarget;
        id<MTLTexture> texture = [device_ newTextureWithDescriptor:descriptor];
        if (texture == nil) {
            error.message = "Failed to create the Metal depth texture for the resized surface.";
            return false;
        }
        depthTexture_ = texture;
        return true;
    }

    void Shutdown() noexcept override
    {
        view_.device = nil;
        depthTexture_ = nil;
        depthState_ = nil;
        pipelineState_ = nil;
        commandQueue_ = nil;
        device_ = nil;
        size_ = {0, 0};
        errorHandler_ = ErrorHandler();
    }

private:
    __strong id<MTLDevice> device_;
    __strong id<MTLCommandQueue> commandQueue_;
    __strong id<MTLRenderPipelineState> pipelineState_;
    __strong id<MTLDepthStencilState> depthState_;
    __strong id<MTLTexture> depthTexture_;
    __weak MTKView *view_;

    Size size_;
    ErrorHandler errorHandler_;
};

} // namespace

std::unique_ptr<Device> CreateMetalDevice()
{
    return std::unique_ptr<Device>(new MetalDevice
());
}

} // namespace Engine
