#include "pch.h"
#include "Engine.h"
#include <MeshMaker\MeshMaker.h>
#include <Scene\Components\MeshInstance.h>
#include <Loader.h>
#include <Utils\Math\MathHelpers.h>
#include <DDSTextureLoader.h>

namespace Engine
{
    // Taken from DeepGBuffer paper : http://graphics.cs.williams.edu/papers/DeepGBuffer16/
    // Precalculated number of spiral taps for calculating AO. Values calculated to minimise
    // Only supports up to 99 AO samples. This is fine though as likely anything more that 100 taps is overkill
    uint32_t CalcSpiralTurns(uint32_t numSamples)
    {
    #define NUM_PRECOMPUTED 100

        static int minDiscrepancyArray[NUM_PRECOMPUTED] = {
            //  0   1   2   3   4   5   6   7   8   9
            1,  1,  1,  2,  3,  2,  5,  2,  3,  2,  // 0
            3,  3,  5,  5,  3,  4,  7,  5,  5,  7,  // 1
            9,  8,  5,  5,  7,  7,  7,  8,  5,  8,  // 2
            11, 12,  7, 10, 13,  8, 11,  8,  7, 14,  // 3
            11, 11, 13, 12, 13, 19, 17, 13, 11, 18,  // 4
            19, 11, 11, 14, 17, 21, 15, 16, 17, 18,  // 5
            13, 17, 11, 17, 19, 18, 25, 18, 19, 19,  // 6
            29, 21, 19, 27, 31, 29, 21, 18, 17, 29,  // 7
            31, 31, 23, 18, 25, 26, 25, 23, 19, 34,  // 8
            19, 27, 21, 25, 39, 29, 17, 21, 27, 29 }; // 9

        if (numSamples < NUM_PRECOMPUTED) {
            return minDiscrepancyArray[numSamples];
        }
        else {
            return 5779; // Some large prime. Hope it does alright. It'll at least never degenerate into a perfect line until we have 5779 samples...
        }

    #undef NUM_PRECOMPUTED
    }

    std::vector<Utils::Maths::Vector2> GenerateCameraJitterSequence(const uint32_t samples)
    {
        auto xSamples = Utils::MathHelpers::GenerateHaltonSequence(samples, 2);
        auto ySamples = Utils::MathHelpers::GenerateHaltonSequence(samples, 3);

        std::vector<Utils::Maths::Vector2> result;
        for (uint32_t i = 0; i < samples; i++)
        {
            // Similar to UE4
            // We want to transform into the range[-0.5, 0.5]
            // and generate the samples in the normal distribution
            // exp(x^2 / Sigma^2)
            float u1 = xSamples[i];
            float u2 = ySamples[i];

            float sigma = 0.47f;
            float temp = 0.5f / sigma;
            float inWindow = expf(-0.5f * temp * temp);

            // Box-Muller transform
            float theta = Utils::Maths::kPI * 2.0f * u2;
            float r = sigma * sqrt(-2.0f * logf((1.0f - u1) * inWindow + u1 ));

            float sampleX = r * cos(theta);
            float sampleY = r * sin(theta);

            result.push_back({ sampleX, sampleY });
        }

        return result;
    }

    // class constants
    const uint32_t kHiZ_MaxMip = 5;
    const uint32_t kRadiosityBuffer_MaxMip = kHiZ_MaxMip;
    const uint32_t kAO_numSamples = 14;
    const uint32_t kAO_numSpiralTurns = CalcSpiralTurns(kAO_numSamples);
    const uint32_t kTemporalAASamples = 8;
    const uint32_t kBilateralBlurWidth = 7;
    const std::vector<Utils::Maths::Vector2> kJitterSequence = GenerateCameraJitterSequence(kTemporalAASamples);

    /*
        The percentage that we want the guard band to extend the view 
        The Guard band is required to avoid suppress incorrect AO and Radiosity
        at screen edges. e.g. A value of 0.1 in x will extend the width by
        10% in both directions. A resoultion of 1280 in x will result in render
        targets with an additional 128 pixels on either side giving a total
        horizontal resolution of 1536
    */
    const Utils::Maths::Vector2 kGuardBandPercentage = {0.1f, 0.1f};



    // TODO: Continue to maintain debug controls
    // TODO: Make switching between different shading models easier. Need way of controlling what is expected as shader constants
    // TODO: Tidy this class it's getting mad.
    // TODO: Fix PostEffect class It's garbage. Each instance shouldn't be assigned to a constant buffer.
            // Why not just pass post effect camera a shader pipeline and we can set effect data here.
    // TODO: Something seems up with ssVel calculation. Check to make sure it is correct.
    // TODO: Apply basic temporal filtering to AO. as it is flickering
    Engine::Engine()
    {
    }


    Engine::~Engine()
    {
    }


