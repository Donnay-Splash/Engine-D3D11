#pragma once

#include "Engine.h"

namespace Engine {

// A surface with no window. The owner drives frames by calling RenderFrame, which runs the
// engine's normal draw callback. Use with SwapchainMode::Headless.
class HeadlessSurface final : public Surface
{
public:
    explicit HeadlessSurface(Size size);

    Size DrawableSize() const noexcept override;
    bool Start(const Callbacks &callbacks, Error &error) override;
    void Stop() noexcept override;

    // Draws one frame. Returns false if the surface is not started or the draw failed.
    bool RenderFrame(Error &error);

private:
    Size m_Size;
    Callbacks m_Callbacks;
    bool m_Started;
};

} // namespace Engine
