#pragma once

#include "Core/Types.h"
#include "Graphics/Adapter.h"
#include "Graphics/BindingLayout.h"
#include "Graphics/BindingSet.h"
#include "Graphics/Buffer.h"
#include "Graphics/BufferView.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/CommandContext.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/ComputeEncoder.h"
#include "Graphics/ComputePipeline.h"
#include "Graphics/CopyEncoder.h"
#include "Graphics/Device.h"
#include "Graphics/Fence.h"
#include "Graphics/GraphicsInstance.h"
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/MemoryAllocation.h"
#include "Graphics/MemoryAllocator.h"
#include "Graphics/RenderPassEncoder.h"
#include "Graphics/Sampler.h"
#include "Graphics/Shader.h"
#include "Graphics/Submission.h"
#include "Graphics/Swapchain.h"
#include "Graphics/Texture.h"
#include "Graphics/TextureView.h"

#include <Foundation/NSAutoreleasePool.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace Engine {
namespace Metal {

using namespace Graphics;

// Metal shares one 31-entry buffer argument table between resource bindings and
// vertex streams. Bindings use indices [0, kVertexBufferBase); vertex stream slot N
// uses index kVertexBufferBase + N.
constexpr std::uint32_t kVertexBufferBase = 16;
constexpr std::uint32_t kMaxVertexBufferSlots = 31 - kVertexBufferBase;
constexpr std::uint32_t kIRArgumentBufferBindPoint = 2;

constexpr std::uint32_t VertexBufferIndex(std::uint32_t slot)
{
    return kVertexBufferBase + slot;
}

class AutoreleasePool final
{
public:
    AutoreleasePool() : pool_(NS::TransferPtr(NS::AutoreleasePool::alloc()->init())) {}

private:
    NS::SharedPtr<NS::AutoreleasePool> pool_;
};

inline void SetError(Error &error, const char *message)
{
    error.message = message;
}

inline void SetMetalError(Error &error, const char *operation, NS::Error *nativeError)
{
    const char *description =
        nativeError == nullptr ? nullptr : nativeError->localizedDescription()->utf8String();
    error.message = operation;
    if (description != nullptr && description[0] != '\0') {
        error.message += ": ";
        error.message += description;
    }
}

inline bool HasBits(std::uint32_t value, std::uint32_t bits)
{
    return (value & bits) == bits;
}

inline bool IsValidMemoryType(MemoryType type)
{
    return type == MemoryType::DeviceLocal || type == MemoryType::Upload ||
           type == MemoryType::Readback;
}

inline bool IsValidTopology(PrimitiveTopology topology)
{
    return topology == PrimitiveTopology::TriangleList ||
           topology == PrimitiveTopology::TriangleStrip ||
           topology == PrimitiveTopology::LineList || topology == PrimitiveTopology::PointList;
}

inline bool IsValidCompareFunction(CompareFunction function)
{
    return function == CompareFunction::Never || function == CompareFunction::Less ||
           function == CompareFunction::LessEqual || function == CompareFunction::Equal ||
           function == CompareFunction::GreaterEqual || function == CompareFunction::Greater ||
           function == CompareFunction::Always;
}

inline bool CheckedMultiply(std::size_t lhs, std::size_t rhs, std::size_t &result)
{
    if (rhs != 0 && lhs > std::numeric_limits<std::size_t>::max() / rhs) {
        return false;
    }
    result = lhs * rhs;
    return true;
}

inline MTL::PixelFormat ToMetal(TextureFormat format)
{
    switch (format) {
    case TextureFormat::R8Unorm:
        return MTL::PixelFormatR8Unorm;
    case TextureFormat::RGBA8Unorm:
        return MTL::PixelFormatRGBA8Unorm;
    case TextureFormat::BGRA8Unorm:
        return MTL::PixelFormatBGRA8Unorm;
    case TextureFormat::RGBA16Float:
        return MTL::PixelFormatRGBA16Float;
    case TextureFormat::Depth32Float:
        return MTL::PixelFormatDepth32Float;
    case TextureFormat::Unknown:
        break;
    }
    return MTL::PixelFormatInvalid;
}

inline std::size_t BytesPerPixel(TextureFormat format)
{
    switch (format) {
    case TextureFormat::R8Unorm:
        return 1;
    case TextureFormat::RGBA8Unorm:
    case TextureFormat::BGRA8Unorm:
    case TextureFormat::Depth32Float:
        return 4;
    case TextureFormat::RGBA16Float:
        return 8;
    case TextureFormat::Unknown:
        return 0;
    }
    return 0;
}

inline MTL::StorageMode StorageModeFor(MemoryType type)
{
    return type == MemoryType::DeviceLocal ? MTL::StorageModePrivate : MTL::StorageModeShared;
}

inline MTL::ResourceOptions ResourceOptionsFor(MemoryType type)
{
    MTL::ResourceOptions options = MTL::ResourceStorageModeShared;
    if (type == MemoryType::DeviceLocal) {
        options = MTL::ResourceStorageModePrivate;
    } else if (type == MemoryType::Upload) {
        options = static_cast<MTL::ResourceOptions>(MTL::ResourceStorageModeShared |
                                                    MTL::ResourceCPUCacheModeWriteCombined);
    }
    return options;
}

inline bool SupportsTextureUsage(TextureUsage usage, TextureFormat format)
{
    const std::uint32_t flags = static_cast<std::uint32_t>(usage);
    constexpr std::uint32_t validFlags =
        static_cast<std::uint32_t>(TextureUsage::Sampled) |
        static_cast<std::uint32_t>(TextureUsage::Storage) |
        static_cast<std::uint32_t>(TextureUsage::ColorAttachment) |
        static_cast<std::uint32_t>(TextureUsage::DepthStencilAttachment) |
        static_cast<std::uint32_t>(TextureUsage::CopySource) |
        static_cast<std::uint32_t>(TextureUsage::CopyDestination);
    if (flags == 0 || (flags & ~validFlags) != 0) {
        return false;
    }
    if ((flags & static_cast<std::uint32_t>(TextureUsage::ColorAttachment)) != 0 &&
        format == TextureFormat::Depth32Float) {
        return false;
    }
    if ((flags & static_cast<std::uint32_t>(TextureUsage::DepthStencilAttachment)) != 0 &&
        format != TextureFormat::Depth32Float) {
        return false;
    }
    if ((flags & static_cast<std::uint32_t>(TextureUsage::Storage)) != 0 &&
        format == TextureFormat::Depth32Float) {
        return false;
    }
    return true;
}

inline MTL::TextureUsage ToMetal(TextureUsage usage)
{
    MTL::TextureUsage result = MTL::TextureUsageUnknown;
    const std::uint32_t flags = static_cast<std::uint32_t>(usage);
    if ((flags & static_cast<std::uint32_t>(TextureUsage::Sampled)) != 0) {
        result = static_cast<MTL::TextureUsage>(result | MTL::TextureUsageShaderRead);
    }
    if ((flags & static_cast<std::uint32_t>(TextureUsage::Storage)) != 0) {
        result = static_cast<MTL::TextureUsage>(result | MTL::TextureUsageShaderWrite);
    }
    if ((flags & (static_cast<std::uint32_t>(TextureUsage::ColorAttachment) |
                  static_cast<std::uint32_t>(TextureUsage::DepthStencilAttachment))) != 0) {
        result = static_cast<MTL::TextureUsage>(result | MTL::TextureUsageRenderTarget);
    }
    return result;
}

inline MTL::CompareFunction ToMetal(CompareFunction function)
{
    switch (function) {
    case CompareFunction::Never:
        return MTL::CompareFunctionNever;
    case CompareFunction::Less:
        return MTL::CompareFunctionLess;
    case CompareFunction::LessEqual:
        return MTL::CompareFunctionLessEqual;
    case CompareFunction::Equal:
        return MTL::CompareFunctionEqual;
    case CompareFunction::GreaterEqual:
        return MTL::CompareFunctionGreaterEqual;
    case CompareFunction::Greater:
        return MTL::CompareFunctionGreater;
    case CompareFunction::Always:
        return MTL::CompareFunctionAlways;
    }
    return MTL::CompareFunctionAlways;
}

inline MTL::PrimitiveType ToMetal(PrimitiveTopology topology)
{
    switch (topology) {
    case PrimitiveTopology::TriangleList:
        return MTL::PrimitiveTypeTriangle;
    case PrimitiveTopology::TriangleStrip:
        return MTL::PrimitiveTypeTriangleStrip;
    case PrimitiveTopology::LineList:
        return MTL::PrimitiveTypeLine;
    case PrimitiveTopology::PointList:
        return MTL::PrimitiveTypePoint;
    }
    return MTL::PrimitiveTypeTriangle;
}

inline MTL::VertexFormat ToMetal(VertexFormat format)
{
    switch (format) {
    case VertexFormat::Float2:
        return MTL::VertexFormatFloat2;
    case VertexFormat::Float3:
        return MTL::VertexFormatFloat3;
    case VertexFormat::Float4:
        return MTL::VertexFormatFloat4;
    case VertexFormat::UInt32:
        return MTL::VertexFormatUInt;
    }
    return MTL::VertexFormatInvalid;
}

inline MTL::SamplerAddressMode ToMetal(AddressMode mode)
{
    switch (mode) {
    case AddressMode::Repeat:
        return MTL::SamplerAddressModeRepeat;
    case AddressMode::ClampToEdge:
        return MTL::SamplerAddressModeClampToEdge;
    case AddressMode::MirroredRepeat:
        return MTL::SamplerAddressModeMirrorRepeat;
    }
    return MTL::SamplerAddressModeClampToEdge;
}

struct NativeDevice {
    explicit NativeDevice(MTL::Device *value) : device(NS::RetainPtr(value))
    {
        capabilities.name = value->name()->utf8String();
        capabilities.maxTextureDimension2D = 16384;
        capabilities.maxColorAttachments = 8;
        capabilities.queueTypes = {QueueType::Graphics, QueueType::Compute, QueueType::Copy};
        capabilities.supportedTextureFormats = {
            TextureFormat::R8Unorm,     TextureFormat::RGBA8Unorm,   TextureFormat::BGRA8Unorm,
            TextureFormat::RGBA16Float, TextureFormat::Depth32Float,
        };
        capabilities.supportsCompute = true;
        capabilities.supportsIndirectDrawing = false;
    }

    NS::SharedPtr<MTL::Device> device;
    DeviceCapabilities capabilities;
};

struct BindingValue {
    std::shared_ptr<BufferView> buffer;
    std::shared_ptr<TextureView> texture;
    std::shared_ptr<Sampler> sampler;
};

struct SubmissionState {
    NS::SharedPtr<MTL::CommandBuffer> commandBuffer;
    mutable std::mutex mutex;
    bool completed = false;
    std::string error;
};

class MetalAdapter;
class MetalGraphicsInstance;
class MetalBuffer;
class MetalBufferView;
class MetalTexture;
class MetalTextureView;
class MetalShader;
class MetalSampler;
class MetalBindingLayout;
class MetalBindingSet;
class MetalGraphicsPipeline;
class MetalComputePipeline;
class MetalMemoryAllocation;
class MetalMemoryAllocator;
class MetalFence;
class MetalSubmission;
class MetalCommandBuffer;
class MetalQueue;
class MetalCommandContext;
class MetalRenderEncoder;
class MetalComputeEncoder;
class MetalCopyEncoder;
class MetalSwapchainImage;
class MetalSwapchain;
class MetalDevice;

} // namespace Metal
} // namespace Engine
