#include "pch.h"
#include "Engine.h"
#include <MeshMaker\MeshMaker.h>
#include <Scene\Components\MeshInstance.h>
#include <Loader.h>

namespace Engine
{

    // TODO: Attempt to load complex models including Sponza and potentially san-miguel
    // TODO: Implement deferred shading using same BRDF from forward shader
    // TODO: Continue to maintain debug controls
    // TODO: Re-read paper. Specifically double check sections about G-Buffer generation
    // TODO: Map out tasks required for computing AO
    // TODO: Begin work on AO
    // TODO: Make switching between different shading models easier. Need way of controlling what is expected as shader constants
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
        m_globalOptions = std::make_shared<SceneElement>(L"Global Options");

        {
            auto getter = [&]()->bool { return m_debugConstants.displaySecondLayer == 1.0f; };
            auto setter = [&](bool value) {m_debugConstants.displaySecondLayer = value ? 1.0f : 0.0f; };
            m_globalOptions->RegisterBooleanProperty(L"Display Second Layer", getter, setter);
        }

        {
            auto getter = [&]()->float { return m_debugConstants.gBufferTargetIndex; };
            auto setter = [&](float value) {m_debugConstants.gBufferTargetIndex = value; };
            m_globalOptions->RegisterScalarProperty(L"RenderTarget index", getter, setter, 0.0f, 4.0f);
        }

        {
            auto getter = [&]()->float { return m_deepGBufferData.minimumSeparation; };
            auto setter = [&](float value) {m_deepGBufferData.minimumSeparation = value; };
            m_globalOptions->RegisterScalarProperty(L"Minimum Separation", getter, setter, 0.0f, 1.0f);
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
        const uint32_t GBufferLayers = 2; // TODO: Formalise this
        RenderTargetBundle::Ptr bundle = std::make_shared<RenderTargetBundle>(m_direct3D->GetDevice(), newWidth, newHeight, GBufferLayers);
        bundle->CreateRenderTarget(L"Main", DXGI_FORMAT_R8G8B8A8_UNORM);
        bundle->CreateRenderTarget(L"Normal", DXGI_FORMAT_R8G8B8A8_UNORM);
        bundle->CreateRenderTarget(L"SSVelocity", DXGI_FORMAT_R16G16_FLOAT);
        bundle->CreateRenderTarget(L"CSZ", DXGI_FORMAT_R16_FLOAT);
        bundle->Finalise();

        // Set the G-Buffer for output from camera
        m_camera->SetRenderTargetBundle(bundle);
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

        // Now need to copy depth
        auto bundle = m_camera->GetRenderTargetBundle();
        if (bundle != nullptr)
        {
            m_prevDepth = m_direct3D->CopyTexture(bundle->GetDepthBuffer()->GetTexture());

            // Upload G-buffer data to device
            bundle->SetShaderResources(m_direct3D->GetDeviceContext());

            // Set post effect constants
            m_postEffect->SetEffectData(m_debugConstants);

            // Upload light data for deferred shading
            m_lightManager.GatherLights(m_scene, m_direct3D->GetDeviceContext(), LightSpaceModifier::Camera);

            // Now need fullscreen pass to process G-Buffer
            m_postProcessCamera->RenderPostEffect(m_direct3D, m_postEffect);
        }

        // Present the rendered scene to the screen.
        m_direct3D->EndScene();

        return true;
    }

}