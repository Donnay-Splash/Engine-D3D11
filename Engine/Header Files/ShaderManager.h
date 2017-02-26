#pragma once

#include <Resources\ShaderPipeline.h>

namespace Engine
{
    // TODO: Come up with a better sounding name
    enum class ShaderName
    {
        Uber = 0,
        DeepGBuffer_Gen = 1,
        GBuffer_Shade = 2,
        DeepGBuffer_csZCopy = 3,
        Generate_HiZ = 4
    };

    class ShaderManager
    {
    public:
        using Ptr = std::shared_ptr<ShaderManager>;

        // TODO: eventually make shader manager a friend of the Renderer class
        // so that it can set this up without having to pass too much stuff around.
        ShaderManager(ID3D11Device* device);

        ShaderPipeline::Ptr GetShaderPipeline(ShaderName);
    private:
        using ShaderMap = std::map<ShaderName, ShaderPipeline::Ptr>;
        using ShaderMapObject = ShaderMap::value_type;
        void LoadCoreShaders(ID3D11Device* device);

    private:
        ShaderMap m_shaderMap;
    };
}

