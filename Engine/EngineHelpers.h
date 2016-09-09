#pragma once
// INCLUDES //
#include <exception>
#include <string>
#include <d3d11.h>
#include <sal.h>

namespace Utils
{
    #define EngineAssert(x) (!!(x) || (__debugbreak(), 0))


    inline void ThrowIfFailed(HRESULT result)
    {
        if (FAILED(result))
        {
            // Throw a breakpoint to catch Win32 errors
            throw std::exception();
        }
    }

    inline std::string GetAssetsPath()
    {
        CHAR path[512];
        DWORD pathSize = static_cast<DWORD>(sizeof(path));
        DWORD size = GetModuleFileName(nullptr, path, pathSize);

        if (size == 0 || size == pathSize)
        {
            // Method failed or path was truncated.
            throw;
        }

        CHAR* lastSlash = strrchr(path, '\\');
        if (lastSlash)
        {
            *(lastSlash + 1) = NULL;
        }

        std::string stringPath(path);
        return stringPath;
    }
}