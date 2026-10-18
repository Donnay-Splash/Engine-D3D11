#import "MetalRenderer.h"

#import "MetalFrame.h"

#include <cstddef>
#include <memory>
#include <type_traits>
#include <vector>

namespace engine {
namespace {

struct RenderUniforms
{
    core::Matrix4 model;
    core::Matrix4 viewProjection;
    core::Vector4 diffuseColor;
};

static_assert(std::is_standard_layout<core::Vector3>::value, "Metal float3 data must be standard layout.");
static_assert(sizeof(core::Vector3) == sizeof(float) * 3, "Metal float3 data must be tightly packed.");
static_assert(std::is_standard_layout<RenderUniforms>::value, "Metal uniforms must be standard layout.");
static_assert(offsetof(RenderUniforms, viewProjection) == sizeof(float) * 16,
    "Metal view-projection uniform offset must match MSL.");
static_assert(offsetof(RenderUniforms, diffuseColor) == sizeof(float) * 32,
    "Metal material uniform offset must match MSL.");
static_assert(sizeof(RenderUniforms) == sizeof(float) * 36,
    "Metal uniform size must match the shader structure.");

class MetalRenderer final : public detail::Renderer
{
public:
    MetalRenderer()
        : device_(nil),
          commandQueue_(nil),
          pipelineState_(nil),
          depthState_(nil),
          vertexBuffer_(nil),
          normalBuffer_(nil),
          indexBuffer_(nil),
          depthTexture_(nil),
          view_(nil),
          scene_(nullptr),
          camera_(nullptr),
          nodeIndex_(core::InvalidIndex),
          indexCount_(0),
          vertexCount_(0),
          size_({0, 0})
    {
    }

    bool initialize(
        void *surface,
        Size size,
        const core::SceneData &scene,
        const core::CameraData &camera,
        const ErrorHandler &errorHandler,
        Error &error) override
    {
        errorHandler_ = errorHandler;
        MTKView *view = (__bridge MTKView *)surface;
        if (view == nil) {
            error.message = "The Metal renderer received no render surface.";
            return false;
        }

        std::string validationError;
        if (!scene.validate(validationError)) {
            error.message = validationError;
            return false;
        }

        nodeIndex_ = core::InvalidIndex;
        for (std::size_t index = 0; index < scene.nodes.size(); ++index) {
            if (scene.nodes[index].meshIndex == core::InvalidIndex) {
                continue;
            }
            if (nodeIndex_ != core::InvalidIndex) {
                error.message = "The first Metal scene renderer supports one scene object.";
                return false;
            }
            nodeIndex_ = static_cast<std::uint32_t>(index);
        }
        if (nodeIndex_ == core::InvalidIndex) {
            error.message = "The engine scene contains no mesh object to render.";
            return false;
        }

        const core::SceneNodeData &node = scene.nodes[nodeIndex_];
        const core::MeshData &mesh = scene.meshes[node.meshIndex];
        if (mesh.positions.empty()) {
            error.message = "The engine scene mesh contains no vertices.";
            return false;
        }

        view_ = view;
        scene_ = &scene;
        camera_ = &camera;
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

        MTLVertexDescriptor *vertexDescriptor = [[MTLVertexDescriptor alloc] init];
        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat3;
        vertexDescriptor.attributes[0].offset = 0;
        vertexDescriptor.attributes[0].bufferIndex = 0;
        vertexDescriptor.attributes[1].format = MTLVertexFormatFloat3;
        vertexDescriptor.attributes[1].offset = 0;
        vertexDescriptor.attributes[1].bufferIndex = 1;
        vertexDescriptor.layouts[0].stride = sizeof(core::Vector3);
        vertexDescriptor.layouts[1].stride = sizeof(core::Vector3);

        MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDescriptor.vertexFunction = vertexFunction;
        pipelineDescriptor.fragmentFunction = fragmentFunction;
        pipelineDescriptor.vertexDescriptor = vertexDescriptor;
        pipelineDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
        pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
        NSError *pipelineError = nil;
        pipelineState_ = [device_ newRenderPipelineStateWithDescriptor:pipelineDescriptor
                                                                 error:&pipelineError];
        if (pipelineState_ == nil) {
            error.message = pipelineError.localizedDescription.UTF8String
                ?: "Failed to create the Metal mesh render pipeline.";
            return false;
        }

        MTLDepthStencilDescriptor *depthDescriptor = [[MTLDepthStencilDescriptor alloc] init];
        depthDescriptor.depthCompareFunction = MTLCompareFunctionLess;
        depthDescriptor.depthWriteEnabled = YES;
        depthState_ = [device_ newDepthStencilStateWithDescriptor:depthDescriptor];
        if (depthState_ == nil) {
            error.message = "Failed to create the Metal depth state.";
            return false;
        }

        vertexBuffer_ = [device_ newBufferWithBytes:mesh.positions.data()
                                              length:mesh.positions.size() * sizeof(core::Vector3)
                                             options:MTLResourceStorageModeShared];
        if (vertexBuffer_ == nil) {
            error.message = "Failed to create the Metal mesh vertex buffer.";
            return false;
        }

        std::vector<core::Vector3> fallbackNormals;
        const std::vector<core::Vector3> *normals = &mesh.normals;
        if (normals->empty()) {
            fallbackNormals.assign(mesh.positions.size(), core::Vector3(0.0f, 0.0f, -1.0f));
            normals = &fallbackNormals;
        }
        normalBuffer_ = [device_ newBufferWithBytes:normals->data()
                                              length:normals->size() * sizeof(core::Vector3)
                                             options:MTLResourceStorageModeShared];
        if (normalBuffer_ == nil) {
            error.message = "Failed to create the Metal mesh normal buffer.";
            return false;
        }

        vertexCount_ = mesh.positions.size();
        if (!mesh.indices.empty()) {
            indexBuffer_ = [device_ newBufferWithBytes:mesh.indices.data()
                                                 length:mesh.indices.size() * sizeof(std::uint32_t)
                                                options:MTLResourceStorageModeShared];
            if (indexBuffer_ == nil) {
                error.message = "Failed to create the Metal mesh index buffer.";
                return false;
            }
            indexCount_ = mesh.indices.size();
        }

        return resize(size, error);
    }

