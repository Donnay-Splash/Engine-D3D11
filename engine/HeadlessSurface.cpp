#include "HeadlessSurface.h"

namespace Engine {

HeadlessSurface::HeadlessSurface(Size size)
    : m_Size(size), m_Callbacks{}, m_Started(false)
{
}

Size HeadlessSurface::DrawableSize() const noexcept
{
    return m_Size;
}

bool HeadlessSurface::Start(const Callbacks &callbacks, Error &)
{
    m_Callbacks = callbacks;
    m_Started = true;
    return true;
}

void HeadlessSurface::Stop() noexcept
{
    m_Started = false;
    m_Callbacks = {};
}

bool HeadlessSurface::RenderFrame(Error &error)
{
    if (!m_Started || m_Callbacks.Draw == nullptr) {
        error.message = "The headless surface is not started.";
        return false;
    }
    return m_Callbacks.Draw(m_Callbacks.context, error);
}

} // namespace Engine
