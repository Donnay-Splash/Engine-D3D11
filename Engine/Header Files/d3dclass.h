#pragma once

//////////////
// INCLUDES //
//////////////
#include "EngineCreateOptions.h"
#include <RenderTargetBundle.h>

namespace Engine
{
    // Forward Declarations
    struct EngineCreateOptions;

    class D3DClass
    {
    public:
        using Ptr = std::shared_ptr<D3DClass>;
        D3DClass();
        D3DClass(const D3DClass&);
        virtual ~D3DClass();

        static void Initialize(EngineCreateOptions createOptions);
		static D3DClass* Instance();
        virtual void Shutdown() = 0;

        virtual void ResizeBuffers(uint32_t newWidth, uint32_t newHeight) = 0;

        virtual void BeginScene(float, float, float, float) = 0;
        virtual void EndScene() = 0;

        // Begin and end render event must be called in pairs.
        // They can be nested bu there must be a matching end for every begin
        virtual void BeginRenderEvent(const std::wstring& eventLabel) = 0;
        virtual void EndRenderEvent() = 0;

        // Potentially rename FrameBuffer?
		virtual void SetRenderTarget(RenderTargetBundle::Ptr, Utils::Maths::Vector2 clipOffset = {}) const = 0;

        virtual void GetVideoCardInfo(char*, int&) = 0;

        virtual Texture::Ptr CopyTexture(Texture::Ptr textureToCopy) const = 0;

        virtual Texture::Ptr CopyBackBuffer() const = 0;

        virtual void UnbindAllRenderTargets() const = 0;
        virtual void UnbindShaderResourceView(uint32_t slot) const = 0;

	protected:
		virtual void Initialize_Internal(EngineCreateOptions createOptions) = 0;
    };
}
