#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include <cstdlib>

@interface EngineAppDelegate : NSObject <NSApplicationDelegate>

@property(nonatomic, strong) NSWindow *window;
@property(nonatomic) int exitStatus;

@end

@implementation EngineAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    (void)notification;

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (device == nil) {
        NSAlert *alert = [[NSAlert alloc] init];
        alert.messageText = @"Metal is unavailable";
        alert.informativeText = @"Engine requires a Mac with a usable Metal device.";
        [alert addButtonWithTitle:@"Quit"];
        [alert runModal];
        self.exitStatus = EXIT_FAILURE;
        [NSApp terminate:nil];
        return;
    }

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
        NSLog(@"Failed to create the Engine window.");
        self.exitStatus = EXIT_FAILURE;
        [NSApp terminate:nil];
        return;
    }

    MTKView *view = [[MTKView alloc] initWithFrame:frame device:device];
    view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    view.paused = YES;
    view.enableSetNeedsDisplay = NO;
    self.window.title = @"Engine";
    self.window.contentView = view;
    [self.window center];
    [self.window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
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
