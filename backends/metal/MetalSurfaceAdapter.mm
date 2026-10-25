#import "MetalSurfaceAdapter.h"

#import "MetalFrame.h"

@interface MetalSurfaceDelegate : NSObject <MTKViewDelegate>
{
@public
    engine::Surface::Callbacks callbacks;
}
@end

@implementation MetalSurfaceDelegate

- (void)drawInMTKView:(MTKView *)view
{
    if (view.device == nil) {
        if (callbacks.failure != nullptr) {
            engine::Error error;
            error.message = "The Metal render surface lost its device.";
            callbacks.failure(callbacks.context, error);
        }
        return;
    }

    id<CAMetalDrawable> drawable = view.currentDrawable;
    MTLRenderPassDescriptor *renderPass = view.currentRenderPassDescriptor;
    if (drawable == nil || renderPass == nil || callbacks.draw == nullptr) {
        return;
    }

    MetalFrame frame = {drawable, renderPass};
    engine::Error error;
    callbacks.draw(callbacks.context, &frame, error);
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size
{
    (void)view;
    if (callbacks.resize == nullptr) {
        return;
    }

    const engine::Size drawableSize = {
        static_cast<std::uint32_t>(size.width > 0 ? size.width : 0),
        static_cast<std::uint32_t>(size.height > 0 ? size.height : 0),
    };
    engine::Error error;
    callbacks.resize(callbacks.context, drawableSize, error);
}

@end

namespace engine {
namespace macos {
namespace {

class MetalSurface final : public Surface
{
public:
    explicit MetalSurface(MTKView *view) : view_(view), delegate_([[MetalSurfaceDelegate alloc] init])
    {
    }

    ~MetalSurface() override
    {
        stop();
    }

    void *nativeHandle() noexcept override
    {
        return (__bridge void *)view_;
    }

    Size drawableSize() const noexcept override
    {
        const CGSize size = view_.drawableSize;
        return {
            static_cast<std::uint32_t>(size.width > 0 ? size.width : 0),
            static_cast<std::uint32_t>(size.height > 0 ? size.height : 0),
        };
    }

    bool start(const Callbacks &surfaceCallbacks, Error &error) override
    {
        (void)error;
        delegate_->callbacks = surfaceCallbacks;
        view_.delegate = delegate_;
        view_.enableSetNeedsDisplay = NO;
        view_.paused = NO;
        return YES;
    }

    void stop() noexcept override
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

std::unique_ptr<Surface> createMetalSurface(MTKView *view)
{
    if (view == nil) {
        return std::unique_ptr<Surface>();
    }
    return std::unique_ptr<Surface>(new MetalSurface(view));
}

} // namespace macos
} // namespace engine
