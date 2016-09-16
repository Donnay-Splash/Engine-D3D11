#include "pch.h"
#include "VMShaderClass.h"
#include "ConstantBuffer.h"
// Include compiled shader files

VMShaderClass::VMShaderClass()
{
    m_timer = 0;
    m_elapsedTime = 0;
}

VMShaderClass::VMShaderClass(const VMShaderClass& other)
{
}


VMShaderClass::~VMShaderClass()
{
}


bool VMShaderClass::Initialize(ID3D11Device* device, ShaderManager::Ptr shaderManager)
{
    bool result;

    // Initialize the vertex and pixel shaders.
    result = InitializeShader(device, shaderManager);
    if(!result)
    {
        return false;
    }

    return true;
}


void VMShaderClass::Shutdown()
{
    // Shutdown the vertex and pixel shaders as well as the related objects.
    ShutdownShader();

    return;
}

bool VMShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, Utils::Maths::Matrix worldMatrix, Utils::Maths::Matrix viewMatrix,
    Utils::Maths::Matrix projectionMatrix, ID3D11ShaderResourceView* texture, LightClass* mlight, float height_in, float length_in)
{
    bool result;

    m_height = height_in;
    m_length = length_in;
    // Set the shader parameters that it will use for rendering.
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, mlight);
    if(!result)
    {
        return false;
    }

    // Now render the prepared buffers with the shader.
    RenderShader(deviceContext, indexCount);

    return true;
}


bool VMShaderClass::InitializeShader(ID3D11Device* device, ShaderManager::Ptr shaderManager)
{
    m_vmShaderPipeline = shaderManager->GetShaderPipeline(ShaderName::VertexManipulation);

    m_sampler = std::make_shared<Sampler>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);

    m_matrixBuffer = std::make_shared<ConstantBuffer<MatrixBuffer>>(0, PipelineStage::Vertex, device);
    m_timeBuffer = std::make_shared<ConstantBuffer<TimeBuffer>>(1, PipelineStage::Vertex, device);
    m_lightBuffer = std::make_shared<ConstantBuffer<LightBuffer>>(0, PipelineStage::Pixel, device);

    return true;
}


void VMShaderClass::ShutdownShader()
{
}


bool VMShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, Utils::Maths::Matrix worldMatrix, Utils::Maths::Matrix viewMatrix,
    Utils::Maths::Matrix projectionMatrix, ID3D11ShaderResourceView* texture, LightClass* light)
{
    MatrixBuffer matrixData{ viewMatrix, projectionMatrix };
    m_matrixBuffer->SetData(matrixData);
    m_matrixBuffer->UploadData(deviceContext);

    time_t prevTime = m_timer;
    time(&m_timer);  /* get current time; same as: timer = time(NULL)  */
    m_elapsedTime += 0.1f;

    TimeBuffer timeData{ m_elapsedTime, m_height, m_length };
    m_timeBuffer->SetData(timeData);
    m_timeBuffer->UploadData(deviceContext);

    LightBuffer lightData{ light->GetAmbientColor(), light->GetDiffuseColor(), light->GetDirection() };
    m_lightBuffer->SetData(lightData);
    m_lightBuffer->UploadData(deviceContext);

    // Set shader texture resource in the pixel shader.
    deviceContext->PSSetShaderResources(0, 1, &texture);

    return true;
}


void VMShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
     
    return;
}
