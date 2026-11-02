#include "Rendering/SceneRenderer.h"

#include "Graphics/BindingLayout.h"
#include "Graphics/BindingSet.h"
#include "Graphics/Buffer.h"
#include "Graphics/BufferView.h"
#include "Graphics/CommandContext.h"
#include "Graphics/Device.h"
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/RenderPassEncoder.h"
#include "Graphics/Shader.h"
#include "Graphics/Submission.h"

#include <algorithm>
#include <cstring>

namespace Engine {

SceneRenderer::~SceneRenderer()
{
    Shutdown();
}

bool SceneRenderer::Initialize(
    std::shared_ptr<Graphics::Device> device,
    GraphicsShaderPair shaders,
    Graphics::TextureFormat colorFormat,
    Error &error)
{
    if (device == nullptr || shaders.vertex == nullptr || shaders.fragment == nullptr) {
        error.message = "The scene renderer requires a device and a shader pair.";
        return false;
    }
    Graphics::BindingLayoutDesc layoutDescription;
    layoutDescription.bindings.push_back({
        0,
        Graphics::BindingType::UniformBuffer,
        1,
        Graphics::ShaderStage::Vertex,
    });
    std::shared_ptr<Graphics::BindingLayout> layout =
        device->CreateBindingLayout(layoutDescription, error);
    if (layout == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the transform binding layout.";
        }
        return false;
    }
    m_Device = std::move(device);
    m_Shaders = std::move(shaders);
    m_ColorFormat = colorFormat;
    m_BindingLayout = std::move(layout);
    return GetPipeline({Graphics::CullMode::None, Graphics::PrimitiveTopology::TriangleList}, error)
        != nullptr;
}

void SceneRenderer::Shutdown() noexcept
{
    m_DrawItems.clear();
    m_Slots.clear();
    m_Pipelines.clear();
    m_BindingLayout.reset();
    m_Shaders = GraphicsShaderPair();
    m_Device.reset();
}

bool SceneRenderer::CollectPendingUploads(
    const Scene &scene,
    std::vector<std::shared_ptr<Graphics::Submission>> &waitFor,
    Error &error)
{
    std::vector<Mesh *> visited;
    for (const std::shared_ptr<ModelInstance> &instance : scene.Instances()) {
        for (const ModelPart &part : instance->GetModel()->Parts()) {
            if (std::find(visited.begin(), visited.end(), part.mesh.get()) != visited.end()) {
                continue;
            }
            visited.push_back(part.mesh.get());
            if (!part.mesh->CollectPendingUploads(waitFor, error)) {
                return false;
            }
        }
    }
    return true;
}

bool SceneRenderer::CreateSlot(Error &error)
{
    DrawSlot slot;
    Graphics::BufferDesc description;
    description.size = sizeof(MeshTransforms);
    description.usage = Graphics::BufferUsage::Uniform;
    description.memoryType = Graphics::MemoryType::Upload;
    slot.buffer = m_Device->CreateBuffer(description, error);
    if (slot.buffer == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the transform uniform buffer.";
        }
        return false;
    }
    slot.bufferView = m_Device->CreateBufferView(slot.buffer, 0, sizeof(MeshTransforms), error);
    if (slot.bufferView == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create a view of the transform uniform buffer.";
        }
        return false;
    }
    slot.bindings = m_Device->CreateBindingSet(m_BindingLayout, error);
    if (slot.bindings == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the transform binding set.";
        }
        return false;
    }
    if (!slot.bindings->SetBuffer(0, 0, slot.bufferView, error)) {
        if (error.message.empty()) {
            error.message = "Failed to bind the transform uniform buffer.";
        }
        return false;
    }
    m_Slots.push_back(std::move(slot));
    return true;
}

bool SceneRenderer::WriteSlot(DrawSlot &slot, const MeshTransforms &transforms, Error &error)
{
    // Constants are rewritten only when they change, so a buffer an earlier frame may
    // still be reading is never overwritten for a static scene.
    if (slot.written && std::memcmp(&slot.contents, &transforms, sizeof(transforms)) == 0) {
        return true;
    }
    void *mapped = nullptr;
    if (!slot.buffer->Map(mapped, error)) {
        return false;
    }
    if (mapped == nullptr) {
        slot.buffer->Unmap();
        error.message = "The transform uniform buffer returned no mapped memory.";
        return false;
    }
    std::memcpy(mapped, &transforms, sizeof(transforms));
    slot.buffer->Unmap();
    slot.contents = transforms;
    slot.written = true;
    return true;
}

