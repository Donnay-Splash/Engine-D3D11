#include "pch.h"
#include "Engine.h"
#include <MeshMaker\MeshMaker.h>
#include <Scene\Components\MeshInstance.h>
#include <Loader.h>
#include <Utils\Math\MathHelpers.h>

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
            result.push_back({ xSamples[i], ySamples[i] });
        }

        return result;
    }

    // class constants
    const uint32_t kHiZ_MaxMip = 5;
    const uint32_t kAO_numSamples = 20;
    const uint32_t kAO_numSpiralTurns = CalcSpiralTurns(kAO_numSamples);
    const uint32_t kTemporalAASamples = 8;
    const std::vector<Utils::Maths::Vector2> kJitterSequence = GenerateCameraJitterSequence(kTemporalAASamples);
    // TODO: Continue to maintain debug controls
    // TODO: Make switching between different shading models easier. Need way of controlling what is expected as shader constants
    // TODO: Make AO Sample normals. Face normals are not smooth enough
    // TODO: Investigate UE4 TSAA. 
    // TODO: Tidy this class it's getting mad.
    // TODO: Potentially separate TSAA convergence into separate pass.
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
        m_scene = std::make_shared<Scene>();
        m_scene->Initialize();

        CreateGlobalOptions();
        // If defined attach the root scene element added callback.
        if (createOptions.RootSceneElementAddedCallback != nullptr)
        {
            createOptions.RootSceneElementAddedCallback(m_globalOptions, 0);
            m_scene->GetRootNode()->SetChildAddedCallback(createOptions.RootSceneElementAddedCallback);
        }

        // Initialize light manager
        m_lightManager.Initialize(m_direct3D->GetDevice());

        // Add lights to the scene
        auto lightNode = m_scene->AddNode();
        lightNode->SetPosition({ -1.0f, 1.0f, 1.0f });
        auto light = lightNode->AddComponent<Light>(m_direct3D->GetDevice());
        light->SetColor({ DirectX::Colors::Wheat });
        light->SetDirection({ -1.0f, 1.0f, 1.0f });

        lightNode = m_scene->AddNode();
        lightNode->SetPosition({ 1.0f, -1.0f, -1.0f });
        light = lightNode->AddComponent<Light>(m_direct3D->GetDevice());
        light->SetColor({ DirectX::Colors::Wheat });
        light->SetDirection({ 1.0f, -1.0f, -1.0f });

        lightNode = m_scene->AddNode();
        lightNode->SetPosition({ 0.0f, 0.0f, 1.0f });
        light = lightNode->AddComponent<Light>(m_direct3D->GetDevice());
        light->SetColor({ DirectX::Colors::Wheat });
        light->SetDirection({ 0.0f, 0.0f, 1.0f });

        lightNode = m_scene->AddNode();
        lightNode->SetPosition({ 0.0f, 0.0f, -1.0f });
        light = lightNode->AddComponent<Light>(m_direct3D->GetDevice());
        light->SetColor({ DirectX::Colors::Wheat });
        light->SetDirection({ 0.0f, 0.0f, -1.0f });

        // Create the camera object
        auto cameraNode = m_scene->AddNode();
        m_camera = cameraNode->AddComponent<Camera>(m_direct3D->GetDevice());
        m_camera->SetJitterEnabled(true);
        m_camera->SetJitterSequence(kJitterSequence);
        cameraNode->SetPosition({ 0.0f, 0.0f, -10.0f });

        // Create post processing camera
        auto postCameraNode = m_scene->AddNode();
        m_postProcessCamera = postCameraNode->AddComponent<PostProcessingCamera>(m_direct3D->GetDevice());

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
        m_depthSampler = std::make_shared<Sampler>(m_direct3D->GetDevice(), D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP);

        InitializeScene();
        return true;
    }

    void Engine::InitializeScene()
    {
    }

    void Engine::CreateGlobalOptions()
    {
        // Set initialvalues for constants
        m_debugConstants.aoRadius = 1.0f; // 1 metre
        m_debugConstants.aoBias = 0.01f; // 1 cm
        m_debugConstants.aoIntensity = 1.0f;
        m_debugConstants.numAOSamples = kAO_numSamples;
        m_debugConstants.numAOSpiralTurns = float(kAO_numSpiralTurns);

        m_deepGBufferData.minimumSeparation = 0.3f;

        m_globalOptions = std::make_shared<SceneElement>(L"Global Options");

        {
            auto getter = [&]()->bool { return m_debugConstants.displaySecondLayer == 1.0f; };
            auto setter = [&](bool value) {m_debugConstants.displaySecondLayer = value ? 1.0f : 0.0f; };
            m_globalOptions->RegisterBooleanProperty(L"Display Second Layer", getter, setter);
        }

        {
            auto getter = [&]()->float { return m_debugConstants.gBufferTargetIndex; };
            auto setter = [&](float value) {m_debugConstants.gBufferTargetIndex = value; };
            m_globalOptions->RegisterScalarProperty(L"RenderTarget index", getter, setter, 0.0f, 5.0f);
        }

        {
            auto getter = [&]()->float { return m_deepGBufferData.minimumSeparation; };
            auto setter = [&](float value) {m_deepGBufferData.minimumSeparation = value; };
            m_globalOptions->RegisterScalarProperty(L"Minimum Separation", getter, setter, 0.0f, 1.0f);
        }

        {
            auto getter = [&]()->float { return m_debugConstants.aoRadius; };
            auto setter = [&](float value) {m_debugConstants.aoRadius = value; };
            m_globalOptions->RegisterScalarProperty(L"AO Radius", getter, setter, 0.0f, 4.0f);
        }

        {
            auto getter = [&]()->float { return m_debugConstants.aoBias; };
            auto setter = [&](float value) {m_debugConstants.aoBias = value; };
            m_globalOptions->RegisterScalarProperty(L"AO Bias", getter, setter, 0.0f, 0.1f);
        }

        {
            auto getter = [&]()->float { return m_debugConstants.aoIntensity; };
            auto setter = [&](float value) {m_debugConstants.aoIntensity = value; };
            m_globalOptions->RegisterScalarProperty(L"AO Intensity", getter, setter, 1.0f, 4.0f);
        }

        {
            auto getter = [&]()->float { return m_debugConstants.aoUseSecondLayer; };
            auto setter = [&](float value) {m_debugConstants.aoUseSecondLayer = value; };
            m_globalOptions->RegisterScalarProperty(L"AO Use second layer", getter, setter, 0.0f, 1.0f);
        }

        {
            auto getter = [&]()->bool { return m_camera->GetJitterEnabled(); };
            auto setter = [&](bool value) {m_camera->SetJitterEnabled(value); };
            m_globalOptions->RegisterBooleanProperty(L"Camera Jitter Enabled", getter, setter);
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

        return result;
    }

    void Engine::ResizeBuffers(uint32_t newWidth, uint32_t newHeight)
    {
        EngineAssert(newWidth > 0);
        EngineAssert(newHeight > 0);
        m_direct3D->ResizeBuffers(newWidth, newHeight);

        // Re-create G-Buffer with new dimensions
        // TODO: Find a way to create a frame buffer where we create multiple
        // render targets to draw to each of the different mip levels of a texture.
        // This is required for generating the hierarchical Z buffer required for AO and SSR.
        const uint32_t GBufferLayers = 2; // TODO: Formalise this
        RenderTargetBundle::Ptr GBuffer = std::make_shared<RenderTargetBundle>(m_direct3D->GetDevice(), newWidth, newHeight, GBufferLayers);
        GBuffer->CreateRenderTarget(L"Main", DXGI_FORMAT_R8G8B8A8_UNORM);
        GBuffer->CreateRenderTarget(L"Normal", DXGI_FORMAT_R8G8B8A8_UNORM);
        GBuffer->CreateRenderTarget(L"SSVelocity", DXGI_FORMAT_R16G16_FLOAT, {1.0f, 1.0f, 1.0f, 1.0f});
        GBuffer->CreateRenderTarget(L"CSZ", DXGI_FORMAT_R32_FLOAT);
        GBuffer->Finalise();

        // Set the G-Buffer for output from camera
        m_camera->SetRenderTargetBundle(GBuffer);

        // Create bundle for hierarchical Z
        m_hiZBundle = std::make_shared<RenderTargetBundle>(m_direct3D->GetDevice(), newWidth, newHeight, 1, kHiZ_MaxMip + 1, false);
        m_hiZBundle->CreateRenderTarget(L"Hi-Z", DXGI_FORMAT_R32G32_FLOAT);
        m_hiZBundle->Finalise();

        auto Hi_ZTex = m_hiZBundle->GetRenderTarget(L"Hi-Z")->GetTexture();
        m_hiZMipView = std::make_shared<TextureMipView>(m_direct3D->GetDevice(), Hi_ZTex, kHiZ_MaxMip + 1);

        m_aoBundle = std::make_shared<RenderTargetBundle>(m_direct3D->GetDevice(), newWidth, newHeight, 1, 0, false);
        m_aoBundle->CreateRenderTarget(L"AO", DXGI_FORMAT_R8G8B8A8_UNORM);
        m_aoBundle->Finalise();

        m_blurBundle = std::make_shared<RenderTargetBundle>(m_direct3D->GetDevice(), newWidth, newHeight, 1, 0, false);
        m_blurBundle->CreateRenderTarget(L"Blurred Texture", DXGI_FORMAT_R8G8B8A8_UNORM);
        m_blurBundle->Finalise();
    }


    void Engine::LoadFile(const uint8_t * data, uint64_t byteCount)
    {
        auto shaderPipeline = m_shaderManager->GetShaderPipeline(ShaderName::DeepGBuffer_Gen);
        Loader::Ptr loader = std::make_shared<Loader>(m_direct3D, m_scene, shaderPipeline);

        loader->LoadFile(data, byteCount);
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


        // Now need to copy depth
        auto bundle = m_camera->GetRenderTargetBundle();
        if (bundle != nullptr)
        {
            m_prevDepth = m_direct3D->CopyTexture(bundle->GetDepthBuffer()->GetTexture());

            // Take the camera-space Z texture and downsample into the mips
            auto HiZTexture = bundle->GetRenderTarget(L"CSZ")->GetTexture();
            GenerateHiZ(HiZTexture);

            // Generate the ambient occlusion
            GenerateAO();

            // Set render target to back buffer
            m_postProcessCamera->SetRenderTargetBundle(nullptr);
            auto aoTarget = m_aoBundle->GetRenderTarget(L"AO");

            // Upload G-buffer data to device
            bundle->SetShaderResources(m_direct3D->GetDeviceContext());
            aoTarget->GetTexture()->UploadData(m_direct3D->GetDeviceContext(), PipelineStage::Pixel, 5);
            if (m_prevFrame)
            {
                m_prevFrame->UploadData(m_direct3D->GetDeviceContext(), PipelineStage::Pixel, 6);
                m_debugConstants.temporalBlendWeight = 0.5f;// 1.0f / float(kTemporalAASamples);
            }

            // Set post effect constants
            m_postEffect->SetEffectData(m_debugConstants);

            // Upload light data for deferred shading
            m_lightManager.GatherLights(m_scene, m_direct3D->GetDeviceContext(), LightSpaceModifier::Camera);

            // Now need fullscreen pass to process G-Buffer
            m_postProcessCamera->RenderPostEffect(m_direct3D, m_postEffect);

        }

        // Present the rendered scene to the screen.
        m_direct3D->EndScene();
        m_prevFrame = m_direct3D->CopyBackBuffer();

        return true;
    }

    // Generates the hierachical Z buffer
    void Engine::GenerateHiZ(Texture::Ptr csZTexture)
    {
        m_hiZBundle->Clear(m_direct3D->GetDeviceContext());
        m_hiZBundle->SetTargetMip(0);
        // Create post effect
        auto csZCopyPipeline = m_shaderManager->GetShaderPipeline(ShaderName::DeepGBuffer_csZCopy);
        auto csZCopyEffect = std::make_shared<PostEffect<PostEffectConstants>>(m_direct3D->GetDevice(), csZCopyPipeline);

        auto HiZConstruction = m_shaderManager->GetShaderPipeline(ShaderName::Generate_HiZ);
        auto HiZPost = std::make_shared<PostEffect<PostEffectConstants>>(m_direct3D->GetDevice(), HiZConstruction);

        // From deep G to single texture
        m_postProcessCamera->SetRenderTargetBundle(m_hiZBundle);
        csZTexture->UploadData(m_direct3D->GetDeviceContext(), PipelineStage::Pixel, 0);
        m_depthSampler->UploadData(m_direct3D->GetDeviceContext(), 0);
        m_postProcessCamera->RenderPostEffect(m_direct3D, csZCopyEffect);


        PostEffectConstants constants;
        for (int i = 1; i <= kHiZ_MaxMip; i++)
        {
            // Set current render to target mip i.
            // Pass mip i-1 to shader
            // Render
            m_direct3D->UnbindAllRenderTargets();
            // render to second mip
            constants.currentMipLevel = float(i);
            HiZPost->SetEffectData(constants);
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
        auto aoEffect = std::make_shared<PostEffect<PostEffectConstants>>(m_direct3D->GetDevice(), aoPipeline);
        aoEffect->SetEffectData(m_debugConstants);

        // Upload hierarchical Z
        m_hiZBundle->SetShaderResources(m_direct3D->GetDeviceContext());

        // Render AO effect
        m_postProcessCamera->SetRenderTargetBundle(m_aoBundle);
        m_postProcessCamera->RenderPostEffect(m_direct3D, aoEffect);

        BlurAO();
    }

    void Engine::BlurAO()
    {
        m_postProcessCamera->SetRenderTargetBundle(m_blurBundle);
        auto blurPipeline = m_shaderManager->GetShaderPipeline(ShaderName::BilateralBlur);
        auto blurEffect = std::make_shared<PostEffect<PostEffectConstants>>(m_direct3D->GetDevice(), blurPipeline);

        // vertical blur
        m_debugConstants.blurAxis = { 0.0f, 1.0f };
        blurEffect->SetEffectData(m_debugConstants);
        m_aoBundle->SetShaderResources(m_direct3D->GetDeviceContext());
        m_postProcessCamera->RenderPostEffect(m_direct3D, blurEffect);

        // Set camera to render back to the original AO texture
        m_postProcessCamera->SetRenderTargetBundle(m_aoBundle);

        // horizontal blur
        // bind old ao render target
        m_debugConstants.blurAxis = { 1.0f, 0.0f };
        blurEffect->SetEffectData(m_debugConstants);
        m_blurBundle->SetShaderResources(m_direct3D->GetDeviceContext());
        m_postProcessCamera->RenderPostEffect(m_direct3D, blurEffect);
    }

}