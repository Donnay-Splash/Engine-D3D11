#include "Rendering/PipelineCache.h"

#include "Graphics/Device.h"

#include <algorithm>
#include <utility>

namespace Engine {
namespace {

bool loadStage(
    ShaderLibrary &library,
    Graphics::Device &device,
    const ShaderRef &ref,
    Graphics::ShaderStage stage,
    LoadedShader &loaded,
    Error &error)
{
    if (!library.RequestShader(device, ref, stage, loaded, error)) {
        if (error.message.empty()) {
            error.message = "Failed to load shader " + ref.source + ":" + ref.entry + ".";
        }
        return false;
    }
    if (loaded.shader == nullptr || loaded.shader->Stage() != stage
        || loaded.reflection.stage != stage) {
        error.message = "The shader library returned an invalid shader for " + ref.source + ":"
            + ref.entry + ".";
        return false;
    }
    return true;
}

// Merges the bindings of every stage; a binding used by several stages is visible to all.
bool mergeBindings(
    const std::vector<const Graphics::ShaderReflection *> &stages,
    std::vector<Graphics::BindingDesc> &merged,
    Error &error)
{
    for (const Graphics::ShaderReflection *stage : stages) {
        for (const Graphics::BindingDesc &binding : stage->bindings) {
            const auto existing = std::find_if(
                merged.begin(), merged.end(),
                [&binding](const Graphics::BindingDesc &candidate) {
                    return candidate.binding == binding.binding;
                });
            if (existing == merged.end()) {
                merged.push_back(binding);
            } else if (existing->type != binding.type
                       || existing->arrayCount != binding.arrayCount) {
                error.message = "Shader stages declare conflicting resources for binding "
                    + std::to_string(binding.binding) + ".";
                return false;
            } else {
                existing->visibility = existing->visibility | binding.visibility;
            }
        }
    }
    return true;
}

} // namespace

bool PipelineCache::Initialize(
    std::shared_ptr<Graphics::Device> device,
    std::shared_ptr<ShaderLibrary> shaderLibrary,
    Error &error)
{
    if (device == nullptr || shaderLibrary == nullptr) {
        error.message = "The pipeline cache requires a device and a shader library.";
        return false;
    }
    std::string configText;
    if (!shaderLibrary->LoadProgramConfig(configText, error)) {
        if (error.message.empty()) {
            error.message = "Failed to load the shader program config.";
        }
        return false;
    }
    ShaderProgramCatalog catalog;
    if (!ShaderProgramCatalog::Parse(configText, catalog, error)) {
        return false;
    }
    m_Catalog = std::move(catalog);
    m_Device = std::move(device);
    m_ShaderLibrary = std::move(shaderLibrary);
    return true;
}

void PipelineCache::Shutdown() noexcept
{
    m_Pipelines.clear();
    m_Programs.clear();
    m_Catalog = ShaderProgramCatalog();
    m_ShaderLibrary.reset();
    m_Device.reset();
}

std::shared_ptr<const ShaderProgram> PipelineCache::GetProgram(
    const std::string &name, Error &error)
{
    if (m_Device == nullptr) {
        error.message = "The pipeline cache is not initialized.";
        return nullptr;
    }
    const auto found = m_Programs.find(name);
    if (found != m_Programs.end()) {
        return found->second;
    }
    const ShaderProgramDesc *desc = m_Catalog.Find(name);
    if (desc == nullptr) {
        error.message = "Unknown shader program '" + name + "'.";
        return nullptr;
    }
    std::shared_ptr<ShaderProgram> program = ResolveProgram(*desc, error);
    if (program != nullptr) {
        m_Programs.emplace(name, program);
    }
    return program;
}

std::shared_ptr<ShaderProgram> PipelineCache::ResolveProgram(
    const ShaderProgramDesc &desc, Error &error)
{
    if (!desc.vertex) {
        error.message = "Shader program '" + desc.name + "' is not a graphics program.";
        return nullptr;
    }
    LoadedShader vertex;
    LoadedShader fragment;
    if (!loadStage(*m_ShaderLibrary, *m_Device, *desc.vertex,
            Graphics::ShaderStage::Vertex, vertex, error)) {
        return nullptr;
    }
    if (desc.fragment
        && !loadStage(*m_ShaderLibrary, *m_Device, *desc.fragment,
            Graphics::ShaderStage::Fragment, fragment, error)) {
        return nullptr;
    }

    std::vector<const Graphics::ShaderReflection *> stages = {&vertex.reflection};
    if (desc.fragment) {
        stages.push_back(&fragment.reflection);
    }
    Graphics::BindingLayoutDesc layoutDescription;
    if (!mergeBindings(stages, layoutDescription.bindings, error)) {
        return nullptr;
    }

    auto program = std::make_shared<ShaderProgram>();
    program->name = desc.name;
    program->vertexInputs = vertex.reflection.vertexInputs;
    program->pipelineTemplate.shaders = {vertex.shader};
    if (desc.fragment) {
        program->pipelineTemplate.shaders.push_back(fragment.shader);
    }
    if (!layoutDescription.bindings.empty()) {
        program->pipelineTemplate.bindingLayout =
            m_Device->CreateBindingLayout(layoutDescription, error);
        if (program->pipelineTemplate.bindingLayout == nullptr) {
            if (error.message.empty()) {
                error.message = "Failed to create the binding layout for program '"
                    + desc.name + "'.";
            }
            return nullptr;
        }
    }
    return program;
}

std::shared_ptr<Graphics::GraphicsPipeline> PipelineCache::GetPipeline(
    const ShaderProgram &program,
    const Mesh &mesh,
    const PipelineState &state,
    Error &error)
{
    if (m_Device == nullptr) {
        error.message = "The pipeline cache is not initialized.";
        return nullptr;
    }

    Graphics::GraphicsPipelineDesc description = program.pipelineTemplate;
    for (const Graphics::ShaderVertexInput &input : program.vertexInputs) {
        const VertexStream *stream = mesh.FindStream(input.semantic);
        if (stream == nullptr) {
            error.message = "The mesh has no vertex stream for " + input.semantic
                + ", which program '" + program.name + "' requires.";
            return nullptr;
        }
        if (stream->format != input.format) {
            error.message = "The mesh vertex stream for " + input.semantic
                + " does not match the format program '" + program.name + "' expects.";
            return nullptr;
        }
        description.vertexAttributes.push_back({input.location, stream->slot, stream->format, 0});
        description.vertexBuffers.push_back({stream->slot, stream->stride});
    }
    description.topology = state.topology;
    description.cullMode = state.cullMode;
    description.colorFormats = state.colorFormats;
    description.depthStencilFormat = state.depthStencilFormat;
    description.sampleCount = state.sampleCount;

    PipelineKey key{
        program.name,
        {},
        state.cullMode,
        state.topology,
        state.colorFormats,
        state.depthStencilFormat,
        state.sampleCount,
    };
    for (const Graphics::VertexAttributeDesc &attribute : description.vertexAttributes) {
        key.vertexLayout.insert(
            key.vertexLayout.end(),
            {attribute.location, attribute.bufferSlot,
             static_cast<std::uint32_t>(attribute.format), attribute.offset});
    }
    for (const Graphics::VertexBufferLayoutDesc &buffer : description.vertexBuffers) {
        key.vertexLayout.insert(key.vertexLayout.end(), {buffer.bufferSlot, buffer.stride});
    }

    const auto found = m_Pipelines.find(key);
    if (found != m_Pipelines.end()) {
        return found->second;
    }
    std::shared_ptr<Graphics::GraphicsPipeline> pipeline =
        m_Device->CreateGraphicsPipeline(description, error);
    if (pipeline == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the graphics pipeline for program '"
                + program.name + "'.";
        }
        return nullptr;
    }
    m_Pipelines.emplace(std::move(key), pipeline);
    return pipeline;
}

} // namespace Engine
