#import <AppKit/AppKit.h>
#import <MetalKit/MetalKit.h>

#include "MacEngine.h"
#include "MetalSurfaceAdapter.h"

#include <cstdlib>
#include <memory>

@interface EngineAppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@property(nonatomic, strong) NSWindow *window;
@property(nonatomic) int exitStatus;

- (instancetype)init;
- (void)showEngineError:(NSString *)message;
- (void)shutdownEngine;
- (void)drawFrame:(NSTimer *)timer;
- (void)updateDrawableSize;

@end

@implementation EngineAppDelegate {
    std::unique_ptr<Engine::Surface> m_Surface;
    std::unique_ptr<Engine::Engine> m_Engine;
    NSTimer *m_DrawTimer;
    MTKView *m_View;
}

- (instancetype)init
{
    self = [super init];
    if (self == nil) {
        return nil;
    }

    _exitStatus = EXIT_SUCCESS;
    return self;
}

- (void)shutdownEngine
{
    [m_DrawTimer invalidate];
    m_DrawTimer = nil;
    if (m_Engine != nullptr) {
        m_Engine->Shutdown();
    }
}

- (void)drawFrame:(NSTimer *)timer
{
    (void)timer;
    if (m_Surface == nullptr) {
        return;
    }

    Engine::Error error = {};
    Engine::Metal::DispatchMetalDraw(*m_Surface, error);
}

- (void)updateDrawableSize
{
    if (m_View == nil) {
        return;
    }

    const CGSize size = [m_View convertSizeToBacking:m_View.bounds.size];
    CAMetalLayer *layer = (CAMetalLayer *)m_View.layer;
    layer.drawableSize = size;
    if (m_Surface != nullptr && m_Engine != nullptr) {
        Engine::Error error = {};
        Engine::Metal::DispatchMetalResize(
            *m_Surface,
            {
                size.width > 0.0 ? static_cast<std::uint32_t>(size.width) : 0,
                size.height > 0.0 ? static_cast<std::uint32_t>(size.height) : 0,
            },
            error);
    }
}

- (void)showEngineError:(NSString *)message
{
    [self shutdownEngine];

    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = @"Engine initialization failed";
    alert.informativeText = message ?: @"An unknown rendering error occurred.";
    [alert addButtonWithTitle:@"Quit"];

    self.exitStatus = EXIT_FAILURE;
    [alert runModal];
    [NSApp terminate:nil];
}

- (BOOL)configureWindowWithFrame:(NSRect)frame
{
    NSWindowStyleMask style = NSWindowStyleMaskTitled
        | NSWindowStyleMaskClosable
        | NSWindowStyleMaskMiniaturizable
        | NSWindowStyleMaskResizable;

    self.window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:style
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    if (self.window == nil) {
        [self showEngineError:@"Failed to create the Engine window."];
        return NO;
    }

    return YES;
}

- (BOOL)initializeEngineWithView:(MTKView *)view
{
    m_View = view;
    [self updateDrawableSize];
    CA::MetalLayer *layer = (__bridge CA::MetalLayer *)view.layer;
    m_Surface = Engine::Metal::CreateMetalSurface(layer);
    m_Engine = Engine::Metal::CreateEngine();
    if (m_Surface == nullptr || m_Engine == nullptr) {
        [self showEngineError:@"Failed to create the engine or render surface."];
        return NO;
    }

    __weak EngineAppDelegate *weakSelf = self;
    Engine::Error error = {};
    const bool initialized = m_Engine->Initialize(
        *m_Surface,
        [weakSelf](const Engine::Error &runtimeError) {
            EngineAppDelegate *strongSelf = weakSelf;
            if (strongSelf == nil) {
                return;
            }

            NSString *message = [NSString stringWithUTF8String:runtimeError.message.c_str()];
            dispatch_async(dispatch_get_main_queue(), ^{
                [strongSelf showEngineError:message ?: @"The engine encountered a runtime failure."];
            });
        },
        error);
    if (!initialized) {
        NSString *message = [NSString stringWithUTF8String:error.message.c_str()];
        [self showEngineError:message ?: @"Failed to initialize the engine."];
        return NO;
    }

    return YES;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    (void)notification;

    const NSRect frame = NSMakeRect(0.0, 0.0, 800.0, 600.0);
    if (![self configureWindowWithFrame:frame]) {
        return;
    }

    MTKView *view = [[MTKView alloc] initWithFrame:frame device:nil];
    view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

    self.window.title = @"Engine";
    self.window.delegate = self;
    self.window.contentView = view;
    [self.window center];
    [self.window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
    [view layoutSubtreeIfNeeded];
    [self updateDrawableSize];

    if (![self initializeEngineWithView:view]) {
        return;
    }
    m_DrawTimer = [NSTimer scheduledTimerWithTimeInterval:(1.0 / 60.0)
                                                   target:self
                                                 selector:@selector(drawFrame:)
                                                 userInfo:nil
                                                  repeats:YES];
}

- (void)windowDidResize:(NSNotification *)notification
{
    (void)notification;
    [self updateDrawableSize];
}

- (void)windowDidChangeBackingProperties:(NSNotification *)notification
{
    (void)notification;
    [self updateDrawableSize];
}

- (void)windowWillClose:(NSNotification *)notification
{
    (void)notification;
    [self shutdownEngine];
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    (void)notification;
    [self shutdownEngine];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    (void)sender;
    return YES;
}

@end

int main()
{
    @autoreleasepool {
        NSApplication *application = [NSApplication sharedApplication];
        [application setActivationPolicy:NSApplicationActivationPolicyRegular];

        EngineAppDelegate *delegate = [[EngineAppDelegate alloc] init];
        application.delegate = delegate;
        [application run];
        return delegate.exitStatus;
    }
}
