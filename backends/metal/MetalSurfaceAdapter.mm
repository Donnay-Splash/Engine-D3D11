#import "MetalSurfaceAdapter.h"

@interface MetalSurfaceDelegate : NSObject <MTKViewDelegate>
{
@public
    Engine
::Surface::Callbacks callbacks;
}
@end

@implementation MetalSurfaceDelegate

- (void)drawInMTKView:(MTKView *)view
{
    (void)view;
    if (callbacks.Draw == nullptr) {
        return;
    }

    Engine
::Error error;
    if (!callbacks.Draw(callbacks.context, error) && callbacks.Failure != nullptr) {
        callbacks.Failure(callbacks.context, error);
    }
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size
{
    (void)view;
    if (callbacks.Resize == nullptr) {
        return;
    }

    const Engine
::Size drawableSize = {
        static_cast<std::uint32_t>(size.width > 0 ? size.width : 0),
        static_cast<std::uint32_t>(size.height > 0 ? size.height : 0),
    };
    Engine
::Error error;
    callbacks.Resize(callbacks.context, drawableSize, error);
}

@end

namespace Engine {
namespace Metal {
namespace {

class MetalSurface final : public Surface
{
public:
    explicit MetalSurface(MTKView *view) : view_(view), delegate_([[MetalSurfaceDelegate alloc] init])
    {
    }

    ~MetalSurface() override
    {
        Stop();
    }

    Size DrawableSize() const noexcept override
    {
        const CGSize size = view_.drawableSize;
        return {
            static_cast<std::uint32_t>(size.width > 0 ? size.width : 0),
            static_cast<std::uint32_t>(size.height > 0 ? size.height : 0),
        };
    }

    bool Start(const Callbacks &surfaceCallbacks, Error &error) override
    {
        (void)error;
        delegate_->callbacks = surfaceCallbacks;
        view_.delegate = delegate_;
        view_.enableSetNeedsDisplay = NO;
        view_.paused = NO;
        return YES;
    }

    void Stop() noexcept override
    {
        view_.paused = YES;
        if (view_.delegate == delegate_) {
            view_.delegate = nil;
        }
        delegate_->callbacks = {};
    }

private:
    __strong MTKView *view_;
    __strong MetalSurfaceDelegate *delegate_;
};

} // namespace

std::unique_ptr<Surface> CreateMetalSurface(MTKView *view)
{
    if (view == nil) {
        return std::unique_ptr<Surface>();
    }
    return std::unique_ptr<Surface>(new MetalSurface(view));
}

} // namespace Metal
} // namespace Engine
