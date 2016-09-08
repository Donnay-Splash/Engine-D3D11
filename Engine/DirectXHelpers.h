#pragma once

// INCLUDES //
#include <collection.h>
#include <d3d12.h>
#include <sal.h>

namespace
{
    
}

namespace Engine_D3D12
{
namespace DirectXHelpers
{
    inline void ThrowIfFailed(HRESULT result)
    {
        if (FAILED(result))
        {
            // Throw a breakpoint to catch Win32 errors
            throw Platform::Exception::CreateException(result);
        }
    }

    inline std::wstring GetAssetsPath()
    {
        WCHAR path[512];
        DWORD pathSize = static_cast<DWORD>(sizeof(path));
        DWORD size = GetModuleFileName(nullptr, path, pathSize);

        if (size == 0 || size == pathSize)
        {
            // Method failed or path was truncated.
            throw;
        }

        WCHAR* lastSlash = wcsrchr(path, L'\\');
        if (lastSlash)
        {
            *(lastSlash + 1) = NULL;
        }

        std::wstring stringPath(path);
        return stringPath;
    }
}
}