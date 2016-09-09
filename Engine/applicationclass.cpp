#include "pch.h"
#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	m_Input = 0;
	m_Direct3D = 0;
	m_Camera = 0;
	m_VMShader = 0;
	m_Light1 = 0;
	m_Timer = 0;
	m_Position = 0;
	m_Mesh = 0;
	m_Plane = 0;
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
    float cameraX, cameraY, cameraZ;
    Utils::Maths::Matrix baseViewMatrix;
    //char videoCard[128];
    //int videoMemory;

    plane_height = 0.0f;
    plane_length = 0.0f;

    // Create the input object.  The input object will be used to handle reading the keyboard and mouse input from the user.
    m_Input = new InputClass;
    if (!m_Input)
    {
        return false;
    }

    // Initialize the input object.
    result = m_Input->Initialize(hinstance, hwnd, screenWidth, screenHeight);
    if (!result)
    {
        MessageBox(hwnd, "Could not initialize the input object.", "Error", MB_OK);
        return false;
    }

    // Create the Direct3D object.
    m_Direct3D = new D3DClass;
    if (!m_Direct3D)
    {
        return false;
    }

    // Initialize the Direct3D object.
    result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
    if (!result)
    {
        MessageBox(hwnd, "Could not initialize DirectX 11.", "Error", MB_OK);
        return false;
    }

    // Create the camera object.
    m_Camera = new CameraClass;
    if (!m_Camera)
    {
        return false;
    }

    // Initialize a base view matrix with the camera for 2D user interface rendering.
    m_Camera->SetPosition({ 0.0f, 0.0f, -10.0f });
    m_Camera->Render();
    baseViewMatrix = m_Camera->GetViewMatrix();

    m_Light1 = new LightClass;
    if (!m_Light1)
    {
        return false;
    }
    m_Light1->SetAmbientColor({ 0.2f, 0.2f, 0.2f, 1.0f });
    m_Light1->SetDiffuseColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    m_Light1->SetDirection({ 0.0f, -1.0f, .0f });
    m_Light1->SetPosition({ 0.0f, 0.0f, -0.5f });
	//m_Light1->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
	//m_Light1->SetSpecularPower(1.0f);

	// Set the initial position of the camera.
	//cameraX = 50.0f;
	//cameraY = 2.0f;
	//cameraZ = -7.0f;

	//m_Camera->SetPosition(cameraX, cameraY, cameraZ);


	// Create Mesh object
	m_Mesh = new MeshClass;
	
	// Initialise the mesh object
	result = m_Mesh->Initialize(m_Direct3D->GetDevice(), L"data/brick1.dds");
	if(!result)
	{
		MessageBox(hwnd, "Could not initialise the mesh object.", "Error", MB_OK);
		return false;
	}

	m_Plane = new PlaneClass;

	result = m_Plane->Initialize(m_Direct3D->GetDevice(), L"data/brick1.dds");
	if(!result)
	{
		MessageBox(hwnd, "Could no initialise the plane object.", "Error", MB_OK);
		return false;
	}

	// Create the color shader object.
	m_VMShader = new VMShaderClass;
	if(!m_VMShader)
	{
		return false;
	}

	// Initialize the color shader object.
	result = m_VMShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, "Could not initialize the texture shader object.", "Error", MB_OK);
		return false;
	}

	// Create the timer object.
	m_Timer = new TimerClass;
	if(!m_Timer)
	{
		return false;
	}

	// Initialize the timer object.
	result = m_Timer->Initialize();
	if(!result)
	{
		MessageBox(hwnd, "Could not initialize the timer object.", "Error", MB_OK);
		return false;
	}

	// Create the position object.
	m_Position = new PositionClass;
	if(!m_Position)
	{
		return false;
	}

	// Set the initial position of the viewer to the same as the initial camera position.
	m_Position->SetPosition(0.0f, 0.0f, -10.0f);

	return true;
}


void ApplicationClass::Shutdown()
{
	
	// Release the position object.
	if(m_Position)
	{
		delete m_Position;
		m_Position = 0;
	}

	// Release the timer object.
	if(m_Timer)
	{
		delete m_Timer;
		m_Timer = 0;
	}

	// Release the color shader object.
	if(m_VMShader)
	{
		m_VMShader->Shutdown();
		delete m_VMShader;
		m_VMShader = 0;
	}

	// Release the mesh object
	if(m_Mesh)
	{
		m_Mesh->Shutdown();
		delete m_Mesh;
		m_Mesh = 0;
	}

	// Release the plane object
	if(m_Plane)
	{
		m_Plane->Shutdown();
		delete m_Plane;
		m_Plane = 0;
	}
	// Release the camera object.
	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the Direct3D object.
	if(m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}

	// Release the input object.
	if(m_Input)
	{
		m_Input->Shutdown();
		delete m_Input;
		m_Input = 0;
	}

	return;
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
    bool keyDown, result;
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
        plane_height -= 0.01;
    }
    if (m_Input->IsYPressed())
    {
        plane_height += 0.01;
    }
    if (m_Input->IsJPressed())
    {
        plane_length -= 0.01;
    }
    if (m_Input->IsUPressed())
    {
        plane_length += 0.01;
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
	bool result = m_VMShader->Render(m_Direct3D->GetDeviceContext(), m_Mesh->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_Plane->GetTexture(), m_Light1, plane_height, plane_length);
	if(!result)
	{
		return false;
	}

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}
