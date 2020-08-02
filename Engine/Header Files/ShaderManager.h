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
        Minify_CSZ = 4,
        AO = 5,
        BilateralBlur = 6,
        BilateralBlurPacked = 7,
        TSAA = 8,
        LambertianOnly = 9,
        Minify_Lambertian = 10,
        ComputeRadiosity = 11,
        BasicTemporalFilter = 12,
        Tonemap = 13,
        ShadowMapping = 14,
		PassThrough = 15,
        ImGui = 16
    };

    class ShaderManager
    {
    public:
        using Ptr = std::shared_ptr<ShaderManager>;

        // TODO: eventually make shader manager a friend of the Renderer class
        // so that it can set this up without having to pass too much stuff around.
        ShaderManager();

        ShaderPipeline::Ptr GetShaderPipeline(ShaderName);
    private:
        using ShaderMap = std::map<ShaderName, ShaderPipeline::Ptr>;
        using ShaderMapObject = ShaderMap::value_type;
        void LoadCoreShaders();

    private:
        ShaderMap m_shaderMap;
    };
}