    bool Engine::Initialize(EngineCreateOptions createOptions)
    {
        bool result;

        // Create the input object.  The input object will be used to handle reading the keyboard and mouse input from the user.
        m_inputManager = std::make_shared<InputManager>();

        // Create the Direct3D object.
        m_direct3D = std::make_shared<D3DClass>();

        // Initialize the Direct3D object.
        m_direct3D->Initialize(createOptions);

        // Initialise the shader manager
        m_shaderManager = std::make_shared<ShaderManager>(m_direct3D->GetDevice());

        // Create Mesh object
        m_mesh = Utils::MeshMaker::CreateCube(m_direct3D->GetDevice());

        // Initialise Scene
        m_scene = std::make_shared<Scene>(m_direct3D.get());
        m_scene->Initialize();

        CreateGlobalOptions();
        // If defined attach the root scene element added callback.
        if (createOptions.RootSceneElementAddedCallback != nullptr)
        {
            createOptions.RootSceneElementAddedCallback(m_debugOptions, 0);
            createOptions.RootSceneElementAddedCallback(m_aoOptions, 0);
            createOptions.RootSceneElementAddedCallback(m_giOptions, 0);
            m_scene->GetRootNode()->SetChildAddedCallback(createOptions.RootSceneElementAddedCallback);
        }

        // Get the shadow map shader pipeline
        auto shadowPipeline = m_shaderManager->GetShaderPipeline(ShaderName::ShadowMapping);
        // Initialize light manager
        m_lightManager.Initialize(m_direct3D->GetDevice(), m_scene, shadowPipeline);

        // Add lights to the scene
        auto lightNode = m_scene->AddNode();
        lightNode->SetPosition({ 0.0f, 10.0f, 0.0f });
        lightNode->SetRotation(Utils::Maths::Quaternion::CreateFromYawPitchRoll(0.0f, Utils::Maths::DegreesToRadians(90.0f), 0.0f));
        auto light = lightNode->AddComponent<Light>();
        light->SetShadowCastingEnabled(true);
        light->SetIntensity(2.0f);
        light->SetColor({ DirectX::Colors::Wheat });

        /*lightNode = m_scene->AddNode();
        lightNode->SetRotation(Utils::Maths::Quaternion::CreateFromYawPitchRoll(Utils::Maths::DegreesToRadians(45.0f), Utils::Maths::DegreesToRadians(45.0f), 0.0f));
        light = lightNode->AddComponent<Light>();
        light->SetColor({ DirectX::Colors::Wheat });*/

        // Create the camera object
        auto cameraNode = m_scene->AddNode();
        m_camera = cameraNode->AddComponent<Camera>();
        m_camera->SetJitterEnabled(true);
        m_camera->SetJitterSequence(kJitterSequence);
        cameraNode->SetPosition({ 0.0f, 0.0f, -10.0f });

        // Create post processing camera
        auto postCameraNode = m_scene->AddNode();
        m_postProcessCamera = postCameraNode->AddComponent<PostProcessingCamera>();

        // Create post effect
        auto postEffectPipeline = m_shaderManager->GetShaderPipeline(ShaderName::GBuffer_Shade);
        m_postEffect = std::make_shared<PostEffect<PostEffectConstants>>(m_direct3D->GetDevice(), postEffectPipeline);

        // Create deep G-Buffer constant buffer
        m_deepGBufferConstant = std::make_shared<ConstantBuffer<DeepGBufferConstants>>(PipelineStage::Pixel, m_direct3D->GetDevice());

        // Create the timer object.
        m_timer = std::make_shared<TimerClass>();

        // Initialize the timer object.
        result = m_timer->Initialize();
        EngineAssert(result);

        // Create the position object.
        m_position = std::make_shared<PositionClass>();

        // Set the initial position of the viewer to the same as the initial camera position.
        m_position->SetPosition(0.0f, 0.0f, -10.0f);

        // Create depth sampler
        m_depthSampler = std::make_shared<Sampler>(m_direct3D->GetDevice(), D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP);
        InitializeScene();
        return true;
    }

    void Engine::InitializeScene()
    {
    }

