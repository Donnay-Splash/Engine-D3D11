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
    namespace Hierarchical_Z
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_Minify.ps.hlsl.h"
    }

    // Scalable Ambient Obscurance algorithm
    namespace AO
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_ComputeAO.ps.hlsl.h"
    }

    // Depth aware blur for reducing noise in AO
    namespace BilateralBlur
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_DepthAwareBlur.ps.hlsl.h"
    }

    // Shader for converging temporal supersampling
    namespace TSAA
    {
        #include "Shaders\Compiled shaders\DeepGBuffer_TSAA.ps.hlsl.h"
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
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, Hierarchical_Z::g_PSMain, sizeof(Hierarchical_Z::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::Generate_HiZ, shaderPipeline);
        }

        // Load the AO shader
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, AO::g_PSMain, sizeof(AO::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::AO, shaderPipeline);
        }

        // Load the Bilateral Blur shader
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, BilateralBlur::g_PSMain, sizeof(BilateralBlur::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::BilateralBlur, shaderPipeline);
        }

        // Load the TSAA shader
        {
            Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, TSAA::g_PSMain, sizeof(TSAA::g_PSMain), device);
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(device, fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

            m_shaderMap.emplace(ShaderName::TSAA, shaderPipeline);
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