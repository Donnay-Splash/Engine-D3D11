#include "pch.h"
#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	rotation = 0.0f;
}


ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}


ApplicationClass::~ApplicationClass()
{
}


bool ApplicationClass::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
    bool result;
    Utils::Maths::Matrix baseViewMatrix;

    plane_height = 0.0f;
    plane_length = 0.0f;

    // Create the input object.  The input object will be used to handle reading the keyboard and mouse input from the user.
    m_Input = std::make_shared<InputClass>();

    // Initialize the input object.
    result = m_Input->Initialize(hinstance, hwnd, screenWidth, screenHeight);
    if (!result)
    {
        MessageBox(hwnd, "Could not initialize the input object.", "Error", MB_OK);
        return false;
    }

    // Create the Direct3D object.
    m_Direct3D = std::make_shared<D3DClass>();

    // Initialize the Direct3D object.
    result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
    if (!result)
    {
        MessageBox(hwnd, "Could not initialize DirectX 11.", "Error", MB_OK);
        return false;
    }

    // Create the camera object.
    m_Camera = std::make_shared<CameraClass>();

    // Initialize a base view matrix with the camera for 2D user interface rendering.
    m_Camera->SetPosition({ 0.0f, 0.0f, -10.0f });
    m_Camera->Render();
    baseViewMatrix = m_Camera->GetViewMatrix();

    m_Light1 = std::make_shared<LightClass>();
    m_Light1->SetAmbientColor({ 0.2f, 0.2f, 0.2f, 1.0f });
    m_Light1->SetDiffuseColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    m_Light1->SetDirection({ 0.0f, -1.0f, .0f });
    m_Light1->SetPosition({ 0.0f, 0.0f, -0.5f });
	//m_Light1->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
	//m_Light1->SetSpecularPower(1.0f);

	// Create Mesh object
	m_Mesh = std::make_shared<MeshClass>();
	
	// Initialise the mesh object
	result = m_Mesh->Initialize(m_Direct3D->GetDevice(), L"data/brick1.dds");
	if(!result)
	{
		MessageBox(hwnd, "Could not initialise the mesh object.", "Error", MB_OK);
		return false;
	}

	m_Plane = std::make_shared<PlaneClass>();

	result = m_Plane->Initialize(m_Direct3D->GetDevice(), L"data/brick1.dds");
	if(!result)
	{
		MessageBox(hwnd, "Could no initialise the plane object.", "Error", MB_OK);
		return false;
	}

	// Create the color shader object.
	m_VMShader = std::make_shared<VMShaderClass>();

	// Initialize the color shader object.
	result = m_VMShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, "Could not initialize the texture shader object.", "Error", MB_OK);
		return false;
	}

	// Create the timer object.
	m_Timer = std::make_shared<TimerClass>();

	// Initialize the timer object.
	result = m_Timer->Initialize();
	if(!result)
	{
		MessageBox(hwnd, "Could not initialize the timer object.", "Error", MB_OK);
		return false;
	}

	// Create the position object.
	m_Position = std::make_shared<PositionClass>();

	// Set the initial position of the viewer to the same as the initial camera position.
	m_Position->SetPosition(0.0f, 0.0f, -10.0f);

	return true;
}


void ApplicationClass::Shutdown()
{
}


bool ApplicationClass::Frame()
{
	bool result;

	rotation += Utils::Maths::kPI * 0.001f;
	if(rotation > 360.0f)
	{
		rotation -= 360.0f;
	}


	// Read the user input.
	result = m_Input->Frame();
	if(!result)
	{
		return false;
	}
	
	// Check if the user pressed escape and wants to exit the application.
	if(m_Input->IsEscapePressed() == true)
	{
		return false;
	}

	// Update the system stats.
	m_Timer->Frame();
	
	// Do the frame input processing.
	result = HandleInput(m_Timer->GetTime());
	if(!result)
	{
		return false;
	}

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
    m_Position->SetFrameTime(frameTime);

    // Handle the input.
    keyDown = m_Input->IsLeftPressed();
    m_Position->TurnLeft(keyDown);

    keyDown = m_Input->IsRightPressed();
    m_Position->TurnRight(keyDown);

    keyDown = m_Input->IsUpPressed();
    m_Position->MoveForward(keyDown);

    keyDown = m_Input->IsDownPressed();
    m_Position->MoveBackward(keyDown);

    keyDown = m_Input->IsAPressed();
    m_Position->MoveUpward(keyDown);

    keyDown = m_Input->IsZPressed();
    m_Position->MoveDownward(keyDown);

    keyDown = m_Input->IsPgUpPressed();
    m_Position->LookUpward(keyDown);

    keyDown = m_Input->IsPgDownPressed();
    m_Position->LookDownward(keyDown);

    if (m_Input->IsHPressed())
    {
        plane_height -= 0.01f;
    }
    if (m_Input->IsYPressed())
    {
        plane_height += 0.01f;
    }
    if (m_Input->IsJPressed())
    {
        plane_length -= 0.01f;
    }
    if (m_Input->IsUPressed())
    {
        plane_length += 0.01f;
    }

    // Get the view point position/rotation.
    m_Position->GetPosition(posX, posY, posZ);
    m_Position->GetRotation(rotX, rotY, rotZ);

    // Set the position of the camera.
    m_Camera->SetPosition({ posX, posY, posZ });
    auto rotation = Utils::Maths::Quaternion::CreateFromYawPitchRoll(rotY, rotX, rotZ);
    m_Camera->SetRotation(rotation);

	return true;
}


bool ApplicationClass::RenderGraphics()
{
	// Clear the scene.
	m_Direct3D->BeginScene(0.0f, 1.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	auto worldMatrix = m_Direct3D->GetWorldMatrix();
	auto viewMatrix = m_Camera->GetViewMatrix();
	auto projectionMatrix = m_Direct3D->GetProjectionMatrix();

	//D3DXMatrixRotationY(&worldMatrix, rotation);

	// Push mesh data onto gfx hardware
	m_Mesh->Render(m_Direct3D->GetDeviceContext());

	// Render the Mesh (data being pushed above) using the color shader.
	bool result = m_VMShader->Render(m_Direct3D->GetDeviceContext(), m_Mesh->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_Plane->GetTexture().Get(), m_Light1.get(), plane_height, plane_length);
	if(!result)
	{
		return false;
	}

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}