    void Engine::CreateGlobalOptions()
    {
        // Set initialvalues for constants
        m_giConstants.aoRadius = 5.0f; // 1 metre
        m_giConstants.aoBias = 0.01f; // 1 cm
        m_giConstants.aoIntensity = 1.0f;
        m_giConstants.numSamples = kAO_numSamples;
        m_giConstants.numSpiralTurns = float(kAO_numSpiralTurns);
        m_giConstants.radiosityRadius = m_giConstants.aoRadius * 5.0f;

        m_deepGBufferData.minimumSeparation = 2.0f;

        m_debugOptions = std::make_shared<SceneElement>(L"Debug Options");
        m_giOptions = std::make_shared<SceneElement>(L"GI Options");
        m_aoOptions = std::make_shared<SceneElement>(L"AO Options");

        // DEBUG OPTIONS
        {
            auto getter = [&]()->bool { return m_debugConstants.displaySecondLayer == 1.0f; };
            auto setter = [&](bool value) {m_debugConstants.displaySecondLayer = value ? 1.0f : 0.0f; };
            m_debugOptions->RegisterBooleanProperty(L"Display Second Layer", getter, setter);
        }

        {
            auto getter = [&]()->bool { return m_camera->GetJitterEnabled(); };
            auto setter = [&](bool value) { m_camera->SetJitterEnabled(value); };
            m_debugOptions->RegisterBooleanProperty(L"Jitter enabled", getter, setter);
        }

        {
            auto getter = [&]()->float { return m_debugConstants.gBufferTargetIndex; };
            auto setter = [&](float value) {m_debugConstants.gBufferTargetIndex = value; };
            m_debugOptions->RegisterScalarProperty(L"RenderTarget index", getter, setter, 0.0f, 6.0f);
        }

        {
            auto getter = [&]()->float { return m_deepGBufferData.minimumSeparation; };
            auto setter = [&](float value) {m_deepGBufferData.minimumSeparation = value; };
            m_debugOptions->RegisterScalarProperty(L"Minimum Separation", getter, setter, 0.0f, 5.0f);
        }

        {
            auto getter = [&]()->float { return m_debugConstants.sceneExposure; };
            auto setter = [&](float value) {m_debugConstants.sceneExposure = value; };
            m_debugOptions->RegisterScalarProperty(L"Exposure", getter, setter, 0.0f, 10.0f);
        }

        // AO OPTIONS

        {
            auto getter = [&]()->float { return m_giConstants.aoRadius; };
            auto setter = [&](float value) {m_giConstants.aoRadius = value; };
            m_aoOptions->RegisterScalarProperty(L"AO Radius", getter, setter, 0.0f, 10.0f);
        }


        {
            auto getter = [&]()->float { return m_giConstants.aoBias; };
            auto setter = [&](float value) {m_giConstants.aoBias = value; };
            m_aoOptions->RegisterScalarProperty(L"AO Bias", getter, setter, 0.0f, 0.1f);
        }

        {
            auto getter = [&]()->float { return m_giConstants.aoIntensity; };
            auto setter = [&](float value) {m_giConstants.aoIntensity = value; };
            m_aoOptions->RegisterScalarProperty(L"AO Intensity", getter, setter, 1.0f, 4.0f);
        }

        {
            auto getter = [&]()->float { return m_giConstants.aoUseSecondLayer; };
            auto setter = [&](float value) {m_giConstants.aoUseSecondLayer = value; };
            m_aoOptions->RegisterScalarProperty(L"AO Use second layer", getter, setter, 0.0f, 1.0f);
        }

        {
            auto getter = [&]()->bool { return m_debugConstants.aoEnabled == 1.0f; };
            auto setter = [&](bool value) { m_debugConstants.aoEnabled = value ? 1.0f : 0.0f; };
            m_aoOptions->RegisterBooleanProperty(L"AO Enabled", getter, setter);
        }

        // GI OPTIONS
        {
            auto getter = [&]()->bool { return m_debugConstants.radiosityEnabled == 1.0f; };
            auto setter = [&](bool value) {m_debugConstants.radiosityEnabled = value ? 1.0f : 0.0f; };
            m_giOptions->RegisterBooleanProperty(L"Radiosity Enabled", getter, setter);
        }

        {
            auto getter = [&]()->float { return m_giConstants.radiosityRadius; };
            auto setter = [&](float value) {m_giConstants.radiosityRadius = value; };
            m_giOptions->RegisterScalarProperty(L"Radiosity Radius", getter, setter, 0.0f, 50.0f);
        }

        {
            auto getter = [&]()->float { return m_giConstants.radiosityPropogationDamping; };
            auto setter = [&](float value) {m_giConstants.radiosityPropogationDamping = value; };
            m_giOptions->RegisterScalarProperty(L"Propogation Damping", getter, setter, 0.0f, 1.0f);
        }

        {
            auto getter = [&]()->float { return m_giConstants.saturatedBoost; };
            auto setter = [&](float value) {m_giConstants.saturatedBoost = value; };
            m_giOptions->RegisterScalarProperty(L"Color Boost", getter, setter, 1.0f, 2.0f);
        }

        {
            auto getter = [&]()->float { return m_giConstants.confidenceCentre; };
            auto setter = [&](float value) {m_giConstants.confidenceCentre = value; };
            m_giOptions->RegisterScalarProperty(L"Confidence Centre", getter, setter, 0.0f, 1.0f);
        }

        {
            auto getter = [&]()->float { return m_giConstants.envIntensity; };
            auto setter = [&](float value) {m_giConstants.envIntensity = value; };
            m_giOptions->RegisterScalarProperty(L"Environment Intensity", getter, setter, 0.0f, 1.0f);
        }
    }

    void Engine::SetFrameInput(InputState newInputState)
    {
        m_inputManager->Update(newInputState);
    }

    // TODO: Remove all of these returned bools
    bool Engine::Frame()
    {
        bool result;

        // Update the system stats.
        m_timer->Frame();
        float deltaTime = m_timer->GetTime();

        // Do the frame input processing.
        result = HandleInput(deltaTime);
        if (!result)
        {
            return false;
        }

        // Update the scene
        m_scene->Update(deltaTime);
        m_scene->SetCameraTransform(m_camera->GetSceneNode()->GetWorldTransform());

        // Render the graphics.
        result = RenderGraphics();
        if (!result)
        {
            return false;
        }

        // Convert ms into seconds
        m_elapsedTime += deltaTime / 1000.0f;
        m_debugConstants.sceneTime = m_elapsedTime;
        return result;
    }

