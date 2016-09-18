#include "pch.h"
#include "applicationclass.h"
#include "MeshMaker.h"
#include "MeshInstance.h"

ApplicationClass::ApplicationClass()
{
}


ApplicationClass::~ApplicationClass()
{
}


bool ApplicationClass::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
    bool result;

    // Create the input object.  The input object will be used to handle reading the keyboard and mouse input from the user.
    m_input = std::make_shared<InputClass>();

    // Initialize the input object.
    result = m_input->Initialize(hinstance, hwnd, screenWidth, screenHeight);
    if (!result)
    {
        MessageBox(hwnd, "Could not initialize the input object.", "Error", MB_OK);
        return false;
    }

    // Create the Direct3D object.
    m_direct3D = std::make_shared<D3DClass>();

    // Initialize the Direct3D object.
    m_direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);

    // Initialise the shader manager
    m_shaderManager = std::make_shared<ShaderManager>(m_direct3D->GetDevice());

    // Create the camera object.
    m_light1 = std::make_shared<LightClass>();
    m_light1->SetAmbientColor({ 0.2f, 0.2f, 0.2f, 1.0f });
    m_light1->SetDiffuseColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    m_light1->SetDirection({ 0.0f, -1.0f, .0f });
    m_light1->SetPosition({ 0.0f, 0.0f, -0.5f });

    // Create Mesh object
    m_mesh = Utils::MeshMaker::CreateCube(m_direct3D->GetDevice());

    // Initialise Scene
    m_scene = std::make_shared<Scene>();
    m_scene->Initialize();

    // Create the camera object
    auto cameraNode = m_scene->AddNode();
    m_camera = cameraNode->AddComponent<Camera>(m_direct3D->GetDevice());

    // Create the mesh object and add it to the scene.
    auto meshNode = m_scene->AddNode();
    auto meshInstance = meshNode->AddComponent<MeshInstance>(m_direct3D->GetDevice());
    auto shaderPipeline = m_shaderManager->GetShaderPipeline(ShaderName::VertexManipulation);
    auto material = std::make_shared<Material>(m_direct3D->GetDevice(), shaderPipeline);
    meshInstance->SetMesh(m_mesh);
    meshInstance->SetMaterial(material);

    // Create the color shader object.
    m_vmShader = std::make_shared<VMShaderClass>();

    // Initialize the color shader object.
    result = m_vmShader->Initialize(m_direct3D->GetDevice(), m_shaderManager);
    if(!result)
    {
        MessageBox(hwnd, "Could not initialize the texture shader object.", "Error", MB_OK);
        return false;
    }

    // Create the timer object.
    m_timer = std::make_shared<TimerClass>();

    // Initialize the timer object.
    result = m_timer->Initialize();
    if(!result)
    {
        MessageBox(hwnd, "Could not initialize the timer object.", "Error", MB_OK);
        return false;
    }

    // Create the position object.
    m_position = std::make_shared<PositionClass>();

    // Set the initial position of the viewer to the same as the initial camera position.
    m_position->SetPosition(0.0f, 0.0f, -10.0f);

    return true;
}


bool ApplicationClass::Frame()
{
    bool result;

    // Read the user input.
    result = m_input->Frame();
    if(!result)
    {
        return false;
    }
    
    // Check if the user pressed escape and wants to exit the application.
    if(m_input->IsEscapePressed() == true)
    {
        return false;
    }

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


bool ApplicationClass::HandleInput(float frameTime)
{
    bool keyDown;
    float posX, posY, posZ, rotX, rotY, rotZ;


    // Set the frame time for calculating the updated position.
    m_position->SetFrameTime(frameTime);

    // Handle the input.
    keyDown = m_input->IsLeftPressed();
    m_position->TurnLeft(keyDown);

    keyDown = m_input->IsRightPressed();
    m_position->TurnRight(keyDown);

    keyDown = m_input->IsUpPressed();
    m_position->MoveForward(keyDown);

    keyDown = m_input->IsDownPressed();
    m_position->MoveBackward(keyDown);

    keyDown = m_input->IsAPressed();
    m_position->MoveUpward(keyDown);

    keyDown = m_input->IsZPressed();
    m_position->MoveDownward(keyDown);

    keyDown = m_input->IsPgUpPressed();
    m_position->LookUpward(keyDown);

    keyDown = m_input->IsPgDownPressed();
    m_position->LookDownward(keyDown);

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


bool ApplicationClass::RenderGraphics()
{
    // Clear the scene.
    m_direct3D->BeginScene(0.0f, 1.0f, 0.0f, 1.0f);

    // Generate the view matrix based on the camera's position.
    m_camera->Render(m_direct3D, m_scene);

    // Present the rendered scene to the screen.
    m_direct3D->EndScene();

    return true;
}
