#pragma once

#include "pch.h"

class IndexBuffer
{
public:
	using Ptr = std::shared_ptr<IndexBuffer>;
	IndexBuffer(void* data, uint32_t indexCount, ID3D11Device* device, bool bigIndices = true);

	void UploadData(ID3D11DeviceContext* deviceContext);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
	const UINT m_indexSizeInBytes;
	const uint32_t m_indexCount;
	const DXGI_FORMAT m_indexFormat;
};