    void Engine::ResizeBuffers(uint32_t newWidth, uint32_t newHeight)
    {
        EngineAssert(newWidth > 0);
        EngineAssert(newHeight > 0);
        m_direct3D->ResizeBuffers(newWidth, newHeight);

        uint32_t guardBandWidth = static_cast<uint32_t>(newWidth * kGuardBandPercentage.x * 2.0f);
        uint32_t guardBandHeight = static_cast<uint32_t>(newHeight * kGuardBandPercentage.y * 2.0f);
        m_guardBandSizePixels = { guardBandWidth/2.0f, guardBandHeight/2.0f };

        // Re-create G-Buffer with new dimensions
        // We use G-Buffer data for ao and radiosity so it must be 
        // extended to the guard band width
        const uint32_t GBufferLayers = 2;
        RenderTargetBundle::Ptr GBuffer = std::make_shared<RenderTargetBundle>(m_direct3D->GetDevice(), newWidth + guardBandWidth, newHeight + guardBandHeight, GBufferLayers);
        GBuffer->CreateRenderTarget(L"Main", DXGI_FORMAT_R8G8B8A8_UNORM);
        GBuffer->CreateRenderTarget(L"Emissive", DXGI_FORMAT_R11G11B10_FLOAT);
        GBuffer->CreateRenderTarget(L"Normal", DXGI_FORMAT_R8G8B8A8_UNORM);
        GBuffer->CreateRenderTarget(L"SSVelocity", DXGI_FORMAT_R16G16_FLOAT, {1.0f, 1.0f, 1.0f, 1.0f});
        GBuffer->CreateRenderTarget(L"CSZ", DXGI_FORMAT_R32_FLOAT, {-1.0f, -1.0f, -1.0f, -1.0f });
        GBuffer->Finalise();

        // Set the G-Buffer for output from camera
        m_camera->SetRenderTargetBundle(GBuffer);

        // Create bundle for hierarchical Z
        // Used in AO calculation and thus must be extended by guard band
        m_hiZBundle = std::make_shared<RenderTargetBundle>(m_direct3D->GetDevice(), GBuffer->GetWidth(), GBuffer->GetHeight(), 1, kHiZ_MaxMip + 1, false);
        m_hiZBundle->CreateRenderTarget(L"Hi-Z", DXGI_FORMAT_R32G32_FLOAT);
        m_hiZBundle->Finalise();

        auto Hi_ZTex = m_hiZBundle->GetRenderTarget(L"Hi-Z")->GetTexture();
        m_hiZMipView = std::make_shared<TextureMipView>(m_direct3D->GetDevice(), Hi_ZTex, kHiZ_MaxMip + 1);

        /*AO needs to store data for the guard band and thus must be extended*/
        m_aoBundle = std::make_shared<RenderTargetBundle>(m_direct3D->GetDevice(), GBuffer->GetWidth(), GBuffer->GetHeight(), 1, 0, false);
        m_aoBundle->CreateRenderTarget(L"AO", DXGI_FORMAT_R8G8B8A8_UNORM);
        m_aoBundle->Finalise();

        // Temporary buffer for post process effects. Used to store temp results. e.g. betweeen seperable blur
        m_tempBundle = std::make_shared<RenderTargetBundle>(m_direct3D->GetDevice(), GBuffer->GetWidth(), GBuffer->GetHeight(), 1, 0, false);
        m_tempBundle->CreateRenderTarget(L"Temp", DXGI_FORMAT_R16G16B16A16_FLOAT);
        m_tempBundle->Finalise();

        // Stores lambertian and packed normals for use in radiosity
        // Since this is used in radiosity it must be extended by guard band
        m_lambertianOnlyBundle = std::make_shared<RenderTargetBundle>(m_direct3D->GetDevice(), GBuffer->GetWidth(), GBuffer->GetHeight(), 1, kRadiosityBuffer_MaxMip + 1, false);
        m_lambertianOnlyBundle->CreateRenderTarget(L"Lambertian1", DXGI_FORMAT_R11G11B10_FLOAT); //TODO: Eventually make HDR
        m_lambertianOnlyBundle->CreateRenderTarget(L"Lambertian2", DXGI_FORMAT_R11G11B10_FLOAT);
        m_lambertianOnlyBundle->CreateRenderTarget(L"PackedNormals", DXGI_FORMAT_R8G8B8A8_UNORM);
        m_lambertianOnlyBundle->Finalise();

        // Create the mip view for the lambertian only buffer. This is used to generate the custom downsample
        m_lambertianOnlyBundleMipView = std::make_shared<TextureBundleMipView>(m_direct3D->GetDevice(), m_lambertianOnlyBundle, kRadiosityBuffer_MaxMip + 1);

        m_radiosityBundle = std::make_shared<RenderTargetBundle>(m_direct3D->GetDevice(), GBuffer->GetWidth(), GBuffer->GetHeight(), 1, 0, false);
        m_radiosityBundle->CreateRenderTarget(L"Radiosity", DXGI_FORMAT_R16G16B16A16_FLOAT);
        m_radiosityBundle->Finalise();

        m_filteredRadiosityBundle = std::make_shared<RenderTargetBundle>(m_direct3D->GetDevice(), GBuffer->GetWidth(), GBuffer->GetHeight(), 1, 0, false);
        m_filteredRadiosityBundle->CreateRenderTarget(L"Filtered Radiosity", DXGI_FORMAT_R16G16B16A16_FLOAT);
        m_filteredRadiosityBundle->Finalise();

        m_HDRSceneBundle = std::make_shared<RenderTargetBundle>(m_direct3D->GetDevice(), newWidth, newHeight, 1, 0, false);
        m_HDRSceneBundle->CreateRenderTarget(L"HDR Scene", DXGI_FORMAT_R16G16B16A16_FLOAT);
        m_HDRSceneBundle->Finalise();
    }


    void Engine::LoadFile(const uint8_t * data, uint64_t byteCount)
    {
        auto shaderPipeline = m_shaderManager->GetShaderPipeline(ShaderName::DeepGBuffer_Gen);
        Loader::Ptr loader = std::make_shared<Loader>(m_direct3D, m_scene, shaderPipeline);

        loader->LoadFile(data, byteCount);
    }

    void Engine::LoadEnvironment(const uint8_t * data, uint64_t byteCount)
    {
        auto envMap = Texture::CreateTextureFromMemory(data, byteCount, m_direct3D.get(), true);
        if (envMap->IsCubeMap())
        {
            m_lightManager.SetEnvironmentMap(envMap);
        }
    }

