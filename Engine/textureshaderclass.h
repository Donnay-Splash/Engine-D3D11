#pragma once

#include <fstream>
#include "Texture.h"


class TextureShaderClass
{
private:
    struct MatrixBufferType
    {
        Utils::Maths::Matrix world;
        Utils::Maths::Matrix view;
        Utils::Maths::Matrix projection;
    };
    Texture* text;

public:
    TextureShaderClass();
    TextureShaderClass(const TextureShaderClass&);
    ~TextureShaderClass();

    bool Initialize(ID3D11Device*, HWND);
    void Shutdown();
    bool Render(ID3D11DeviceContext*, int, Utils::Maths::Matrix, Utils::Maths::Matrix, Utils::Maths::Matrix, ID3D11ShaderResourceView*);

private:
    bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
    void ShutdownShader();

    bool SetShaderParameters(ID3D11DeviceContext*, Utils::Maths::Matrix, Utils::Maths::Matrix, Utils::Maths::Matrix, ID3D11ShaderResourceView*);
    void RenderShader(ID3D11DeviceContext*, int);

private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_layout;
    ID3D11Buffer* m_matrixBuffer;

    ID3D11SamplerState* m_sampleState;
};

