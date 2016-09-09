#ifndef _VMSHADERCLASS_H_
#define _VMSHADERCLASS_H_

#include <d3d11.h>
#include <fstream>
using namespace std;
#include <time.h>
#include "lightclass.h"


class VMShaderClass
{
private:
	struct MatrixBufferType
	{
		Utils::Maths::Matrix world;
        Utils::Maths::Matrix view;
        Utils::Maths::Matrix projection;
	};

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

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, Utils::Maths::Matrix, Utils::Maths::Matrix, Utils::Maths::Matrix, ID3D11ShaderResourceView*, LightClass*, float, float);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, Utils::Maths::Matrix, Utils::Maths::Matrix, Utils::Maths::Matrix, ID3D11ShaderResourceView*, LightClass*);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_matrixBuffer;

	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_timeBuffer;

	time_t timer;
	float elapsedTime;
	float height;
	float length;
};

#endif