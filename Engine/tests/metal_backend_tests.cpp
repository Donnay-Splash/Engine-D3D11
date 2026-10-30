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
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/MemoryAllocation.h"
#include "Graphics/MemoryAllocator.h"
#include "Graphics/RenderPassEncoder.h"
#include "Graphics/Submission.h"
#include "Graphics/Texture.h"
#include "Graphics/TextureView.h"
#include "MetalBackend.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#ifndef METAL_SHADER_LIBRARY_PATH
#error "METAL_SHADER_LIBRARY_PATH must point to the compiled test metallib."
#endif

namespace {

bool Check(bool condition, const char *operation, const Engine::Error &error)
{
    if (condition) {
        return true;
    }
    std::cerr << operation;
    if (!error.message.empty()) {
        std::cerr << ": " << error.message;
    }
    std::cerr << '\n';
    return false;
}

bool Check(bool condition, const char *message)
{
    if (condition) {
        return true;
    }
    std::cerr << message << '\n';
    return false;
}

bool RunMetalIntegrationTests()
{
    using namespace Engine;
    using namespace Engine::Graphics;

    Error error;
    const std::shared_ptr<GraphicsInstance> instance =
        Engine::Metal::CreateMetalGraphicsInstance();
    std::vector<std::shared_ptr<Adapter>> adapters;
    if (!Check(instance->EnumerateAdapters(adapters, error), "Enumerate Metal adapters", error)
        || !Check(!adapters.empty(), "Metal reported no adapters")) {
        return false;
    }

    const std::shared_ptr<Adapter> adapter = adapters.front();
    const std::shared_ptr<Device> device = adapter->CreateDevice(error);
    if (!Check(device != nullptr, "Create Metal device", error)) {
        return false;
    }
    const std::shared_ptr<CommandQueue> queue = device->GetQueue(QueueType::Graphics);
    if (!Check(queue != nullptr, "Create Metal graphics queue")) {
        return false;
    }

    std::ifstream shaderFile(METAL_SHADER_LIBRARY_PATH, std::ios::binary);
    if (!Check(static_cast<bool>(shaderFile), "Open the compiled Metal shader library")) {
        return false;
    }
    std::vector<std::uint8_t> shaderBytecode{
        std::istreambuf_iterator<char>{shaderFile},
        std::istreambuf_iterator<char>{}};
    if (!Check(!shaderBytecode.empty(), "The compiled Metal shader library is empty")) {
        return false;
    }

    ShaderDesc vertexShaderDescription;
    vertexShaderDescription.stage = ShaderStage::Vertex;
    vertexShaderDescription.entryPoint = "mesh_vertex";
    vertexShaderDescription.format = "metallib";
    vertexShaderDescription.bytecode = shaderBytecode;
    const std::shared_ptr<Shader> vertexShader =
        device->CreateShader(vertexShaderDescription, error);
    if (!Check(vertexShader != nullptr, "Create Metal vertex shader", error)) {
        return false;
    }

    ShaderDesc fragmentShaderDescription;
    fragmentShaderDescription.stage = ShaderStage::Fragment;
    fragmentShaderDescription.entryPoint = "mesh_fragment";
    fragmentShaderDescription.format = "metallib";
    fragmentShaderDescription.bytecode = shaderBytecode;
    const std::shared_ptr<Shader> fragmentShader =
        device->CreateShader(fragmentShaderDescription, error);
    if (!Check(fragmentShader != nullptr, "Create Metal fragment shader", error)) {
        return false;
    }

    ShaderDesc computeShaderDescription;
    computeShaderDescription.stage = ShaderStage::Compute;
    computeShaderDescription.entryPoint = "fill_values";
    computeShaderDescription.format = "metallib";
    computeShaderDescription.bytecode = shaderBytecode;
    const std::shared_ptr<Shader> computeShader =
        device->CreateShader(computeShaderDescription, error);
    if (!Check(computeShader != nullptr, "Create Metal compute shader", error)) {
        return false;
    }

    const BindingLayoutDesc computeLayoutDescription{{
        {0, BindingType::StorageBuffer, 1, ShaderStage::Compute},
    }};
    const std::shared_ptr<BindingLayout> computeLayout =
        device->CreateBindingLayout(computeLayoutDescription, error);
    if (!Check(computeLayout != nullptr, "Create Metal compute binding layout", error)) {
        return false;
    }
    const std::shared_ptr<BindingSet> computeBindings =
        device->CreateBindingSet(computeLayout, error);
    if (!Check(computeBindings != nullptr, "Create Metal compute binding set", error)) {
        return false;
    }
    constexpr std::size_t computeOutputCount = 65536;
    const BufferDesc computeBufferDescription{
        computeOutputCount * sizeof(std::uint32_t),
        BufferUsage::Storage,
        MemoryType::Readback,
    };
    const std::shared_ptr<Buffer> computeBuffer =
        device->CreateBuffer(computeBufferDescription, error);
    if (!Check(computeBuffer != nullptr, "Create Metal compute buffer", error)) {
        return false;
    }
    const std::shared_ptr<BufferView> computeBufferView =
        device->CreateBufferView(
            computeBuffer, 0, computeBufferDescription.size, error);
    if (!Check(computeBufferView != nullptr, "Create Metal compute buffer view", error)
        || !Check(computeBindings->SetBuffer(0, 0, computeBufferView, error),
            "Set Metal compute buffer", error)) {
        return false;
    }
    const std::shared_ptr<ComputePipeline> computePipeline =
        device->CreateComputePipeline({computeShader, computeLayout}, error);
    if (!Check(computePipeline != nullptr, "Create Metal compute pipeline", error)) {
        return false;
    }
    const std::shared_ptr<CommandQueue> computeQueue =
        device->GetQueue(QueueType::Compute);
    const std::shared_ptr<CommandContext> computeContext =
        computeQueue == nullptr ? nullptr : computeQueue->CreateContext(error);
    if (!Check(computeContext != nullptr, "Create Metal compute context", error)) {
        return false;
    }
    std::unique_ptr<ComputeEncoder> computeEncoder =
        computeContext->BeginCompute(error);
    if (!Check(computeEncoder != nullptr, "Begin Metal compute encoder", error)
        || !Check(computeEncoder->SetPipeline(computePipeline, error),
            "Set Metal compute pipeline", error)
        || !Check(computeEncoder->SetBindingSet(0, computeBindings, error),
            "Set Metal compute binding set", error)) {
        return false;
    }
    computeEncoder->Dispatch(1, 1, 1);
    if (!Check(computeEncoder->End(error), "End Metal compute encoder", error)) {
        return false;
    }
    const std::shared_ptr<CommandBuffer> computeCommands = computeContext->Finish(error);
    if (!Check(computeCommands != nullptr, "Finish Metal compute commands", error)) {
        return false;
    }
    const std::shared_ptr<Submission> computeSubmission =
        computeQueue->Submit(computeCommands, {}, {}, error);
    if (!Check(computeSubmission != nullptr, "Submit Metal compute commands", error)
        || !Check(computeSubmission->Wait(error), "Wait for Metal compute submission", error)) {
        return false;
    }
    void *mappedComputeData = nullptr;
    if (!Check(
            computeBuffer->Map(mappedComputeData, error),
            "Map Metal compute output",
            error)) {
        return false;
    }
    const auto *computeValues = static_cast<const std::uint32_t *>(mappedComputeData);
    const bool computeOutputMatches = std::equal(
        computeValues, computeValues + 8,
        std::array<std::uint32_t, 8>{{1, 2, 3, 4, 5, 6, 7, 8}}.begin());
    computeBuffer->Unmap();
    if (!Check(computeOutputMatches, "Metal compute output was not written")) {
        return false;
    }

    const BindingLayoutDesc bindingLayoutDescription{{
        {0, BindingType::UniformBuffer, 1, ShaderStage::Vertex},
    }};
    const std::shared_ptr<BindingLayout> bindingLayout =
        device->CreateBindingLayout(bindingLayoutDescription, error);
    if (!Check(bindingLayout != nullptr, "Create Metal binding layout", error)) {
        return false;
    }
    const std::shared_ptr<BindingSet> bindingSet =
        device->CreateBindingSet(bindingLayout, error);
    if (!Check(bindingSet != nullptr, "Create Metal binding set", error)) {
        return false;
    }

    struct Vertex
    {
        float position[3];
        float uv[2];
    };
    const std::array<Vertex, 3> vertices{{
        {{-0.8f, -0.8f, 0.0f}, {0.0f, 0.0f}},
        {{0.8f, -0.8f, 0.0f}, {1.0f, 0.0f}},
        {{0.0f, 0.8f, 0.0f}, {0.5f, 1.0f}},
    }};
    const BufferDesc vertexBufferDescription{
        sizeof(vertices), BufferUsage::Vertex, MemoryType::Upload};
    const std::shared_ptr<Buffer> vertexBuffer =
        device->CreateBuffer(vertexBufferDescription, error);
    if (!Check(vertexBuffer != nullptr, "Create Metal vertex buffer", error)) {
        return false;
    }
    void *mappedVertexData = nullptr;
    if (!Check(vertexBuffer->Map(mappedVertexData, error), "Map Metal vertex buffer", error)) {
        return false;
    }
    std::memcpy(mappedVertexData, vertices.data(), sizeof(vertices));
    vertexBuffer->Unmap();

    struct alignas(16) Matrix
    {
        float values[16];
    };
    struct alignas(16) Uniforms
    {
        Matrix world;
        Matrix view;
        Matrix projection;
    };
    Uniforms uniforms{};
    for (Matrix *matrix : {&uniforms.world, &uniforms.view, &uniforms.projection}) {
        matrix->values[0] = 1.0f;
        matrix->values[5] = 1.0f;
        matrix->values[10] = 1.0f;
        matrix->values[15] = 1.0f;
    }
    const BufferDesc uniformBufferDescription{
        sizeof(uniforms), BufferUsage::Uniform, MemoryType::Upload};
    const std::shared_ptr<Buffer> uniformBuffer =
        device->CreateBuffer(uniformBufferDescription, error);
    if (!Check(uniformBuffer != nullptr, "Create Metal uniform buffer", error)) {
        return false;
    }
    void *mappedUniformData = nullptr;
    if (!Check(uniformBuffer->Map(mappedUniformData, error), "Map Metal uniform buffer", error)) {
        return false;
    }
    std::memcpy(mappedUniformData, &uniforms, sizeof(uniforms));
    uniformBuffer->Unmap();

    const std::shared_ptr<BufferView> uniformView =
        device->CreateBufferView(uniformBuffer, 0, sizeof(uniforms), error);
    if (!Check(uniformView != nullptr, "Create Metal uniform buffer view", error)
        || !Check(bindingSet->SetBuffer(0, 0, uniformView, error),
            "Set Metal uniform binding", error)) {
        return false;
    }

    GraphicsPipelineDesc pipelineDescription;
    pipelineDescription.shaders = {vertexShader, fragmentShader};
    pipelineDescription.bindingLayout = bindingLayout;
    pipelineDescription.vertexAttributes = {
        {0, 0, VertexFormat::Float3, 0},
        {1, 0, VertexFormat::Float2, 12},
    };
    pipelineDescription.vertexBuffers = {{0, sizeof(Vertex)}};
    pipelineDescription.cullMode = CullMode::None;
    pipelineDescription.colorFormats = {TextureFormat::RGBA8Unorm};
    const std::shared_ptr<GraphicsPipeline> pipeline =
        device->CreateGraphicsPipeline(pipelineDescription, error);
    if (!Check(pipeline != nullptr, "Create Metal graphics pipeline", error)) {
        return false;
    }

    const TextureDesc renderTargetDescription{
        TextureFormat::RGBA8Unorm,
        {16, 16, 1},
        1,
        1,
        1,
        TextureUsage::ColorAttachment | TextureUsage::CopySource,
        MemoryType::DeviceLocal,
    };
    const std::shared_ptr<Texture> renderTarget =
        device->CreateTexture(renderTargetDescription, error);
    if (!Check(renderTarget != nullptr, "Create Metal render target", error)) {
        return false;
    }
    const std::shared_ptr<TextureView> renderTargetView =
        device->CreateTextureView(renderTarget, ResourceRange{}, TextureFormat::Unknown, error);
    if (!Check(renderTargetView != nullptr, "Create Metal render target view", error)) {
        return false;
    }

    const MemoryRequirements allocationRequirements =
        device->GetMemoryAllocator()->GetRequirements(renderTargetDescription);
    const std::shared_ptr<MemoryAllocation> allocation =
        device->GetMemoryAllocator()->Allocate(
            allocationRequirements, MemoryType::DeviceLocal, error);
    if (!Check(allocation != nullptr, "Allocate Metal memory", error)
        || !Check(allocation->Size() >= allocationRequirements.size,
            "The Metal allocation is smaller than its requirements")) {
        return false;
    }

    RenderPassDesc renderPass;
    renderPass.colorAttachments.push_back({});
    renderPass.colorAttachments[0].view = renderTargetView;
    renderPass.colorAttachments[0].loadAction = LoadAction::Clear;
    renderPass.extent = {16, 16, 1};
    const std::shared_ptr<CommandContext> renderContext =
        queue->CreateContext(error);
    if (!Check(renderContext != nullptr, "Create Metal render context", error)) {
        return false;
    }
    std::unique_ptr<RenderPassEncoder> renderEncoder =
        renderContext->BeginRenderPass(renderPass, error);
    if (!Check(renderEncoder != nullptr, "Begin Metal render pass", error)
        || !Check(renderEncoder->SetPipeline(pipeline, error),
            "Set Metal graphics pipeline", error)
        || !Check(renderEncoder->SetBindingSet(0, bindingSet, error),
            "Set Metal graphics binding set", error)
        || !Check(renderEncoder->SetVertexBuffer(0, vertexBuffer, 0, error),
            "Set Metal vertex buffer", error)
        || !Check(renderEncoder->SetViewport({0, 0, 16, 16, 0, 1}, error),
            "Set Metal viewport", error)) {
        return false;
    }
    renderEncoder->Draw(3, 1, 0, 0);
    if (!Check(renderEncoder->End(error), "End Metal render pass", error)) {
        return false;
    }
    const std::shared_ptr<CommandBuffer> renderCommands = renderContext->Finish(error);
    if (!Check(renderCommands != nullptr, "Finish Metal render commands", error)) {
        return false;
    }
    const std::shared_ptr<Fence> renderFence = device->CreateFence(error);
    if (!Check(renderFence != nullptr, "Create Metal render fence", error)) {
        return false;
    }
    const std::shared_ptr<Submission> renderSubmission =
        queue->Submit(renderCommands, {}, {renderFence}, error);
    if (!Check(renderSubmission != nullptr, "Submit Metal render commands", error)) {
        return false;
    }

    constexpr std::array<std::uint32_t, 4> expectedCopy{{3, 5, 7, 11}};
    const BufferDesc sourceDescription{
        sizeof(expectedCopy), BufferUsage::CopySource, MemoryType::Upload};
    const BufferDesc destinationDescription{
        sizeof(expectedCopy), BufferUsage::CopyDestination, MemoryType::Readback};
    const std::shared_ptr<Buffer> sourceBuffer =
        device->CreateBuffer(sourceDescription, error);
    const std::shared_ptr<Buffer> destinationBuffer =
        device->CreateBuffer(destinationDescription, error);
    if (!Check(sourceBuffer != nullptr, "Create Metal copy source", error)
        || !Check(destinationBuffer != nullptr, "Create Metal copy destination", error)) {
        return false;
    }
    void *mappedCopySource = nullptr;
    if (!Check(sourceBuffer->Map(mappedCopySource, error), "Map Metal copy source", error)) {
        return false;
    }
    std::memcpy(mappedCopySource, expectedCopy.data(), sizeof(expectedCopy));
    sourceBuffer->Unmap();

    const std::shared_ptr<CommandContext> copyContext =
        queue->CreateContext(error);
    if (!Check(copyContext != nullptr, "Create Metal copy context", error)) {
        return false;
    }
    std::unique_ptr<CopyEncoder> copyEncoder = copyContext->BeginCopy(error);
    if (!Check(copyEncoder != nullptr, "Begin Metal copy encoder", error)
        || !Check(copyEncoder->CopyBuffer(
                sourceBuffer, 0, destinationBuffer, 0, sizeof(expectedCopy), error),
            "Record Metal buffer copy", error)
        || !Check(copyEncoder->End(error), "End Metal copy encoder", error)) {
        return false;
    }
    const std::shared_ptr<CommandBuffer> copyCommands = copyContext->Finish(error);
    if (!Check(copyCommands != nullptr, "Finish Metal copy commands", error)) {
        return false;
    }
    const std::shared_ptr<Fence> copyFence = device->CreateFence(error);
    if (!Check(copyFence != nullptr, "Create Metal copy fence", error)) {
        return false;
    }
    const std::shared_ptr<Submission> copySubmission =
        queue->Submit(copyCommands, {renderSubmission}, {copyFence}, error);
    if (!Check(copySubmission != nullptr, "Submit Metal copy commands", error)
        || !Check(copySubmission->Wait(error), "Wait for Metal copy submission", error)
        || !Check(copyFence->Wait(error), "Wait for Metal copy fence", error)
        || !Check(copyFence->IsSignaled(), "The Metal copy fence was not signaled")) {
        return false;
    }

    void *mappedCopyDestination = nullptr;
    if (!Check(
            destinationBuffer->Map(mappedCopyDestination, error),
            "Map Metal copy destination",
            error)) {
        return false;
    }
    const bool copyMatches =
        std::memcmp(mappedCopyDestination, expectedCopy.data(), sizeof(expectedCopy)) == 0;
    destinationBuffer->Unmap();
    return Check(copyMatches, "Metal buffer copy returned incorrect data");
}

} // namespace

int main()
{
    return RunMetalIntegrationTests() ? 0 : 1;
}
