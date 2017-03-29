#pragma once


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "InputManager.h"
#include "d3dclass.h"
#include "timerclass.h"
#include "positionclass.h"
#include "ShaderManager.h"
#include "LightManager.h"
#include "EngineCreateOptions.h"
#include <Scene\Components\PostProcessingCamera.h>
#include <Scene\Components\Light.h>
#include <Scene\Scene.h>
#include <Resources\Mesh.h>
#include <Resources\Sampler.h>
#include <Resources\TextureBundleMipView.h>

// Don't really want this sat here.
#include <Resources\ConstantBufferLayouts.h>

namespace Engine
{
    class Engine
    {
    public:
        using Ptr = std::shared_ptr<Engine>;
        Engine();
        ~Engine();

        bool Initialize(EngineCreateOptions createOptions);
        bool Frame();

        void ResizeBuffers(uint32_t newWidth, uint32_t newHeight);

        void SetFrameInput(InputState newInputState);

        InputManager::Ptr GetInputManager() const { return m_inputManager; }

        void LoadFile(const uint8_t* data, uint64_t byteCount);

        void LoadEnvironment(const uint8_t* data, uint64_t byteCount);

        void CreateGlobalOptions();


    private:
        /*Handles current input for the frame. Taking delta time as input*/
        bool HandleInput(float);
        /*  Renders the frame*/
        bool RenderGraphics();
        /*  Renders all objects in the scene into the deep G-Buffer
            We store diffuse colour, camera-space normal, screen-space velocity and camera-space Z at each pixel*/
        void GenerateDeepGBuffer();
        /*  Shades the deep G-Buffer*/
        void ShadeGBuffer(RenderTargetBundle::Ptr GBuffer);
        /*  Shades the G-Buffer with only lambertian shading. This is in preparation
            for radiosity calculation. Note: we also pack camera-space normals into a single texture
            TODO: Come up with a better name for this*/
        void LambertianOnly(RenderTargetBundle::Ptr GBuffer);
        /*  Takes the lambertian shaded textures and packed normals and generates a mip map chain in exactly the same way as
            GenerateHiZ. This helps reduce the cache misses for radiosity calculation*/
        void GenerateRadiosityBufferMips();
        /*  Takes the downsampled textures and computes the screen space radiosity*/
        void ComputeRadiosity();
        /*  Temporally filters the current frames computed raw radiosity with an accumulation buffer. Then applies a bilateral blur*/
        void FilterRadiosity(Texture::Ptr ssVelTexture);
        /*  Initialises the scene and loads generic objects*/
        void InitializeScene();
        /*  Uses rotated grid subsampling to generate a mip map chain for camera-space Z
            In the same was as GenerateRadiostityBufferMips. The hierarchical-z helps to
            reduce texture cache misses for large radii in the AO.*/
        void GenerateHiZ(Texture::Ptr csZTexture);
        /*  Calculates ambient occlusion at each pixel. Taking the hierarchical Z as input*/
        void GenerateAO();
        /*  Applys a bilateral filter to the given bundle to reduce noise while preserving edges*/
        void BlurBundle(RenderTargetBundle::Ptr targetBundle, Texture::Ptr csZTexture);
        /*  Runs TSAA merging this frames values with the accumulation*/
        void RunTSAA(Texture::Ptr ssVelTexture);
        /*  Applies a post process depth of field effect to the scene*/
        void ApplyDoF();
        /*  Applies conversion from HDR to LDR before presenting*/
        void Tonemap();

    private:
        InputManager::Ptr m_inputManager;
        std::shared_ptr<D3DClass> m_direct3D;
        std::shared_ptr<Light> m_light1;
        std::shared_ptr<TimerClass> m_timer;
        std::shared_ptr<PositionClass> m_position;
        Mesh::Ptr m_mesh;
        Camera::Ptr m_camera;
        ShaderManager::Ptr m_shaderManager;
        Scene::Ptr m_scene;
        LightManager m_lightManager;
        EngineCreateOptions m_createOptions;

        // These can be moved to a more specific application class
        PostProcessingCamera::Ptr m_postProcessCamera;
        PostEffect<PostEffectConstants>::Ptr m_postEffect;
        PostEffectConstants m_debugConstants;
        DepthOfFieldConstants m_dofConstants;
        SceneElement::Ptr m_debugOptions;
        SceneElement::Ptr m_giOptions;
        SceneElement::Ptr m_aoOptions;
        SceneElement::Ptr m_dofOptions;
        ConstantBuffer<DeepGBufferConstants>::Ptr m_deepGBufferConstant;
        DeepGBufferConstants m_deepGBufferData;
        RenderTargetBundle::Ptr m_hiZBundle;
        TextureMipView::Ptr m_hiZMipView;
        RenderTargetBundle::Ptr m_aoBundle;
        // A bundle for postprocessing temporary steps. e.g. mid seperable blur.
        RenderTargetBundle::Ptr m_tempBundle; 
        // Contains HDR scene data to be sent to tonemap shader.
        // When using HDR this should always be the last buffer before presenting
        RenderTargetBundle::Ptr m_HDRSceneBundle;

        RenderTargetBundle::Ptr m_lambertianOnlyBundle;
        TextureBundleMipView::Ptr m_lambertianOnlyBundleMipView;
        RenderTargetBundle::Ptr m_radiosityBundle;
        RenderTargetBundle::Ptr m_filteredRadiosityBundle;
        RenderTargetBundle::Ptr m_dofBundle;
        Texture::Ptr m_prevRawRadiosity;
        GIConstants m_giConstants;

        Utils::Maths::Vector2 m_guardBandSizePixels;

        // Store prev depth for deep G-Buffer prediction
        Texture::Ptr m_prevDepth;
        // Store previous frame for temporal super sampling
        Texture::Ptr m_prevFrame;
        Sampler::Ptr m_depthSampler;

        float m_weightThisFrame = 0.05f;
        float m_elapsedTime = 0.0f;
        float m_lensRadius = 0.01f;
    };
}