    bool render(void *opaqueFrame, Error &error) override
    {
        if (size_.width == 0 || size_.height == 0) {
            return true;
        }
        MetalFrame *frame = static_cast<MetalFrame *>(opaqueFrame);
        if (frame == nullptr || frame->drawable == nil || frame->renderPass == nil) {
            error.message = "The render surface supplied an invalid Metal frame.";
            return false;
        }
        if (depthTexture_ == nil || scene_ == nullptr || camera_ == nullptr
            || nodeIndex_ >= scene_->nodes.size()) {
            error.message = "The Metal renderer is missing scene or depth resources.";
            return false;
        }

        const core::SceneNodeData &node = scene_->nodes[nodeIndex_];
        const core::MaterialData material = node.materialIndex == core::InvalidIndex
            ? core::MaterialData()
            : scene_->materials[node.materialIndex];
        const float aspect = static_cast<float>(size_.width) / static_cast<float>(size_.height);
        RenderUniforms uniforms;
        uniforms.model = node.localTransform;
        uniforms.viewProjection = camera_->viewTransform * camera_->projection(aspect);
        uniforms.diffuseColor = material.diffuseColor;

        frame->renderPass.depthAttachment.texture = depthTexture_;
        frame->renderPass.depthAttachment.loadAction = MTLLoadActionClear;
        frame->renderPass.depthAttachment.storeAction = MTLStoreActionDontCare;
        frame->renderPass.depthAttachment.clearDepth = 1.0;

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
        [encoder setDepthStencilState:depthState_];
        [encoder setVertexBuffer:vertexBuffer_ offset:0 atIndex:0];
        [encoder setVertexBuffer:normalBuffer_ offset:0 atIndex:1];
        [encoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:2];
        if (indexBuffer_ != nil) {
            [encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                indexCount:indexCount_
                                 indexType:MTLIndexTypeUInt32
                               indexBuffer:indexBuffer_
                         indexBufferOffset:0];
        } else {
            [encoder drawPrimitives:MTLPrimitiveTypeTriangle
                       vertexStart:0
                       vertexCount:vertexCount_];
        }
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

    void shutdown() noexcept override
    {
        view_.device = nil;
        depthTexture_ = nil;
        indexBuffer_ = nil;
        normalBuffer_ = nil;
        vertexBuffer_ = nil;
        depthState_ = nil;
        pipelineState_ = nil;
        commandQueue_ = nil;
        device_ = nil;
        view_ = nil;
        scene_ = nullptr;
        camera_ = nullptr;
        nodeIndex_ = core::InvalidIndex;
        size_ = {0, 0};
        errorHandler_ = ErrorHandler();
    }

private:
    __strong id<MTLDevice> device_;
    __strong id<MTLCommandQueue> commandQueue_;
    __strong id<MTLRenderPipelineState> pipelineState_;
    __strong id<MTLDepthStencilState> depthState_;
    __strong id<MTLBuffer> vertexBuffer_;
    __strong id<MTLBuffer> normalBuffer_;
    __strong id<MTLBuffer> indexBuffer_;
    __strong id<MTLTexture> depthTexture_;
    __weak MTKView *view_;
    const core::SceneData *scene_;
    const core::CameraData *camera_;
    std::uint32_t nodeIndex_;
    NSUInteger indexCount_;
    NSUInteger vertexCount_;
    Size size_;
    ErrorHandler errorHandler_;
};

} // namespace

std::unique_ptr<detail::Renderer> createMetalRenderer()
{
    return std::unique_ptr<detail::Renderer>(new MetalRenderer());
}

} // namespace engine
