#include "pch.h"
#include <Resources\ShaderResource.h>
#include "d3dClass.h"

namespace Engine
{
	ShaderResource::~ShaderResource()
	{
		if (m_descriptor != nullptr)
		{
			D3DClass::Instance()->FreeDescriptor(m_descriptor);
		}
	}
}