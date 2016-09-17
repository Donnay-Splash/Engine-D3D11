#pragma once

#include <fstream>
using namespace std;
#include "lightclass.h"
#include "ShaderManager.h"
#include "ConstantBufferLayouts.h"
#include "ConstantBuffer.h"
#include "Sampler.h"

class VMShaderClass
{
private:

    struct TimeBufferType
    {
        float time;
        float height;
        float length;
        float padding;
    };

    struct LightBufferType
    {
        Utils::Maths::Color ambientColor;
        Utils::Maths::Color diffuseColor;
        Utils::Maths::Vector3 lightDirection;
        float padding;
    };

public:
    VMShaderClass();
    VMShaderClass(const VMShaderClass&);
    ~VMShaderClass();

    bool Initialize(ID3D11Device*, ShaderManager::Ptr shaderManager);
    void Shutdown();

    // This function should become part of an interface so that all rendering can be done in a single call to multiple objects.
    bool Render(ID3D11DeviceContext*, int, Utils::Maths::Matrix, Utils::Maths::Matrix, Utils::Maths::Matrix, ID3D11ShaderResourceView*, LightClass*, float, float);

private:
    bool InitializeShader(ID3D11Device*, ShaderManager::Ptr shaderManager);
    void ShutdownShader();

    bool SetShaderParameters(ID3D11DeviceContext*, Utils::Maths::Matrix, Utils::Maths::Matrix, Utils::Maths::Matrix, ID3D11ShaderResourceView*, LightClass*);
    void RenderShader(ID3D11DeviceContext*, int);

private:
    // Create shader pipeline object to manage
    // shaders and samplers and input layout.
    ShaderPipeline::Ptr m_vmShaderPipeline;
    Sampler::Ptr m_sampler;

    // Need some sort of constant buffer class.
    // How we manage these internally I am unsure.
    ConstantBuffer<ViewConstants>::Ptr m_matrixBuffer;
    ConstantBuffer<LightBuffer>::Ptr m_lightBuffer;
    ConstantBuffer<TimeBuffer>::Ptr m_timeBuffer;

    // This is technique/shader specific.
    time_t m_timer;
    float m_elapsedTime;
    float m_height;
    float m_length;
};

