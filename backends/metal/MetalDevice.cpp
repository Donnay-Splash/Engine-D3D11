#include "MetalBackendInternal.h"
#include "MetalSurfaceAdapter.h"

#include <charconv>
#include <string_view>

namespace Engine {
namespace Metal {

using namespace Graphics;

namespace {

template <typename T>
bool JsonNumber(std::string_view object, const char *name, T &value)
{
    const std::string key = std::string("\"") + name + "\"";
    const std::size_t keyPosition = object.find(key);
    if (keyPosition == std::string_view::npos) {
        return false;
    }
    const std::size_t colon = object.find(':', keyPosition + key.size());
    if (colon == std::string_view::npos) {
        return false;
    }
    std::size_t begin = colon + 1;
    while (begin < object.size() &&
           (object[begin] == ' ' || object[begin] == '\t' || object[begin] == '\r' ||
            object[begin] == '\n')) {
        ++begin;
    }
    std::size_t end = begin;
    while (end < object.size() && object[end] >= '0' && object[end] <= '9') {
        ++end;
    }
    return end != begin &&
           std::from_chars(object.data() + begin, object.data() + end, value).ec ==
               std::errc{};
}

bool JsonString(std::string_view object, const char *name, std::string &value)
{
    const std::string key = std::string("\"") + name + "\"";
    const std::size_t keyPosition = object.find(key);
    if (keyPosition == std::string_view::npos) {
        return false;
    }
    const std::size_t colon = object.find(':', keyPosition + key.size());
    if (colon == std::string_view::npos) {
        return false;
    }
    const std::size_t begin = object.find('"', colon + 1);
    if (begin == std::string_view::npos) {
        return false;
    }
    const std::size_t end = object.find('"', begin + 1);
    if (end == std::string_view::npos) {
        return false;
    }
    value.assign(object.substr(begin + 1, end - begin - 1));
    return true;
}

bool ParseBindingLocations(const std::vector<std::uint8_t> &reflection,
                           std::vector<MetalBindingLocation> &locations, Error &error)
{
    if (reflection.empty()) {
        return true;
    }
    const std::string_view json(reinterpret_cast<const char *>(reflection.data()),
                                reflection.size());
    const std::string_view property = "\"TopLevelArgumentBuffer\"";
    const std::size_t propertyPosition = json.find(property);
    if (propertyPosition == std::string_view::npos) {
        return true;
    }
    const std::size_t arrayBegin = json.find('[', propertyPosition + property.size());
    if (arrayBegin == std::string_view::npos) {
        SetError(error, "The Metal shader reflection has an invalid argument-buffer layout.");
        return false;
    }

    std::size_t position = arrayBegin + 1;
    while (position < json.size()) {
        const std::size_t objectBegin = json.find('{', position);
        const std::size_t arrayEnd = json.find(']', position);
        if (arrayEnd == std::string_view::npos ||
            (objectBegin != std::string_view::npos && objectBegin > arrayEnd)) {
            return true;
        }
        const std::size_t objectEnd = json.find('}', objectBegin + 1);
        if (objectEnd == std::string_view::npos || objectEnd > arrayEnd) {
            SetError(error, "The Metal shader reflection has an invalid argument-buffer entry.");
            return false;
        }

        const std::string_view object = json.substr(objectBegin, objectEnd - objectBegin + 1);
        MetalBindingLocation location;
        if (!JsonNumber(object, "Slot", location.slot) ||
            !JsonNumber(object, "Space", location.space) ||
            !JsonNumber(object, "EltOffset", location.offset) ||
            !JsonNumber(object, "Size", location.size) ||
            !JsonString(object, "Type", location.type)) {
            SetError(error, "The Metal shader reflection has an incomplete argument-buffer entry.");
            return false;
        }
        if (location.size == 0 || location.offset > std::numeric_limits<std::size_t>::max() -
                                                       location.size) {
            SetError(error, "The Metal shader reflection contains an invalid argument-buffer range.");
            return false;
        }
        locations.push_back(std::move(location));
        position = objectEnd + 1;
    }
    SetError(error, "The Metal shader reflection has an unterminated argument-buffer layout.");
    return false;
}

} // namespace

MetalDevice::MetalDevice(NS::SharedPtr<MTL::Device> device)
    : device_(std::move(device)), allocator_(std::make_shared<MetalMemoryAllocator>(device_))
{
    capabilities_.name = device_->name()->utf8String();
    capabilities_.maxTextureDimension2D = 16384;
    capabilities_.maxColorAttachments = 8;
    capabilities_.queueTypes = {QueueType::Graphics, QueueType::Compute, QueueType::Copy};
    capabilities_.supportedTextureFormats = {TextureFormat::R8Unorm, TextureFormat::RGBA8Unorm,
                                             TextureFormat::BGRA8Unorm, TextureFormat::RGBA16Float,
                                             TextureFormat::Depth32Float};
    capabilities_.supportsCompute = true;
    capabilities_.supportsIndirectDrawing = false;
    nativeQueue_ = NS::RetainPtr(device_->newCommandQueue());
}

const DeviceCapabilities &MetalDevice::Capabilities() const noexcept
{
    return capabilities_;
}

std::shared_ptr<Buffer> MetalDevice::CreateBuffer(const BufferDesc &description, Error &error)
{
    AutoreleasePool pool;
    constexpr std::uint32_t validUsage = static_cast<std::uint32_t>(BufferUsage::Vertex) |
                                         static_cast<std::uint32_t>(BufferUsage::Index) |
                                         static_cast<std::uint32_t>(BufferUsage::Uniform) |
                                         static_cast<std::uint32_t>(BufferUsage::Storage) |
                                         static_cast<std::uint32_t>(BufferUsage::CopySource) |
                                         static_cast<std::uint32_t>(BufferUsage::CopyDestination) |
                                         static_cast<std::uint32_t>(BufferUsage::Indirect);
    const std::uint32_t usage = static_cast<std::uint32_t>(description.usage);
    if (description.size == 0 || description.size > device_->maxBufferLength() || usage == 0 ||
        (usage & ~validUsage) != 0 || !IsValidMemoryType(description.memoryType)) {
        SetError(error, "The Metal buffer size or usage is invalid.");
        return nullptr;
    }
    MTL::Buffer *buffer =
        device_->newBuffer(description.size, ResourceOptionsFor(description.memoryType));
    if (buffer == nullptr) {
        SetError(error, "Metal failed to create the requested buffer.");
        return nullptr;
    }
    return std::make_shared<MetalBuffer>(description, buffer);
}

std::shared_ptr<Texture> MetalDevice::CreateTexture(const TextureDesc &description, Error &error)
{
    AutoreleasePool pool;
    const std::uint32_t maxDimension = capabilities_.maxTextureDimension2D;
    if (ToMetal(description.format) == MTL::PixelFormatInvalid || description.extent.width == 0 ||
        description.extent.height == 0 || description.extent.depth == 0 ||
        description.mipLevels == 0 || description.arrayLayers == 0 ||
        description.sampleCount == 0 || description.extent.width > maxDimension ||
        description.extent.height > maxDimension || description.extent.depth > maxDimension ||
        !IsValidMemoryType(description.memoryType) ||
        !SupportsTextureUsage(description.usage, description.format)) {
        SetError(error, "The Metal texture description is invalid or unsupported.");
        return nullptr;
    }
    const std::uint32_t largest = std::max(
        description.extent.width, std::max(description.extent.height, description.extent.depth));
    std::uint32_t maxMipLevels = 1;
    for (std::uint32_t size = largest; size > 1; size >>= 1) {
        ++maxMipLevels;
    }
    if (description.mipLevels > maxMipLevels ||
        (description.extent.depth > 1 && description.arrayLayers != 1) ||
        (description.extent.depth > 1 && description.sampleCount != 1) ||
        (description.sampleCount > 1 &&
         (description.extent.depth != 1 || description.arrayLayers != 1 ||
          description.mipLevels != 1 || description.memoryType != MemoryType::DeviceLocal)) ||
        !device_->supportsTextureSampleCount(description.sampleCount)) {
        SetError(error, "The Metal texture mip, layer, or sample configuration is invalid.");
        return nullptr;
    }
    NS::SharedPtr<MTL::TextureDescriptor> descriptor =
        NS::TransferPtr(MTL::TextureDescriptor::alloc()->init());
    descriptor->setPixelFormat(ToMetal(description.format));
    descriptor->setWidth(description.extent.width);
    descriptor->setHeight(description.extent.height);
    descriptor->setDepth(description.extent.depth);
    descriptor->setMipmapLevelCount(description.mipLevels);
    descriptor->setArrayLength(description.arrayLayers);
    descriptor->setSampleCount(description.sampleCount);
    descriptor->setStorageMode(StorageModeFor(description.memoryType));
    descriptor->setUsage(ToMetal(description.usage));
    descriptor->setTextureType(description.sampleCount > 1    ? MTL::TextureType2DMultisample
                               : description.extent.depth > 1 ? MTL::TextureType3D
                               : description.arrayLayers > 1  ? MTL::TextureType2DArray
                                                              : MTL::TextureType2D);
    MTL::Texture *texture = device_->newTexture(descriptor.get());
    if (texture == nullptr) {
        SetError(error, "Metal failed to create the requested texture.");
        return nullptr;
    }
    return std::make_shared<MetalTexture>(description, texture);
}

std::shared_ptr<Shader> MetalDevice::CreateShader(const ShaderDesc &description, Error &error)
{
    AutoreleasePool pool;
    if ((description.stage != ShaderStage::Vertex && description.stage != ShaderStage::Fragment &&
         description.stage != ShaderStage::Compute) ||
        description.entryPoint.empty() || description.bytecode.empty() ||
        description.format != "metallib") {
        SetError(error, "Metal shaders require a valid stage, entry point, and metallib bytecode.");
        return nullptr;
    }
    std::vector<MetalBindingLocation> bindingLocations;
    if (!ParseBindingLocations(description.reflection, bindingLocations, error)) {
        return nullptr;
    }
    void *ownedBytes = std::malloc(description.bytecode.size());
    if (ownedBytes == nullptr) {
        SetError(error, "Metal could not allocate shader bytecode storage.");
        return nullptr;
    }
    std::memcpy(ownedBytes, description.bytecode.data(), description.bytecode.size());
    dispatch_data_t data =
        dispatch_data_create(ownedBytes, description.bytecode.size(), dispatch_get_main_queue(),
                             DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    if (data == nullptr) {
        std::free(ownedBytes);
        SetError(error, "Metal could not create shader bytecode data.");
        return nullptr;
    }
    NS::Error *nativeError = nullptr;
    MTL::Library *library = device_->newLibrary(data, &nativeError);
    dispatch_release(data);
    if (library == nullptr) {
        SetMetalError(error, "Metal failed to load the shader library", nativeError);
        return nullptr;
    }
    NS::SharedPtr<MTL::Library> retainedLibrary = NS::RetainPtr(library);
    NS::SharedPtr<NS::String> functionName =
        NS::RetainPtr(NS::String::string(description.entryPoint.c_str(), NS::UTF8StringEncoding));
    MTL::Function *function = retainedLibrary->newFunction(functionName.get());
    if (function == nullptr) {
        error.message = "The Metal shader library does not contain entry point '" +
                        description.entryPoint + "'.";
        return nullptr;
    }
    return std::make_shared<MetalShader>(description.stage, description.entryPoint,
                                         retainedLibrary.get(), function,
                                         std::move(bindingLocations));
}

std::shared_ptr<BufferView> MetalDevice::CreateBufferView(const std::shared_ptr<Buffer> &buffer,
                                                          std::size_t offset, std::size_t size,
                                                          Error &error)
{
    auto metalBuffer = std::dynamic_pointer_cast<MetalBuffer>(buffer);
    if (metalBuffer == nullptr || size == 0 || offset > metalBuffer->Description().size ||
        size > metalBuffer->Description().size - offset) {
        SetError(error, "The Metal buffer view range is invalid.");
        return nullptr;
    }
    return std::make_shared<MetalBufferView>(buffer, offset, size);
}

std::shared_ptr<TextureView> MetalDevice::CreateTextureView(const std::shared_ptr<Texture> &texture,
                                                            const ResourceRange &range,
                                                            TextureFormat format, Error &error)
{
    AutoreleasePool pool;
    auto metalTexture = std::dynamic_pointer_cast<MetalTexture>(texture);
    if (metalTexture == nullptr) {
        SetError(error, "The Metal texture view requires a Metal texture.");
        return nullptr;
    }
    const TextureDesc &description = metalTexture->Description();
    const TextureFormat viewFormat = format == TextureFormat::Unknown ? description.format : format;
    if (ToMetal(viewFormat) == MTL::PixelFormatInvalid || viewFormat != description.format ||
        range.mipCount == 0 || range.layerCount == 0 || range.firstMip >= description.mipLevels ||
        range.mipCount > description.mipLevels - range.firstMip ||
        range.firstLayer >= description.arrayLayers ||
        range.layerCount > description.arrayLayers - range.firstLayer) {
        SetError(error, "The Metal texture view format or subresource range is invalid.");
        return nullptr;
    }
    MTL::Texture *view = metalTexture->Native();
    if (range.firstMip != 0 || range.mipCount != description.mipLevels || range.firstLayer != 0 ||
        range.layerCount != description.arrayLayers) {
        const MTL::TextureType type = description.sampleCount > 1    ? MTL::TextureType2DMultisample
                                      : description.extent.depth > 1 ? MTL::TextureType3D
                                      : description.arrayLayers > 1  ? MTL::TextureType2DArray
                                                                     : MTL::TextureType2D;
        view = metalTexture->Native()->newTextureView(
            ToMetal(viewFormat), type, NS::Range(range.firstMip, range.mipCount),
            NS::Range(range.firstLayer, range.layerCount));
        if (view == nullptr) {
            SetError(error, "Metal failed to create the requested texture view.");
            return nullptr;
        }
    }
    return std::make_shared<MetalTextureView>(texture, range, viewFormat, view);
}

std::shared_ptr<Sampler> MetalDevice::CreateSampler(const SamplerDesc &description, Error &error)
{
    AutoreleasePool pool;
    NS::SharedPtr<MTL::SamplerDescriptor> descriptor =
        NS::TransferPtr(MTL::SamplerDescriptor::alloc()->init());
    const auto filter = [](FilterMode mode) {
        return mode == FilterMode::Nearest ? MTL::SamplerMinMagFilterNearest
                                           : MTL::SamplerMinMagFilterLinear;
    };
    const auto validFilter = [](FilterMode mode) {
        return mode == FilterMode::Nearest || mode == FilterMode::Linear;
    };
    const auto validAddress = [](AddressMode mode) {
        return mode == AddressMode::Repeat || mode == AddressMode::ClampToEdge ||
               mode == AddressMode::MirroredRepeat;
    };
    if (!validFilter(description.minFilter) || !validFilter(description.magFilter) ||
        !validAddress(description.addressU) || !validAddress(description.addressV) ||
        !validAddress(description.addressW)) {
        SetError(error, "The Metal sampler description contains an invalid mode.");
        return nullptr;
    }
    descriptor->setMinFilter(filter(description.minFilter));
    descriptor->setMagFilter(filter(description.magFilter));
    descriptor->setSAddressMode(ToMetal(description.addressU));
    descriptor->setTAddressMode(ToMetal(description.addressV));
    descriptor->setRAddressMode(ToMetal(description.addressW));
    MTL::SamplerState *sampler = device_->newSamplerState(descriptor.get());
    if (sampler == nullptr) {
        SetError(error, "Metal failed to create the requested sampler.");
        return nullptr;
    }
    return std::make_shared<MetalSampler>(description, sampler);
}

std::shared_ptr<BindingLayout>
MetalDevice::CreateBindingLayout(const BindingLayoutDesc &description, Error &error)
{
    std::vector<BindingDesc> sorted = description.bindings;
    std::sort(sorted.begin(), sorted.end(), [](const BindingDesc &lhs, const BindingDesc &rhs) {
        return lhs.binding < rhs.binding;
    });
    for (std::size_t index = 0; index < sorted.size(); ++index) {
        if (sorted[index].arrayCount == 0 || sorted[index].visibility == ShaderStage::None ||
            static_cast<std::uint64_t>(sorted[index].binding) + sorted[index].arrayCount > 31 ||
            (index != 0 &&
             static_cast<std::uint64_t>(sorted[index - 1].binding) + sorted[index - 1].arrayCount >
                 sorted[index].binding)) {
            SetError(error, "The Metal binding layout contains a duplicate or invalid binding.");
            return nullptr;
        }
        switch (sorted[index].type) {
        case BindingType::UniformBuffer:
        case BindingType::StorageBuffer:
        case BindingType::SampledTexture:
        case BindingType::StorageTexture:
        case BindingType::Sampler:
            break;
        default:
            SetError(error, "The Metal binding layout contains an unsupported binding type.");
            return nullptr;
        }
        const std::uint32_t visibility = static_cast<std::uint32_t>(sorted[index].visibility);
        if ((visibility & ~static_cast<std::uint32_t>(ShaderStage::Vertex | ShaderStage::Fragment |
                                                      ShaderStage::Compute)) != 0) {
            SetError(error, "The Metal binding layout has unsupported shader visibility.");
            return nullptr;
        }
    }
    return std::make_shared<MetalBindingLayout>(std::move(sorted));
}

std::shared_ptr<BindingSet>
MetalDevice::CreateBindingSet(const std::shared_ptr<BindingLayout> &layout, Error &error)
{
    if (std::dynamic_pointer_cast<MetalBindingLayout>(layout) == nullptr) {
        SetError(error, "The Metal binding set requires a Metal binding layout.");
        return nullptr;
    }
    return std::make_shared<MetalBindingSet>(layout);
}

std::shared_ptr<GraphicsPipeline>
MetalDevice::CreateGraphicsPipeline(const GraphicsPipelineDesc &description, Error &error)
{
    AutoreleasePool pool;
    auto bindingLayout = std::dynamic_pointer_cast<MetalBindingLayout>(description.bindingLayout);
    if (description.shaders.size() != 2 || description.colorFormats.empty() ||
        description.colorFormats.size() > 8 || description.sampleCount == 0 ||
        !device_->supportsTextureSampleCount(description.sampleCount) ||
        !IsValidTopology(description.topology) ||
        (description.cullMode != CullMode::None && description.cullMode != CullMode::Front &&
         description.cullMode != CullMode::Back) ||
        !IsValidCompareFunction(description.depthStencil.depthCompare) ||
        (description.depthStencilFormat != TextureFormat::Unknown &&
         description.depthStencilFormat != TextureFormat::Depth32Float) ||
        (description.bindingLayout != nullptr && bindingLayout == nullptr)) {
        SetError(error, "The Metal graphics pipeline description is invalid.");
        return nullptr;
    }
    std::shared_ptr<MetalShader> vertex;
    std::shared_ptr<MetalShader> fragment;
    for (const std::shared_ptr<Shader> &shader : description.shaders) {
        auto metalShader = std::dynamic_pointer_cast<MetalShader>(shader);
        if (metalShader == nullptr) {
            SetError(error, "The graphics pipeline contains a non-Metal shader.");
            return nullptr;
        }
        if (metalShader->Stage() == ShaderStage::Vertex) {
            vertex = metalShader;
        } else if (metalShader->Stage() == ShaderStage::Fragment) {
            fragment = metalShader;
        }
    }
    if (vertex == nullptr || fragment == nullptr) {
        SetError(error, "A Metal graphics pipeline requires one vertex and one fragment shader.");
        return nullptr;
    }
    if (description.depthStencil.depthTestEnabled &&
        description.depthStencilFormat == TextureFormat::Unknown) {
        SetError(error, "Depth testing requires a depth attachment format.");
        return nullptr;
    }

    NS::SharedPtr<MTL::RenderPipelineDescriptor> pipelineDescriptor =
        NS::TransferPtr(MTL::RenderPipelineDescriptor::alloc()->init());
    pipelineDescriptor->setVertexFunction(vertex->Native());
    pipelineDescriptor->setFragmentFunction(fragment->Native());
    pipelineDescriptor->setSampleCount(description.sampleCount);
    pipelineDescriptor->setDepthAttachmentPixelFormat(ToMetal(description.depthStencilFormat));
    MTL::VertexDescriptor *vertexDescriptor = MTL::VertexDescriptor::vertexDescriptor();
    std::array<bool, 31> vertexAttributeLocations{};
    std::array<const VertexBufferLayoutDesc *, kMaxVertexBufferSlots> vertexBufferLayouts{};
    for (const VertexBufferLayoutDesc &buffer : description.vertexBuffers) {
        if (buffer.bufferSlot >= vertexBufferLayouts.size() || buffer.stride == 0 ||
            vertexBufferLayouts[buffer.bufferSlot] != nullptr) {
            SetError(error, "The Metal vertex buffer layout is invalid.");
            return nullptr;
        }
        vertexBufferLayouts[buffer.bufferSlot] = &buffer;
        MTL::VertexBufferLayoutDescriptor *nativeLayout =
            vertexDescriptor->layouts()->object(VertexBufferIndex(buffer.bufferSlot));
        nativeLayout->setStride(buffer.stride);
        nativeLayout->setStepFunction(MTL::VertexStepFunctionPerVertex);
        nativeLayout->setStepRate(1);
    }
    for (const VertexAttributeDesc &attribute : description.vertexAttributes) {
        const std::uint32_t attributeSize = attribute.format == VertexFormat::Float2   ? 8
                                            : attribute.format == VertexFormat::Float3 ? 12
                                            : attribute.format == VertexFormat::Float4 ? 16
                                            : attribute.format == VertexFormat::UInt32 ? 4
                                                                                       : 0;
        if (attribute.location >= vertexAttributeLocations.size() - kIRStageInAttributeStartIndex ||
            attribute.bufferSlot >= vertexBufferLayouts.size() ||
            vertexAttributeLocations[attribute.location] ||
            vertexBufferLayouts[attribute.bufferSlot] == nullptr || attributeSize == 0 ||
            attribute.offset > vertexBufferLayouts[attribute.bufferSlot]->stride ||
            attributeSize > vertexBufferLayouts[attribute.bufferSlot]->stride - attribute.offset ||
            ToMetal(attribute.format) == MTL::VertexFormatInvalid) {
            SetError(error, "The Metal vertex attribute is invalid.");
            return nullptr;
        }
        vertexAttributeLocations[attribute.location] = true;
        MTL::VertexAttributeDescriptor *nativeAttribute =
            vertexDescriptor->attributes()->object(kIRStageInAttributeStartIndex + attribute.location);
        nativeAttribute->setFormat(ToMetal(attribute.format));
        nativeAttribute->setBufferIndex(VertexBufferIndex(attribute.bufferSlot));
        nativeAttribute->setOffset(attribute.offset);
    }
    pipelineDescriptor->setVertexDescriptor(vertexDescriptor);

    for (std::size_t index = 0; index < description.colorFormats.size(); ++index) {
        const MTL::PixelFormat format = ToMetal(description.colorFormats[index]);
        if (format == MTL::PixelFormatInvalid ||
            description.colorFormats[index] == TextureFormat::Depth32Float) {
            SetError(error, "The Metal pipeline has an invalid color format.");
            return nullptr;
        }
        pipelineDescriptor->colorAttachments()->object(index)->setPixelFormat(format);
    }
    NS::Error *nativeError = nullptr;
    MTL::RenderPipelineState *pipeline =
        device_->newRenderPipelineState(pipelineDescriptor.get(), &nativeError);
    if (pipeline == nullptr) {
        SetMetalError(error, "Metal failed to create the render pipeline", nativeError);
        return nullptr;
    }

    NS::SharedPtr<MTL::DepthStencilState> depthState;
    NS::SharedPtr<MTL::DepthStencilDescriptor> depthDescriptor =
        NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    depthDescriptor->setDepthCompareFunction(description.depthStencil.depthTestEnabled
                                                 ? ToMetal(description.depthStencil.depthCompare)
                                                 : MTL::CompareFunctionAlways);
    depthDescriptor->setDepthWriteEnabled(description.depthStencil.depthTestEnabled &&
                                          description.depthStencil.depthWriteEnabled);
    depthState = NS::RetainPtr(device_->newDepthStencilState(depthDescriptor.get()));
    if (!depthState) {
        SetError(error, "Metal failed to create a depth-stencil state.");
        return nullptr;
    }
    return std::make_shared<MetalGraphicsPipeline>(description, pipeline, depthState.get(),
                                                   device_.get());
}

std::shared_ptr<ComputePipeline>
MetalDevice::CreateComputePipeline(const ComputePipelineDesc &description, Error &error)
{
    AutoreleasePool pool;
    auto shader = std::dynamic_pointer_cast<MetalShader>(description.shader);
    auto bindingLayout = std::dynamic_pointer_cast<MetalBindingLayout>(description.bindingLayout);
    if (shader == nullptr || shader->Stage() != ShaderStage::Compute ||
        (description.bindingLayout != nullptr && bindingLayout == nullptr)) {
        SetError(error, "A Metal compute pipeline requires a Metal compute shader.");
        return nullptr;
    }
    NS::Error *nativeError = nullptr;
    MTL::ComputePipelineState *pipeline =
        device_->newComputePipelineState(shader->Native(), &nativeError);
    if (pipeline == nullptr) {
        SetMetalError(error, "Metal failed to create the compute pipeline", nativeError);
        return nullptr;
    }
    return std::make_shared<MetalComputePipeline>(description, pipeline, device_.get());
}

std::shared_ptr<CommandQueue> MetalDevice::GetQueue(QueueType type)
{
    if (type != QueueType::Graphics && type != QueueType::Compute && type != QueueType::Copy) {
        return nullptr;
    }
    const std::size_t index = static_cast<std::size_t>(type);
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (queues_[index] == nullptr && nativeQueue_) {
        queues_[index] = std::make_shared<MetalQueue>(type, nativeQueue_, device_.get());
    }
    return queues_[index];
}

std::shared_ptr<Fence> MetalDevice::CreateFence(Error &)
{
    return std::make_shared<MetalFence>();
}

std::shared_ptr<MemoryAllocator> MetalDevice::GetMemoryAllocator()
{
    return allocator_;
}

MTL::Device *MetalDevice::Native() const noexcept
{
    return device_.get();
}

std::shared_ptr<Swapchain> MetalDevice::CreateSwapchain(PresentationSurface &surface,
                                                        const SwapchainDesc &description,
                                                        Error &error)
{
    AutoreleasePool pool;
    auto *metalSurface = dynamic_cast<MetalSurfaceAdapter *>(&surface);
    if (metalSurface == nullptr) {
        SetError(error, "Metal swapchains require a Metal surface adapter.");
        return nullptr;
    }
    CA::MetalLayer *layer = metalSurface->Layer();
    if (layer == nullptr || description.extent.width == 0 || description.extent.height == 0 ||
        description.extent.depth != 1 || ToMetal(description.format) == MTL::PixelFormatInvalid ||
        description.format == TextureFormat::Depth32Float || description.imageCount < 2 ||
        description.imageCount > 3) {
        SetError(error, "The Metal swapchain description or surface is invalid.");
        return nullptr;
    }
    return std::make_shared<MetalSwapchain>(layer, device_.get(), description);
}

} // namespace Metal
} // namespace Engine