std::shared_ptr<Graphics::GraphicsPipeline> SceneRenderer::GetPipeline(
    const PipelineKey &key, Error &error)
{
    const auto found = m_Pipelines.find(key);
    if (found != m_Pipelines.end()) {
        return found->second;
    }
    Graphics::GraphicsPipelineDesc description;
    description.shaders = {m_Shaders.vertex, m_Shaders.fragment};
    description.bindingLayout = m_BindingLayout;
    description.vertexAttributes = {
        {0, 0, Graphics::VertexFormat::Float3, 0},
        {1, 1, Graphics::VertexFormat::Float2, 0},
    };
    description.vertexBuffers = {
        {0, static_cast<std::uint32_t>(sizeof(float) * 3)},
        {1, static_cast<std::uint32_t>(sizeof(float) * 2)},
    };
    description.topology = key.second;
    description.cullMode = key.first;
    description.colorFormats = {m_ColorFormat};
    std::shared_ptr<Graphics::GraphicsPipeline> pipeline =
        m_Device->CreateGraphicsPipeline(description, error);
    if (pipeline == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the mesh graphics pipeline.";
        }
        return nullptr;
    }
    m_Pipelines.emplace(key, pipeline);
    return pipeline;
}

bool SceneRenderer::Prepare(const Scene &scene, Graphics::CommandContext &context, Error &error)
{
    if (m_Device == nullptr) {
        error.message = "The scene renderer is not initialized.";
        return false;
    }
    m_DrawItems.clear();
    const Camera &camera = scene.GetCamera();
    for (const std::shared_ptr<ModelInstance> &instance : scene.Instances()) {
        const Maths::Matrix4 world = instance->WorldMatrix();
        for (const ModelPart &part : instance->GetModel()->Parts()) {
            if (part.mesh->VertexCount() == 0) {
                continue;
            }
            const std::size_t slotIndex = m_DrawItems.size();
            if (slotIndex == m_Slots.size() && !CreateSlot(error)) {
                return false;
            }
            DrawSlot &slot = m_Slots[slotIndex];
            MeshTransforms transforms;
            transforms.world = world;
            transforms.view = camera.view;
            transforms.projection = camera.projection;
            if (!WriteSlot(slot, transforms, error)) {
                return false;
            }
            if (!slot.prepared) {
                Graphics::ResourceBarrierDesc barrier;
                barrier.before = Graphics::ResourceUsage::Undefined;
                barrier.after = Graphics::ResourceUsage::UniformBuffer;
                if (!context.Barrier(slot.buffer, barrier, error)) {
                    if (error.message.empty()) {
                        error.message = "Failed to prepare the transform uniform buffer.";
                    }
                    return false;
                }
            }
            DrawItem item;
            item.mesh = part.mesh;
            item.pipeline = GetPipeline({part.material->CullMode(), part.material->Topology()}, error);
            if (item.pipeline == nullptr) {
                return false;
            }
            item.slot = slotIndex;
            m_DrawItems.push_back(std::move(item));
        }
    }
    return true;
}

bool SceneRenderer::Record(Graphics::RenderPassEncoder &renderPass, Error &error)
{
    const Graphics::GraphicsPipeline *boundPipeline = nullptr;
    for (const DrawItem &item : m_DrawItems) {
        if (item.pipeline.get() != boundPipeline) {
            if (!renderPass.SetPipeline(item.pipeline, error)) {
                return false;
            }
            boundPipeline = item.pipeline.get();
        }
        if (!renderPass.SetBindingSet(0, m_Slots[item.slot].bindings, error)) {
            return false;
        }
        for (const VertexStream &stream : item.mesh->VertexStreams()) {
            if (!renderPass.SetVertexBuffer(stream.slot, stream.buffer, 0, error)) {
                return false;
            }
        }
        renderPass.Draw(item.mesh->VertexCount());
    }
    return true;
}

void SceneRenderer::MarkSubmitted() noexcept
{
    for (const DrawItem &item : m_DrawItems) {
        m_Slots[item.slot].prepared = true;
    }
}

} // namespace Engine