    bool Engine::HandleInput(float frameTime)
    {
        bool keyDown;
        float posX, posY, posZ, rotX, rotY, rotZ;

        // Set the frame time for calculating the updated position.
        m_position->SetFrameTime(frameTime);

        // Handle the input.
        keyDown = m_inputManager->IsKeyDown(KeyCodes::LEFT);
        m_position->TurnLeft(keyDown);

        keyDown = m_inputManager->IsKeyDown(KeyCodes::RIGHT);
        m_position->TurnRight(keyDown);

        keyDown = m_inputManager->IsKeyDown(KeyCodes::UP);
        m_position->MoveForward(keyDown);

        keyDown = m_inputManager->IsKeyDown(KeyCodes::DOWN);
        m_position->MoveBackward(keyDown);

        keyDown = m_inputManager->IsKeyDown(KeyCodes::A);
        m_position->MoveUpward(keyDown);

        keyDown = m_inputManager->IsKeyDown(KeyCodes::Z);
        m_position->MoveDownward(keyDown);

        keyDown = m_inputManager->IsKeyDown(KeyCodes::PAGE_UP);
        m_position->LookUpward(keyDown);

        keyDown = m_inputManager->IsKeyDown(KeyCodes::PAGE_DOWN);
        m_position->LookDownward(keyDown);

        if (m_inputManager->IsKeyPressed(KeyCodes::H))
        {
            auto projection = m_camera->GetProjectionMode();
            projection = projection == Camera::ProjectionMode::Orthographic ? Camera::ProjectionMode::Perspective : Camera::ProjectionMode::Orthographic;
            m_camera->SetProjectionMode(projection);
        }

        if (m_inputManager->IsKeyDown(KeyCodes::Y))
        {
            //m_camera->SetProjectionMode(Camera::ProjectionMode::Perspective);
        }

        // Get the view point position/rotation.
        m_position->GetPosition(posX, posY, posZ);
        m_position->GetRotation(rotX, rotY, rotZ);
        auto rotation = Utils::Maths::Quaternion::CreateFromYawPitchRoll(
            Utils::Maths::DegreesToRadians(rotY),
            Utils::Maths::DegreesToRadians(rotX),
            Utils::Maths::DegreesToRadians(rotZ));

        m_camera->GetSceneNode()->SetPosition({ posX, posY, posZ });
        m_camera->GetSceneNode()->SetRotation(rotation);
        return true;
    }


    bool Engine::RenderGraphics()
    {
        // Clear the scene.
        m_direct3D->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);

        // Render the deep G-Buffer
        m_direct3D->BeginRenderEvent(L"Generating Deep G-Buffer");
            GenerateDeepGBuffer();
        m_direct3D->EndRenderEvent();

        // Now need to copy depth
        auto bundle = m_camera->GetRenderTargetBundle();
        if (bundle != nullptr)
        {
            // We can move this to where we downsample colour and normals to avoid any additional overhead.
            m_direct3D->BeginRenderEvent(L"Generating Hierarchical Z buffer");
                // Take the camera-space Z texture and downsample into the mips
                auto HiZTexture = bundle->GetRenderTarget(L"CSZ")->GetTexture();
                GenerateHiZ(HiZTexture);
            m_direct3D->EndRenderEvent();

            // We want to find a way to temporally accumulate the ambient occlusion.
            // This will avoid flickering on small objects.
            m_direct3D->BeginRenderEvent(L"Generating Ambient Occlusion");
                // Generate the ambient occlusion
                GenerateAO();
            m_direct3D->EndRenderEvent();

            // HERE LIES WHERE WE SHALL GENERATE RADIOSITY

            // Upload light data for deferred shading
            m_lightManager.GatherLights(m_scene, m_direct3D->GetDeviceContext(), LightSpaceModifier::Camera);

            m_direct3D->BeginRenderEvent(L"Rendering lambertian only and packed normals");
                // Diffuse lighting of scene
                LambertianOnly(bundle);
            m_direct3D->EndRenderEvent();

            m_direct3D->BeginRenderEvent(L"Downsampling lambertian and normals");
                // Generate Hierarchical mip chain for colour, normals and camera-space Z for all layers
                // Note: Can use hierarchical Z generated for AO
                GenerateRadiosityBufferMips();
            m_direct3D->EndRenderEvent();

            m_direct3D->BeginRenderEvent(L"Generating Radiosity");
                // Sample hierarchical G-Buffer to generate radioisty.
                ComputeRadiosity();
            m_direct3D->EndRenderEvent();

            // TODO: We can add a clip rectangle to the radiosity filter so that only the required pixels
            // are available for the blur.
            // During the blur we can then reduce the total width down to the output width
            auto ssVelTexture = bundle->GetRenderTarget(L"SSVelocity")->GetTexture();
            // Blend new samples with radiosity from last frame
            m_direct3D->BeginRenderEvent(L"Filter Radiosity");
                // Sample hierarchical G-Buffer to generate radioisty.
                FilterRadiosity(ssVelTexture);
            m_direct3D->EndRenderEvent();

            // Save generated texture to be used next frame

            // Bilateral blur on resulting texture

            // BINGO BONGO BANGO: Pass texture to deep G-Buffer shade to have it applied to the scene
            
            // HERE ENDS WHERE WE SHALL HAVE GENERATED RADIOSITY

            m_direct3D->BeginRenderEvent(L"Shading G-Buffer");
                // Shade the GBuffer using generated AO.
                ShadeGBuffer(bundle);
            m_direct3D->EndRenderEvent();

            m_direct3D->BeginRenderEvent(L"Accumulating TSAA");
                RunTSAA(ssVelTexture);
                // We want the prev frame before it has been tonemapped so we take it from the HDR scene bundle
                m_prevFrame = m_direct3D->CopyTexture(m_HDRSceneBundle->GetRenderTarget(0)->GetTexture());
            m_direct3D->EndRenderEvent();
            
            // Finally apply tonemapping and exposure
            m_direct3D->BeginRenderEvent(L"Tonemap");
                Tonemap();
            m_direct3D->EndRenderEvent();

            m_prevDepth = m_direct3D->CopyTexture(bundle->GetDepthBuffer()->GetTexture());
        }
        // Present the rendered scene to the screen.
        m_direct3D->EndScene();

