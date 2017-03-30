#include "pch.h"
#include "ShaderManager.h"
#include <Resources\ShaderPipeline.h>
#include <Resources\InputLayout.h>

namespace Engine
{
    // Standard forward shader pipeline
    namespace UberShader
    {
        #include "Shaders\Compiled shaders\Uber.ps.hlsl.h"
        #include "Shaders\Compiled shaders\Uber.vs.hlsl.h"
    }

    // Deep G-Buffer generation pipeline
    namespace DeepGBuffer
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_Gen.ps.hlsl.h"
        #include "Shaders\Compiled shaders\DeepGBuffer_Gen.gs.hlsl.h"
        #include "Shaders\Compiled shaders\DeepGBuffer_Gen.vs.hlsl.h"
    }

    // ShadowMapping shaders
    namespace ShadowMapping
    {
        #include "Shaders\Compiled shaders\ShadowMap.vs.hlsl.h"
    }

    // Full screen quad vertex shader
    namespace FullScreenQuad
    {
        #include "Shaders\Compiled shaders\FullscreenQuad.vs.hlsl.h"
    }

    // Final fullscreen shader to resolve shader G-Buffer
    namespace Deferred
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_Shade.ps.hlsl.h"
    }

    // Copy the two layer camera space Z into two channels of a single texture
    namespace csZCopy
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_CameraZCopy.ps.hlsl.h"
    }

    // Generate the hierarchical Z buffer
    namespace Minify_CSZ
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_Minify_CSZ.ps.hlsl.h"
    }

    namespace Minify_Lambertian
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_Minify_Lambertian.ps.hlsl.h"
    }

    // Scalable Ambient Obscurance algorithm
    namespace AO
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_ComputeAO.ps.hlsl.h"
    }

    // Depth aware blur for reducing noise in AO and radiosity. 
    // Sampled depth from a texture
    namespace BilateralBlurSampled
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_DepthAwareBlur.ps.hlsl.h"
    }


    // Depth aware blur for reducing noise in AO and radiosity. 
    // Works with packed bilateral key in alpha
    namespace BilateralBlurPacked
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_DepthAwareBlur_Packed.ps.hlsl.h"
    }

    // Shader for converging temporal supersampling
    namespace TSAA
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_TSAA.ps.hlsl.h"
    }

    // A basic temporal filter for accumulating effects such as SSR, AO, Radiosity
    namespace BasicTemporalFilter
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_BasicTemporalFilter.ps.hlsl.h"
    }

    // Lambertian only shader
    namespace LambertianOnly
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_LambertianOnly.ps.hlsl.h"
    }

    namespace ComputeRadiosity
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_ComputeRadiosity.ps.hlsl.h"
    }

    // Transforms HDR data to LDR using filmic tonemap
    namespace Tonemap
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_TonemapAndExposure.ps.hlsl.h"
    }

    namespace DoF_Split
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_DoF_SplitScene.ps.hlsl.h"
    }

    namespace DoF_Blur_Vertical
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_DoF_Blur.ps.hlsl.h"
    }

    namespace DoF_Blur_Horizontal
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_DoF_Blur_Horizontal.ps.hlsl.h"
    }

    namespace DoF_Composite
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_DoF_Composite.ps.hlsl.h"
    }

    ShaderManager::ShaderManager(ID3D11Device* device)
    {
        LoadCoreShaders(device);
    }

    void ShaderManager::LoadCoreShaders(ID3D11Device* device)
    {
        // Here we create the D3D11 shader objects to be placed into the bundle.
        // At this time we will also create the input layouts for each shader bundle.

        // Load the Standard forward shaders
        {

            InputLayout::Ptr layout = std::make_shared<InputLayout>(InputElement::Position | InputElement::Normal0 | InputElement::TexCoord0);
            Shader::Ptr vertexShader = std::make_shared<Shader>(Shader::Type::Vertex, UberShader::g_VSMain, sizeof(UberShader::g_VSMain), device);
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, UberShader::g_PSMain, sizeof(UberShader::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, layout, vertexShader, pixelShader);

            m_shaderMap.insert(ShaderMapObject(ShaderName::Uber, shaderPipeline));
        }

        // Load deep G-Buffer generation shaders
        {
            InputLayout::Ptr layout = std::make_shared<InputLayout>(InputElement::Position | InputElement::Normal0 | InputElement::TexCoord0 | InputElement::Tangents);
            Shader::Ptr vertexShader = std::make_shared<Shader>(Shader::Type::Vertex, DeepGBuffer::g_VSMain, sizeof(DeepGBuffer::g_VSMain), device);
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, DeepGBuffer::g_PSMain, sizeof(DeepGBuffer::g_PSMain), device);
            Shader::Ptr geometryShader = std::make_shared<Shader>(Shader::Type::Geometry, DeepGBuffer::g_GSMain, sizeof(DeepGBuffer::g_GSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, layout, vertexShader, pixelShader, geometryShader);

            m_shaderMap.insert(ShaderMapObject(ShaderName::DeepGBuffer_Gen, shaderPipeline));
        }
            
        // Load Shadow map shader
        {
            // Only position is required for shadow mapping
            InputLayout::Ptr layout = std::make_shared<InputLayout>(InputElement::Position);
            Shader::Ptr vertexShader = std::make_shared<Shader>(Shader::Type::Vertex, ShadowMapping::g_VSMain, sizeof(ShadowMapping::g_VSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, layout, vertexShader, nullptr, nullptr);

            m_shaderMap.insert(ShaderMapObject(ShaderName::ShadowMapping, shaderPipeline));
        }

        // Load post process shaders
        InputLayout::Ptr fullscreenQuadLayout = std::make_shared<InputLayout>(InputElement::Position | InputElement::TexCoord0);
        Shader::Ptr fullScreenQuadVS = std::make_shared<Shader>(Shader::Type::Vertex, FullScreenQuad::g_VSMain, sizeof(FullScreenQuad::g_VSMain), device);

        // Load the post process deferred shaders
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, Deferred::g_PSMain, sizeof(Deferred::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::GBuffer_Shade, shaderPipeline);
        }

        // Load the camera-space Z copy
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, csZCopy::g_PSMain, sizeof(csZCopy::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::DeepGBuffer_csZCopy, shaderPipeline);
        }

        // Load the Hi_Z generation
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, Minify_CSZ::g_PSMain, sizeof(Minify_CSZ::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::Minify_CSZ, shaderPipeline);
        }

        // Load Lambertian minify. This is in preparation for use in the radiosity calculation
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, Minify_Lambertian::g_PSMain, sizeof(Minify_Lambertian::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::Minify_Lambertian, shaderPipeline);
        }

        // Load the AO shader
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, AO::g_PSMain, sizeof(AO::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::AO, shaderPipeline);
        }

        // Load the Bilateral Blur shader with sampled keys
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, BilateralBlurSampled::g_PSMain, sizeof(BilateralBlurSampled::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::BilateralBlur, shaderPipeline);
        }

        // Load the Bilateral Blur shader With packed keys
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, BilateralBlurPacked::g_PSMain, sizeof(BilateralBlurPacked::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::BilateralBlurPacked, shaderPipeline);
        }

        // Load the TSAA shader
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, TSAA::g_PSMain, sizeof(TSAA::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::TSAA, shaderPipeline);
        }

        // Load the Basic tempral filter
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, BasicTemporalFilter::g_PSMain, sizeof(BasicTemporalFilter::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::BasicTemporalFilter, shaderPipeline);
        }

        // Load the Lambertian lighting shader
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, LambertianOnly::g_PSMain, sizeof(LambertianOnly::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::LambertianOnly, shaderPipeline);
        }

        // Load the Radiosity computation shader
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, ComputeRadiosity::g_PSMain, sizeof(ComputeRadiosity::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::ComputeRadiosity, shaderPipeline);
        }

        // Load the Tonemap shader
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, Tonemap::g_PSMain, sizeof(Tonemap::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::Tonemap, shaderPipeline);
        }

        // Load the DoF scene split shader
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, DoF_Split::g_PSMain, sizeof(DoF_Split::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::DoF_Split, shaderPipeline);
        }

        // Load the DoF vertical blur shader
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, DoF_Blur_Vertical::g_PSMain, sizeof(DoF_Blur_Vertical::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::DoF_Blur_Vertical, shaderPipeline);
        }

        // Load the DoF horizontal blur shader
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, DoF_Blur_Horizontal::g_PSMain, sizeof(DoF_Blur_Horizontal::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::DoF_Blur_Horizontal, shaderPipeline);
        }

        // Load the shader for compositing depth of field together
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, DoF_Composite::g_PSMain, sizeof(DoF_Composite::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::DoF_Composite, shaderPipeline);
        }
    }

    ShaderPipeline::Ptr ShaderManager::GetShaderPipeline(ShaderName shaderName)
    {
        auto shaderPipelineIt = m_shaderMap.find(shaderName);

        // The shader should exist within the map if it has an enum value.
        // If passed an incorrect enum value then we need to assert.
        EngineAssert(shaderPipelineIt != m_shaderMap.end());

        return shaderPipelineIt->second;
    }
}