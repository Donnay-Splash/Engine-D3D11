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

	namespace PassThrough
	{
		#include "Shaders\Compiled shaders\PassThrough.vs.hlsl.h"
		#include "Shaders\Compiled shaders\PassThrough.ps.hlsl.h"
	}

    namespace ImGuiShaders
    {
        #include "Shaders/Compiled shaders/ImGui.vs.hlsl.h"
        #include "Shaders/Compiled shaders/ImGui.ps.hlsl.h"
    }

    ShaderManager::ShaderManager()
    {
        LoadCoreShaders();
    }

    void ShaderManager::LoadCoreShaders()
    {
        // Here we create the D3D11 shader objects to be placed into the bundle.
        // At this time we will also create the input layouts for each shader bundle.

		// initialise the pass through shader
		{
			uint32_t inputFlags = (InputElement::Position | InputElement::Normal0 | InputElement::TexCoord0);
			Shader vertexShader = { ShaderType::Vertex, PassThrough::g_VSMain, sizeof(PassThrough::g_VSMain) };
			Shader pixelShader = { ShaderType::Pixel, PassThrough::g_PSMain, sizeof(PassThrough::g_PSMain) };
			ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(inputFlags, vertexShader, pixelShader);

			m_shaderMap.insert(ShaderMapObject(ShaderName::PassThrough, shaderPipeline));
		}

        // Initialise the ImGui shader/material
        {
            uint32_t inputFlags = (InputElement::Position | InputElement::Colour | InputElement::TexCoord0);
            Shader vertexShader = { ShaderType::Vertex, ImGuiShaders::g_VSMain, sizeof(ImGuiShaders::g_VSMain) };
            Shader pixelShader = { ShaderType::Pixel, ImGuiShaders::g_PSMain, sizeof(ImGuiShaders::g_PSMain) };
            ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(inputFlags, vertexShader, pixelShader, Shader(), true);

            m_shaderMap.insert(ShaderMapObject(ShaderName::ImGui, shaderPipeline));
        }


		// TODO: Eventually re-add all of these 

        // Load the Standard forward shaders
        //{

        //    InputLayout::Ptr layout = std::make_shared<InputLayout>(InputElement::Position | InputElement::Normal0 | InputElement::TexCoord0);
        //    Shader::Ptr vertexShader = std::make_shared<Shader>(ShaderType::Vertex, UberShader::g_VSMain, sizeof(UberShader::g_VSMain));
        //    Shader::Ptr pixelShader = std::make_shared<Shader>(ShaderType::Pixel, UberShader::g_PSMain, sizeof(UberShader::g_PSMain));
        //    ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(layout, vertexShader, pixelShader);

        //    m_shaderMap.insert(ShaderMapObject(ShaderName::Uber, shaderPipeline));
        //}

        //// Load deep G-Buffer generation shaders
        //{
        //    InputLayout::Ptr layout = std::make_shared<InputLayout>(InputElement::Position | InputElement::Normal0 | InputElement::TexCoord0 | InputElement::Tangents);
        //    Shader::Ptr vertexShader = std::make_shared<Shader>(ShaderType::Vertex, DeepGBuffer::g_VSMain, sizeof(DeepGBuffer::g_VSMain));
        //    Shader::Ptr pixelShader = std::make_shared<Shader>(ShaderType::Pixel, DeepGBuffer::g_PSMain, sizeof(DeepGBuffer::g_PSMain));
        //    Shader::Ptr geometryShader = std::make_shared<Shader>(ShaderType::Geometry, DeepGBuffer::g_GSMain, sizeof(DeepGBuffer::g_GSMain));
        //    ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(layout, vertexShader, pixelShader, geometryShader);

        //    m_shaderMap.insert(ShaderMapObject(ShaderName::DeepGBuffer_Gen, shaderPipeline));
        //}
        //    
        //// Load Shadow map shader
        //{
        //    // Only position is required for shadow mapping
        //    InputLayout::Ptr layout = std::make_shared<InputLayout>(InputElement::Position);
        //    Shader::Ptr vertexShader = std::make_shared<Shader>(ShaderType::Vertex, ShadowMapping::g_VSMain, sizeof(ShadowMapping::g_VSMain));
        //    ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(layout, vertexShader, nullptr, nullptr);

        //    m_shaderMap.insert(ShaderMapObject(ShaderName::ShadowMapping, shaderPipeline));
        //}

        //// Load post process shaders
        //InputLayout::Ptr fullscreenQuadLayout = std::make_shared<InputLayout>(InputElement::Position | InputElement::TexCoord0);
        //Shader::Ptr fullScreenQuadVS = std::make_shared<Shader>(ShaderType::Vertex, FullScreenQuad::g_VSMain, sizeof(FullScreenQuad::g_VSMain));

        //// Load the post process deferred shaders
        //{
        //    Shader::Ptr pixelShader = std::make_shared<Shader>(ShaderType::Pixel, Deferred::g_PSMain, sizeof(Deferred::g_PSMain));
        //    ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

        //    m_shaderMap.emplace(ShaderName::GBuffer_Shade, shaderPipeline);
        //}

        //// Load the camera-space Z copy
        //{
        //    Shader::Ptr pixelShader = std::make_shared<Shader>(ShaderType::Pixel, csZCopy::g_PSMain, sizeof(csZCopy::g_PSMain));
        //    ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

        //    m_shaderMap.emplace(ShaderName::DeepGBuffer_csZCopy, shaderPipeline);
        //}

        //// Load the Hi_Z generation
        //{
        //    Shader::Ptr pixelShader = std::make_shared<Shader>(ShaderType::Pixel, Minify_CSZ::g_PSMain, sizeof(Minify_CSZ::g_PSMain));
        //    ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

        //    m_shaderMap.emplace(ShaderName::Minify_CSZ, shaderPipeline);
        //}

        //// Load Lambertian minify. This is in preparation for use in the radiosity calculation
        //{
        //    Shader::Ptr pixelShader = std::make_shared<Shader>(ShaderType::Pixel, Minify_Lambertian::g_PSMain, sizeof(Minify_Lambertian::g_PSMain));
        //    ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

        //    m_shaderMap.emplace(ShaderName::Minify_Lambertian, shaderPipeline);
        //}

        //// Load the AO shader
        //{
        //    Shader::Ptr pixelShader = std::make_shared<Shader>(ShaderType::Pixel, AO::g_PSMain, sizeof(AO::g_PSMain));
        //    ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

        //    m_shaderMap.emplace(ShaderName::AO, shaderPipeline);
        //}

        //// Load the Bilateral Blur shader with sampled keys
        //{
        //    Shader::Ptr pixelShader = std::make_shared<Shader>(ShaderType::Pixel, BilateralBlurSampled::g_PSMain, sizeof(BilateralBlurSampled::g_PSMain));
        //    ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

        //    m_shaderMap.emplace(ShaderName::BilateralBlur, shaderPipeline);
        //}

        //// Load the Bilateral Blur shader With packed keys
        //{
        //    Shader::Ptr pixelShader = std::make_shared<Shader>(ShaderType::Pixel, BilateralBlurPacked::g_PSMain, sizeof(BilateralBlurPacked::g_PSMain));
        //    ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

        //    m_shaderMap.emplace(ShaderName::BilateralBlurPacked, shaderPipeline);
        //}

        //// Load the TSAA shader
        //{
        //    Shader::Ptr pixelShader = std::make_shared<Shader>(ShaderType::Pixel, TSAA::g_PSMain, sizeof(TSAA::g_PSMain));
        //    ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

        //    m_shaderMap.emplace(ShaderName::TSAA, shaderPipeline);
        //}

        //// Load the Basic tempral filter
        //{
        //    Shader::Ptr pixelShader = std::make_shared<Shader>(ShaderType::Pixel, BasicTemporalFilter::g_PSMain, sizeof(BasicTemporalFilter::g_PSMain));
        //    ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

        //    m_shaderMap.emplace(ShaderName::BasicTemporalFilter, shaderPipeline);
        //}

        //// Load the Lambertian lighting shader
        //{
        //    Shader::Ptr pixelShader = std::make_shared<Shader>(ShaderType::Pixel, LambertianOnly::g_PSMain, sizeof(LambertianOnly::g_PSMain));
        //    ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

        //    m_shaderMap.emplace(ShaderName::LambertianOnly, shaderPipeline);
        //}

        //// Load the Radiosity computation shader
        //{
        //    Shader::Ptr pixelShader = std::make_shared<Shader>(ShaderType::Pixel, ComputeRadiosity::g_PSMain, sizeof(ComputeRadiosity::g_PSMain));
        //    ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

        //    m_shaderMap.emplace(ShaderName::ComputeRadiosity, shaderPipeline);
        //}

        //// Load the Tonemap shader
        //{
        //    Shader::Ptr pixelShader = std::make_shared<Shader>(ShaderType::Pixel, Tonemap::g_PSMain, sizeof(Tonemap::g_PSMain));
        //    ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(fullscreenQuadLayout, fullScreenQuadVS, pixelShader);

        //    m_shaderMap.emplace(ShaderName::Tonemap, shaderPipeline);
        //}
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