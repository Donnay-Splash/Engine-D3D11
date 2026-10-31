#ifndef ENGINE_GRAPHICS_GRAPHICS_TYPES_H
#define ENGINE_GRAPHICS_GRAPHICS_TYPES_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Engine {
namespace Graphics {

class TextureView;

enum class QueueType
{
    Graphics,
    Compute,
    Copy
};

enum class MemoryType
{
    DeviceLocal,
    Upload,
    Readback
};

enum class BufferUsage : std::uint32_t
{
    None = 0,
    Vertex = 1 << 0,
    Index = 1 << 1,
    Uniform = 1 << 2,
    Storage = 1 << 3,
    CopySource = 1 << 4,
    CopyDestination = 1 << 5,
    Indirect = 1 << 6
};

enum class TextureUsage : std::uint32_t
{
    None = 0,
    Sampled = 1 << 0,
    Storage = 1 << 1,
    ColorAttachment = 1 << 2,
    DepthStencilAttachment = 1 << 3,
    CopySource = 1 << 4,
    CopyDestination = 1 << 5
};

enum class TextureFormat
{
    Unknown,
    R8Unorm,
    RGBA8Unorm,
    BGRA8Unorm,
    RGBA16Float,
    Depth32Float
};

enum class BindingType
{
    UniformBuffer,
    StorageBuffer,
    SampledTexture,
    StorageTexture,
    Sampler
};

enum class FilterMode
{
    Nearest,
    Linear
};

enum class AddressMode
{
    Repeat,
    ClampToEdge,
    MirroredRepeat
};

enum class ShaderStage : std::uint32_t
{
    None = 0,
    Vertex = 1 << 0,
    Fragment = 1 << 1,
    Compute = 1 << 2
};

enum class PrimitiveTopology
{
    TriangleList,
    TriangleStrip,
    LineList,
    PointList
};

enum class IndexFormat
{
    UInt16,
    UInt32
};

enum class VertexFormat
{
    Float2,
    Float3,
    Float4,
    UInt32
};

enum class CullMode
{
    None,
    Front,
    Back
};

enum class CompareFunction
{
    Never,
    Less,
    LessEqual,
    Equal,
    GreaterEqual,
    Greater,
    Always
};

enum class LoadAction
{
    Load,
    Clear,
    DontCare
};

enum class StoreAction
{
    Store,
    DontCare
};

enum class ResourceUsage
{
    Undefined,
    CopySource,
    CopyDestination,
    VertexBuffer,
    IndexBuffer,
    UniformBuffer,
    ShaderRead,
    ShaderWrite,
    ColorAttachment,
    DepthStencilAttachment,
    Present
};

inline std::uint32_t BytesPerPixel(TextureFormat format)
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
        break;
    }
    return 0;
}

constexpr BufferUsage operator|(BufferUsage lhs, BufferUsage rhs)
{
    return static_cast<BufferUsage>(
        static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
}

constexpr TextureUsage operator|(TextureUsage lhs, TextureUsage rhs)
{
    return static_cast<TextureUsage>(
        static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
}

constexpr ShaderStage operator|(ShaderStage lhs, ShaderStage rhs)
{
    return static_cast<ShaderStage>(
        static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
}

struct Extent3D
{
    std::uint32_t width = 1;
    std::uint32_t height = 1;
    std::uint32_t depth = 1;
};

struct Viewport
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float minDepth = 0.0f;
    float maxDepth = 1.0f;
};

struct BufferDesc
{
    std::size_t size = 0;
    BufferUsage usage = BufferUsage::None;
    MemoryType memoryType = MemoryType::DeviceLocal;
};

struct TextureDesc
{
    TextureFormat format = TextureFormat::Unknown;
    Extent3D extent;
    std::uint32_t mipLevels = 1;
    std::uint32_t arrayLayers = 1;
    std::uint32_t sampleCount = 1;
    TextureUsage usage = TextureUsage::None;
    MemoryType memoryType = MemoryType::DeviceLocal;
};

struct ResourceRange
{
    std::uint32_t firstMip = 0;
    std::uint32_t mipCount = 1;
    std::uint32_t firstLayer = 0;
    std::uint32_t layerCount = 1;
};

struct ShaderDesc
{
    ShaderStage stage = ShaderStage::None;
    std::string entryPoint;
    std::string format;
    std::vector<std::uint8_t> bytecode;
    std::vector<std::uint8_t> reflection;
};

struct SamplerDesc
{
    FilterMode minFilter = FilterMode::Linear;
    FilterMode magFilter = FilterMode::Linear;
    AddressMode addressU = AddressMode::Repeat;
    AddressMode addressV = AddressMode::Repeat;
    AddressMode addressW = AddressMode::Repeat;
};

struct BindingDesc
{
    std::uint32_t binding = 0;
    BindingType type = BindingType::UniformBuffer;
    std::uint32_t arrayCount = 1;
    ShaderStage visibility = ShaderStage::None;
};

struct VertexAttributeDesc
{
    std::uint32_t location = 0;
    std::uint32_t bufferSlot = 0;
    VertexFormat format = VertexFormat::Float3;
    std::uint32_t offset = 0;
};

struct VertexBufferLayoutDesc
{
    std::uint32_t bufferSlot = 0;
    std::uint32_t stride = 0;
};

struct DepthStencilState
{
    bool depthTestEnabled = false;
    bool depthWriteEnabled = false;
    CompareFunction depthCompare = CompareFunction::Less;
};

struct RenderAttachmentDesc
{
    std::shared_ptr<TextureView> view;
    LoadAction loadAction = LoadAction::DontCare;
    StoreAction storeAction = StoreAction::Store;
    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    float clearDepth = 1.0f;
    std::uint32_t clearStencil = 0;
};

struct RenderPassDesc
{
    std::vector<RenderAttachmentDesc> colorAttachments;
    bool hasDepthStencilAttachment = false;
    RenderAttachmentDesc depthStencilAttachment;
    Extent3D extent;
};

struct ResourceBarrierDesc
{
    ResourceUsage before = ResourceUsage::Undefined;
    ResourceUsage after = ResourceUsage::Undefined;
    ResourceRange range;
};

} // namespace Graphics
} // namespace Engine

#endif
