#include "Rendering/SceneRenderer.h"

#include "Core/Material.h"
#include "Graphics/BindingLayout.h"
#include "Graphics/BindingSet.h"
#include "Graphics/Buffer.h"
#include "Graphics/BufferView.h"
#include "Graphics/CommandContext.h"
#include "Graphics/Device.h"
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/RenderPassEncoder.h"
#include "Graphics/Sampler.h"
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
    std::shared_ptr<ShaderLibrary> shaderLibrary,
    Graphics::TextureFormat colorFormat,
    Error &error)
{
    if (device == nullptr || shaderLibrary == nullptr) {
        error.message = "The scene renderer requires a device and a shader library.";
        return false;
    }
    if (!m_PipelineCache.Initialize(device, std::move(shaderLibrary), error)) {
        return false;
    }
    if (!m_Samplers.Initialize(*device, error)
        || !m_CommonTextures.Initialize(*device, error)
        || !m_MaterialBinder.Initialize(device, m_CommonTextures.White(), error)) {
        m_CommonTextures.Shutdown();
        m_PipelineCache.Shutdown();
        return false;
    }
    m_Device = std::move(device);
    m_ColorFormat = colorFormat;
    return true;
}

void SceneRenderer::Shutdown() noexcept
{
    m_DrawItems.clear();
    m_Slots.clear();
    m_MaterialBinder.Shutdown();
    m_CommonTextures.Shutdown();
    m_Samplers.Shutdown();
    m_PipelineCache.Shutdown();
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

std::shared_ptr<const MaterialLayout> SceneRenderer::MaterialLayoutFor(
    const std::string &programName, Error &error)
{
    const std::shared_ptr<const ShaderProgram> program =
        m_PipelineCache.GetProgram(programName, error);
    return program == nullptr ? nullptr : program->materialLayout;
}

bool SceneRenderer::CreateSlot(
    std::shared_ptr<const ShaderProgram> program, DrawSlot &slot, Error &error)
{
    // The renderer supplies the transform constants, so the program must reflect a uniform
    // buffer for them.
    const std::optional<Graphics::BindingDesc> transformBinding =
        program->FindBinding(Graphics::BindingType::UniformBuffer);
    if (!transformBinding) {
        error.message = "Shader program '" + program->name + "' has no uniform buffer binding "
            "for the mesh transforms.";
        return false;
    }
    if (slot.buffer == nullptr) {
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
        slot.bufferView =
            m_Device->CreateBufferView(slot.buffer, 0, sizeof(MeshTransforms), error);
        if (slot.bufferView == nullptr) {
            if (error.message.empty()) {
                error.message = "Failed to create a view of the transform uniform buffer.";
            }
            return false;
        }
    }
    slot.bindings = m_Device->CreateBindingSet(program->BindingLayout(), error);
    if (slot.bindings == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the transform binding set.";
        }
        return false;
    }
    if (!slot.bindings->SetBuffer(transformBinding->binding, 0, slot.bufferView, error)) {
        if (error.message.empty()) {
            error.message = "Failed to bind the transform uniform buffer.";
        }
        return false;
    }
    // Samplers are engine owned; the shader selects one by name.
    for (const Graphics::BindingDesc &binding : program->BindingLayout(0)->Bindings()) {
        if (binding.type != Graphics::BindingType::Sampler) {
            continue;
        }
        const std::shared_ptr<Graphics::Sampler> sampler = m_Samplers.Find(binding.name);
        if (sampler == nullptr) {
            error.message = "Shader program '" + program->name + "' declares sampler '"
                + binding.name + "', which is not an engine sampler.";
            return false;
        }
        if (!slot.bindings->SetSampler(binding.binding, 0, sampler, error)) {
            if (error.message.empty()) {
                error.message = "Failed to bind engine sampler '" + binding.name + "'.";
            }
            return false;
        }
    }
    slot.program = std::move(program);
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

bool SceneRenderer::Prepare(const Scene &scene, Graphics::CommandContext &context, Error &error)
{
    if (m_Device == nullptr) {
        error.message = "The scene renderer is not initialized.";
        return false;
    }
    m_DrawItems.clear();
    m_MaterialBinder.BeginFrame();
    const Camera &camera = scene.GetCamera();
    for (const std::shared_ptr<ModelInstance> &instance : scene.Instances()) {
        const Maths::Matrix4 world = instance->WorldMatrix();
        const std::vector<ModelPart> &parts = instance->GetModel()->Parts();
        for (std::size_t partIndex = 0; partIndex < parts.size(); ++partIndex) {
            const ModelPart &part = parts[partIndex];
            if (part.mesh->VertexCount() == 0) {
                continue;
            }
            std::shared_ptr<const ShaderProgram> program =
                m_PipelineCache.GetProgram(part.material->ShaderProgram(), error);
            if (program == nullptr) {
                return false;
            }
            const std::size_t slotIndex = m_DrawItems.size();
            if (slotIndex == m_Slots.size()) {
                m_Slots.emplace_back();
            }
            DrawSlot &slot = m_Slots[slotIndex];
            if (slot.program != program && !CreateSlot(program, slot, error)) {
                return false;
            }
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
            PipelineState state;
            state.cullMode = part.material->CullMode();
            state.topology = part.material->Topology();
            state.colorFormats = {m_ColorFormat};
            item.pipeline = m_PipelineCache.GetPipeline(*program, *part.mesh, state, error);
            if (item.pipeline == nullptr) {
                return false;
            }
            item.materialBindings = m_MaterialBinder.Acquire(
                instance->GetMaterialInstance(partIndex), *program, context, error);
            if (program->materialLayout != nullptr && item.materialBindings == nullptr) {
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
        if (item.materialBindings != nullptr
            && !renderPass.SetBindingSet(MaterialLayout::kMaterialSet, item.materialBindings, error)) {
            return false;
        }
        // Bind only the streams the pipeline reads.
        for (const Graphics::VertexBufferLayoutDesc &buffer :
             item.pipeline->Description().vertexBuffers) {
            for (const VertexStream &stream : item.mesh->VertexStreams()) {
                if (stream.slot == buffer.bufferSlot
                    && !renderPass.SetVertexBuffer(stream.slot, stream.buffer, 0, error)) {
                    return false;
                }
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
