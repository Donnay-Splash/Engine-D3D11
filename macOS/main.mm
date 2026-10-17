#import <AppKit/AppKit.h>
#import <MetalKit/MetalKit.h>

#include "Engine.h"
#include "MetalSurfaceAdapter.h"

#include <cstdlib>
#include <memory>

@interface EngineAppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
{
@private
    std::unique_ptr<engine::Surface> surface_;
    std::unique_ptr<engine::Engine> engine_;
}

@property(nonatomic, strong) NSWindow *window;
@property(nonatomic) int exitStatus;

- (void)showMetalError:(NSString *)message;
- (void)shutdownEngine;

@end

@implementation EngineAppDelegate

- (void)shutdownEngine
{
    if (engine_ != nullptr) {
        engine_->shutdown();
    }
}

- (void)showMetalError:(NSString *)message
{
    [self shutdownEngine];
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = @"Metal rendering failed";
    alert.informativeText = message;
    [alert addButtonWithTitle:@"Quit"];
    self.exitStatus = EXIT_FAILURE;
    [alert runModal];
    [NSApp terminate:nil];
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    (void)notification;

    NSRect frame = NSMakeRect(0.0, 0.0, 800.0, 600.0);
    NSWindowStyleMask style = NSWindowStyleMaskTitled
        | NSWindowStyleMaskClosable
        | NSWindowStyleMaskMiniaturizable
        | NSWindowStyleMaskResizable;
    self.window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:style
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    if (self.window == nil) {
        [self showMetalError:@"Failed to create the Engine window."];
        return;
    }

    MTKView *view = [[MTKView alloc] initWithFrame:frame device:nil];
    view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    surface_ = engine::macos::createMetalSurface(view);
    engine_ = engine::createEngine();
    if (surface_ == nullptr || engine_ == nullptr) {
        [self showMetalError:@"Failed to create the engine or render surface."];
        return;
    }

    __weak EngineAppDelegate *weakSelf = self;
    engine::Error error;
    const bool initialized = engine_->initialize(
        *surface_,
        [weakSelf](const engine::Error &runtimeError) {
            NSString *message = [NSString stringWithUTF8String:runtimeError.message.c_str()];
            dispatch_async(dispatch_get_main_queue(), ^{
                [weakSelf showMetalError:message ?: @"The engine encountered a runtime failure."];
            });
        },
        error);
    if (!initialized) {
        NSString *message = [NSString stringWithUTF8String:error.message.c_str()];
        [self showMetalError:message ?: @"Failed to initialize the engine."];
        return;
    }

    self.window.title = @"Engine";
    self.window.delegate = self;
    self.window.contentView = view;
    [self.window center];
    [self.window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
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