        return true;
    }

    void Engine::GenerateDeepGBuffer()
    {
        // Upload previous depth to shader
        if (m_prevDepth)
        {
            // Upload at register 7 as material takes registers 0 -> 6
            // TODO: Formalise this
            m_prevDepth->UploadData(m_direct3D->GetDeviceContext(), PipelineStage::Pixel, 7);
            m_depthSampler->UploadData(m_direct3D->GetDeviceContext(), 7);
        }

        // Upload deep gbuffer constants
        m_deepGBufferConstant->SetData(m_deepGBufferData);
        m_deepGBufferConstant->UploadData(m_direct3D->GetDeviceContext());
        // Render the scene.
        // This generates our deep G-Buffer
        m_camera->Render(m_direct3D, m_scene);

        // We are finished drawing to the G-Buffer and now want to send
        // it as a shader resource so all RTVs must be cleared from the device
        m_direct3D->UnbindAllRenderTargets();
    }

    void Engine::ShadeGBuffer(RenderTargetBundle::Ptr GBuffer)
    {
        auto aoTarget = m_aoBundle->GetRenderTarget(L"AO");

        // Upload G-buffer data to device
        GBuffer->SetShaderResources(m_direct3D->GetDeviceContext());
        aoTarget->GetTexture()->UploadData(m_direct3D->GetDeviceContext(), PipelineStage::Pixel, 6);
        m_filteredRadiosityBundle->SetShaderResources(m_direct3D->GetDeviceContext(), 7);

        // Set post effect constants
        m_postEffect->SetEffectData(m_debugConstants);

        // Shade G-Buffer
        m_postProcessCamera->SetRenderTargetBundle(m_tempBundle); // Draw to the temp bundle before passing to TSAA
        m_postProcessCamera->RenderPostEffect(m_direct3D, m_postEffect);
    }

    void Engine::LambertianOnly(RenderTargetBundle::Ptr GBuffer)
    {
        m_postProcessCamera->SetRenderTargetBundle(m_lambertianOnlyBundle);
        m_lambertianOnlyBundle->Clear(m_direct3D->GetDeviceContext());
        m_lambertianOnlyBundle->SetTargetMip(0);

        // Upload G-Buffer data
        GBuffer->SetShaderResources(m_direct3D->GetDeviceContext());

        // Create shader effect
        auto lambertShaderPipeline = m_shaderManager->GetShaderPipeline(ShaderName::LambertianOnly);
        auto lambertOnlyEffect = std::make_shared<PostEffect<PostEffectConstants>>(m_direct3D->GetDevice(), lambertShaderPipeline);

        // TODO: When ready upload previous indirect lighting
        const uint32_t radiosityRegister = 6;
        m_filteredRadiosityBundle->SetShaderResources(m_direct3D->GetDeviceContext(), radiosityRegister);
        if (m_prevDepth != nullptr)
        {
            m_prevDepth->UploadData(m_direct3D->GetDeviceContext(), PipelineStage::Pixel, 7);
        }

        lambertOnlyEffect->SetEffectData(m_debugConstants);
        // Render the effect
        m_postProcessCamera->RenderPostEffect(m_direct3D, lambertOnlyEffect);

        // To avoid the filtered radioisty still being bound as input when generating next radiosity clear that register
        m_direct3D->UnbindShaderResourceView(radiosityRegister);
    }

    void Engine::GenerateRadiosityBufferMips()
    {
        // We want to render to the mip maps of the lambertian only buffer
        m_postProcessCamera->SetRenderTargetBundle(m_lambertianOnlyBundle);

        // Create our effect
        auto radiosityBufferDownsamplePipeline = m_shaderManager->GetShaderPipeline(ShaderName::Minify_Lambertian);
        auto radiosityBufferDownsampleEffect = std::make_shared<PostEffect<PostEffectConstants>>(m_direct3D->GetDevice(), radiosityBufferDownsamplePipeline);

        // For each of the targeted mip maps. Render our effect binding the correct RTVs and SRVs
        for (int i = 1; i <= kRadiosityBuffer_MaxMip; i++)
        {
            // Unbind current render targets so we don't get collisions when binding previous output views as input
            m_direct3D->UnbindAllRenderTargets();

            radiosityBufferDownsampleEffect->SetEffectData(m_debugConstants);

            // Set the mip we are currently rendering to
            m_lambertianOnlyBundle->SetTargetMip(i);
            // We want to sample from the previous mip map
            m_lambertianOnlyBundleMipView->SetCurrentMip(i - 1);
            m_lambertianOnlyBundleMipView->UploadData(m_direct3D->GetDeviceContext());

            // Render our effect
            m_postProcessCamera->RenderPostEffect(m_direct3D, radiosityBufferDownsampleEffect);
        }
    }

    void Engine::ComputeRadiosity()
    {
        m_postProcessCamera->SetRenderTargetBundle(m_radiosityBundle);

        // Create post effect
        auto radiosityPipeline = m_shaderManager->GetShaderPipeline(ShaderName::ComputeRadiosity);
        auto radiosityEffect = std::make_shared<PostEffect<GIConstants>>(m_direct3D->GetDevice(), radiosityPipeline);

        // Upload downsampled maps
        m_lambertianOnlyBundle->SetShaderResources(m_direct3D->GetDeviceContext());
        m_depthSampler->UploadData(m_direct3D->GetDeviceContext(), 0);
        // upload csZ
        m_hiZBundle->SetShaderResources(m_direct3D->GetDeviceContext(), 3);
        // Set effect data
        radiosityEffect->SetEffectData(m_giConstants);

        // Render effect
        m_postProcessCamera->RenderPostEffect(m_direct3D, radiosityEffect);
    }

    void Engine::FilterRadiosity(Texture::Ptr ssVelTexture)
    {
        // Apply temporal filter
        auto temoralFilterPipeline = m_shaderManager->GetShaderPipeline(ShaderName::BasicTemporalFilter);
        auto temporalFilterEffect = std::make_shared<PostEffect<PostEffectConstants>>(m_direct3D->GetDevice(), temoralFilterPipeline);

        // Upload resources to shader.
        m_radiosityBundle->SetShaderResources(m_direct3D->GetDeviceContext());
        m_hiZBundle->SetShaderResources(m_direct3D->GetDeviceContext(), 2);
        ssVelTexture->UploadData(m_direct3D->GetDeviceContext(), PipelineStage::Pixel, 4);
        m_depthSampler->UploadData(m_direct3D->GetDeviceContext(), 0);

        if (m_prevRawRadiosity != nullptr && m_prevDepth != nullptr)
        {
            m_prevRawRadiosity->UploadData(m_direct3D->GetDeviceContext(), PipelineStage::Pixel, 1);
            m_prevDepth->UploadData(m_direct3D->GetDeviceContext(), PipelineStage::Pixel, 3);
            m_debugConstants.basicTemporalFilterBlend = 0.9f;
        }

        temporalFilterEffect->SetEffectData(m_debugConstants);
        m_postProcessCamera->SetRenderTargetBundle(m_filteredRadiosityBundle);
        m_postProcessCamera->RenderPostEffect(m_direct3D, temporalFilterEffect);

        // Copy temporally filtered result
        m_prevRawRadiosity = m_direct3D->CopyTexture(m_filteredRadiosityBundle->GetRenderTarget(0)->GetTexture());


        // Apply bilateral blur

        // Reduce viewport to include blur radius without extending
        // too far into guard band
        Utils::Maths::Vector2 clipOffset{ std::max(m_guardBandSizePixels.x - kBilateralBlurWidth, 0.0f), std::max(m_guardBandSizePixels.y - kBilateralBlurWidth, 0.0f) };
        m_postProcessCamera->SetClipRect(clipOffset);

        auto csZTex = m_hiZBundle->GetRenderTarget(0)->GetTexture();
        BlurBundle(m_filteredRadiosityBundle, csZTex);

        // Remove clip offset
        m_postProcessCamera->SetClipRect({ 0.0f, 0.0f });
    }

    // Generates the hierachical Z buffer
    void Engine::GenerateHiZ(Texture::Ptr csZTexture)
    {
        m_hiZBundle->Clear(m_direct3D->GetDeviceContext());
        m_hiZBundle->SetTargetMip(0);
        // Create post effect
        auto csZCopyPipeline = m_shaderManager->GetShaderPipeline(ShaderName::DeepGBuffer_csZCopy);
        auto csZCopyEffect = std::make_shared<PostEffect<PostEffectConstants>>(m_direct3D->GetDevice(), csZCopyPipeline);

        auto HiZConstruction = m_shaderManager->GetShaderPipeline(ShaderName::Minify_CSZ);
        auto HiZPost = std::make_shared<PostEffect<PostEffectConstants>>(m_direct3D->GetDevice(), HiZConstruction);

        // From deep G to single texture
        m_postProcessCamera->SetRenderTargetBundle(m_hiZBundle);
        csZTexture->UploadData(m_direct3D->GetDeviceContext(), PipelineStage::Pixel, 0);
        m_depthSampler->UploadData(m_direct3D->GetDeviceContext(), 0);
        m_postProcessCamera->RenderPostEffect(m_direct3D, csZCopyEffect);


        for (int i = 1; i <= kHiZ_MaxMip; i++)
        {
            // Set current render to target mip i.
            // Pass mip i-1 to shader
            // Render
            m_direct3D->UnbindAllRenderTargets();
            // render to second mip
            HiZPost->SetEffectData(m_debugConstants);
            m_hiZBundle->SetTargetMip(i);
            m_hiZMipView->SetCurrentMip(i - 1); // The mip level we are currently sampling from
            m_hiZMipView->UploadData(m_direct3D->GetDeviceContext(), PipelineStage::Pixel, 0);
            m_postProcessCamera->RenderPostEffect(m_direct3D, HiZPost);
        }

        m_postProcessCamera->SetRenderTargetBundle(nullptr);
    }

    void Engine::GenerateAO()
    {
        // Pass Hierarchical Z to shader to generate AO
        auto aoPipeline = m_shaderManager->GetShaderPipeline(ShaderName::AO);
        auto aoEffect = std::make_shared<PostEffect<GIConstants>>(m_direct3D->GetDevice(), aoPipeline);
        aoEffect->SetEffectData(m_giConstants);

        // Upload hierarchical Z
        m_hiZBundle->SetShaderResources(m_direct3D->GetDeviceContext());

        // Render AO effect
        m_postProcessCamera->SetRenderTargetBundle(m_aoBundle);
        m_postProcessCamera->RenderPostEffect(m_direct3D, aoEffect);

        // Reduce viewport to include blur radius without extending
        // too far into guard band
        Utils::Maths::Vector2 clipOffset {std::max(m_guardBandSizePixels.x - kBilateralBlurWidth, 0.0f), std::max(m_guardBandSizePixels.y - kBilateralBlurWidth, 0.0f)};
        m_postProcessCamera->SetClipRect(clipOffset);
        BlurBundle(m_aoBundle, nullptr);
        
        // Remove clip offset
        m_postProcessCamera->SetClipRect({0.0f, 0.0f});
    }

    void Engine::BlurBundle(RenderTargetBundle::Ptr targetBundle, Texture::Ptr csZTexture)
    {
        m_postProcessCamera->SetRenderTargetBundle(m_tempBundle);
        ShaderName shader = csZTexture == nullptr ? ShaderName::BilateralBlurPacked : ShaderName::BilateralBlur;
        auto blurPipeline = m_shaderManager->GetShaderPipeline(shader);
        auto blurEffect = std::make_shared<PostEffect<PostEffectConstants>>(m_direct3D->GetDevice(), blurPipeline);

        if (csZTexture != nullptr)
        {
            csZTexture->UploadData(m_direct3D->GetDeviceContext(), PipelineStage::Pixel, 1);
        }

        // vertical blur
        m_debugConstants.blurAxis = { 0.0f, 1.0f };
        blurEffect->SetEffectData(m_debugConstants);
        targetBundle->SetShaderResources(m_direct3D->GetDeviceContext());
        m_postProcessCamera->RenderPostEffect(m_direct3D, blurEffect);

        // Set camera to render back to the original AO texture
        m_postProcessCamera->SetRenderTargetBundle(targetBundle);

        // horizontal blur
        // bind old ao render target
        m_debugConstants.blurAxis = { 1.0f, 0.0f };
        blurEffect->SetEffectData(m_debugConstants);
        m_tempBundle->SetShaderResources(m_direct3D->GetDeviceContext());
        m_postProcessCamera->RenderPostEffect(m_direct3D, blurEffect);
    }

    float CatMullRom(float x)
    {
        float ax = abs(x);
        if (ax > 1.0f)
            return ((-0.5f * ax + 2.5f) * ax - 4.0f) * ax + 2.0f;
        else
            return (1.5f * ax - 2.5f) *ax*ax + 1.0f;
    }

    void Engine::RunTSAA(Texture::Ptr ssVelTexture)
    {
        // Now converge TSAA to back buffer
        m_postProcessCamera->SetRenderTargetBundle(m_HDRSceneBundle);
        auto TSAAPipeline = m_shaderManager->GetShaderPipeline(ShaderName::TSAA);
        auto TSAAEffect = std::make_shared<PostEffect<TemporalAAConstants>>(m_direct3D->GetDevice(), TSAAPipeline);
        TemporalAAConstants TSAAData;

        static const float sampleOffsets[9][2]  =
        {
            { -1.0f, -1.0f },
            { 0.0f, -1.0f },
            { 1.0f, -1.0f },
            { -1.0f,  0.0f },
            { 0.0f,  0.0f },
            { 1.0f,  0.0f },
            { -1.0f,  1.0f },
            { 0.0f,  1.0f },
            { 1.0f,  1.0f }
        };


        float weights[kTSAASampleCount];
        float totalWeight = 0.0f;
        auto jitter = m_camera->GetCurrentTemporalJitter();

        for (int i = 0; i < kTSAASampleCount; i++)
        {
            float offsetX = sampleOffsets[i][0] - jitter.x;
            float offsetY = sampleOffsets[i][1] - jitter.y;

            weights[i] = CatMullRom(offsetX) * CatMullRom(offsetY);
            totalWeight += weights[i];
        }
        for (int i = 0; i < kTSAASampleCount; i++)
        {
            TSAAData.sampleWeights[i] = weights[i] / totalWeight;
        }

        TSAAData.currentFrameWeight = m_weightThisFrame;
        TSAAData.guardBandSize = m_guardBandSizePixels;
        TSAAEffect->SetEffectData(TSAAData);

        // Upload current, previous and ssVel
        m_tempBundle->SetShaderResources(m_direct3D->GetDeviceContext());
        if (m_prevFrame != nullptr)
        {
            m_prevFrame->UploadData(m_direct3D->GetDeviceContext(), PipelineStage::Pixel, 1);
        }
        else
        {
            // Need to clear SRV index to avoid reading from incorrect texture
            m_direct3D->UnbindShaderResourceView(1);
        }
        ssVelTexture->UploadData(m_direct3D->GetDeviceContext(), PipelineStage::Pixel, 2);

        // Converge TSAA
        m_postProcessCamera->RenderPostEffect(m_direct3D, TSAAEffect);
    }

    void Engine::Tonemap()
    {
        m_postProcessCamera->SetRenderTargetBundle(nullptr);

        // Create tonemap effect
        auto tonemapPipeline = m_shaderManager->GetShaderPipeline(ShaderName::Tonemap);
        auto tonemapEffect = std::make_shared<PostEffect<PostEffectConstants>>(m_direct3D->GetDevice(), tonemapPipeline);

        tonemapEffect->SetEffectData(m_debugConstants);

        // Upload HDR buffer
        m_HDRSceneBundle->SetShaderResources(m_direct3D->GetDeviceContext());

        // render to back buffer
        m_postProcessCamera->RenderPostEffect(m_direct3D, tonemapEffect);
    }

}