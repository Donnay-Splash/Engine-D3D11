#include "pch.h"
#include "Engine.h"
#include "MeshMaker.h"
#include "MeshInstance.h"
#include "Loader.h"

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
    m_direct3D->Initialize(createOptions, SCREEN_DEPTH, SCREEN_NEAR);

    // Initialise the shader manager
    m_shaderManager = std::make_shared<ShaderManager>(m_direct3D->GetDevice());

    // Create Mesh object
    m_mesh = Utils::MeshMaker::CreateCube(m_direct3D->GetDevice());

    // Initialise Scene
    m_scene = std::make_shared<Scene>();
    m_scene->Initialize();

    // Initialize light manager
    m_lightManager.Initialize(m_direct3D->GetDevice());

    // Add lights to the scene
    auto lightNode = m_scene->AddNode();
    lightNode->SetPosition({ -1.0f, -1.0f, -1.0f });
    auto light = lightNode->AddComponent<Light>(m_direct3D->GetDevice());
    light->SetColor({ DirectX::Colors::Yellow });

    lightNode = m_scene->AddNode();
    lightNode->SetPosition({ -1.0f, 1.0f, -1.0f });
    light = lightNode->AddComponent<Light>(m_direct3D->GetDevice());
    light->SetColor({ DirectX::Colors::Blue });

    lightNode = m_scene->AddNode();
    lightNode->SetPosition({ 1.0f, 1.0f, 1.0f });
    light = lightNode->AddComponent<Light>(m_direct3D->GetDevice());
    light->SetColor({ DirectX::Colors::Green });

    lightNode = m_scene->AddNode();
    lightNode->SetPosition({ 1.0f, -1.0f, 1.0f });
    light = lightNode->AddComponent<Light>(m_direct3D->GetDevice());
    light->SetColor({ DirectX::Colors::Red });

    // Create the camera object
    auto cameraNode = m_scene->AddNode();
    m_camera = cameraNode->AddComponent<Camera>(m_direct3D->GetDevice());
    cameraNode->SetPosition({0.0f, 0.0f, -10.0f});

    // Create the mesh object and add it to the scene.
    auto meshNode = m_scene->AddNode();
    auto meshInstance = meshNode->AddComponent<MeshInstance>(m_direct3D->GetDevice());
    auto shaderPipeline = m_shaderManager->GetShaderPipeline(ShaderName::Uber);
    auto material = std::make_shared<Material>(m_direct3D->GetDevice(), shaderPipeline);
    meshInstance->SetMesh(m_mesh);
    meshInstance->SetMaterial(material);

    // Create the timer object.
    m_timer = std::make_shared<TimerClass>();

    // Initialize the timer object.
    result = m_timer->Initialize();
    EngineAssert(result);

    // Create the position object.
    m_position = std::make_shared<PositionClass>();

    // Set the initial position of the viewer to the same as the initial camera position.
    m_position->SetPosition(0.0f, 0.0f, -10.0f);

    InitializeScene();
    return true;
}

void Engine::InitializeScene()
{
    auto shaderPipeline = m_shaderManager->GetShaderPipeline(ShaderName::Uber);
    Loader::Ptr loader = std::make_shared<Loader>(m_direct3D, m_scene, shaderPipeline);

    loader->LoadFile(R"(E:\GitHub\Engine-D3D11\Assets\teapot\teapot.obj)");
}

void Engine::SetFrameInput(InputState newInputState)
{
    m_inputManager->Update(newInputState);
}

bool Engine::Frame()
{
    bool result;
    
    // Update the system stats.
    m_timer->Frame();
    
    // Do the frame input processing.
    result = HandleInput(m_timer->GetTime());
    if(!result)
    {
        return false;
    }

    // Update the scene
    m_scene->Update(m_timer->GetTime());

    // Render the graphics.
    result = RenderGraphics();
    if(!result)
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
    m_direct3D->BeginScene(0.0f, 1.0f, 0.0f, 1.0f);

    m_lightManager.GatherLights(m_scene, m_direct3D->GetDeviceContext());
    // Generate the view matrix based on the camera's position.
    m_camera->Render(m_direct3D, m_scene);

    // Present the rendered scene to the screen.
    m_direct3D->EndScene();

    return true;
}