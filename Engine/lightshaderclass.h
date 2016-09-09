#ifndef _LIGHTSHADERCLASS_H_
#define _LIGHTSHADERCLASS_H_

#include <d3d11.h>
#include <fstream>
#include "Math.h"
#include "lightclass.h"
using namespace std;


class LightShaderClass
{
private:
	struct MatrixBufferType
	{
		Utils::Maths::Matrix world;
        Utils::Maths::Matrix view;
        Utils::Maths::Matrix projection;
	};

	struct LightBufferType
	{
        Utils::Maths::Color ambientColor;
        Utils::Maths::Color diffuseColor;
        Utils::Maths::Vector3 lightDirection;
		float padding;
        Utils::Maths::Vector3 position;
		float padding2;
		//float specularPower;
		//D3DXVECTOR4 specularColor;	
	};

	struct CameraBufferType
	{
        Utils::Maths::Vector3 CameraPosition;
		float padding;
	};

public:
	LightShaderClass();
	LightShaderClass(const LightShaderClass&);
	~LightShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, Utils::Maths::Matrix, Utils::Maths::Matrix, Utils::Maths::Matrix, ID3D11ShaderResourceView*, LightClass*, Utils::Maths::Vector3);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();

	bool SetShaderParameters(ID3D11DeviceContext*, Utils::Maths::Matrix, Utils::Maths::Matrix, Utils::Maths::Matrix, ID3D11ShaderResourceView*, LightClass*, Utils::Maths::Vector3);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_matrixBuffer;

	ID3D11Buffer* m_lightBuffer;

	ID3D11Buffer* m_cameraBuffer;
};

#endif