#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include <cstdlib>

static NSError *rendererError(NSInteger code, NSString *description)
{
    return [NSError errorWithDomain:@"EngineMetalRenderer"
                               code:code
                           userInfo:@{NSLocalizedDescriptionKey : description}];
}

@interface EngineRenderer : NSObject <MTKViewDelegate>

@property(nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property(nonatomic, strong) id<MTLRenderPipelineState> pipelineState;
@property(nonatomic, copy) void (^errorHandler)(NSString *message);
@property(nonatomic) BOOL failed;

- (instancetype)initWithView:(MTKView *)view error:(NSError **)error;

@end

@implementation EngineRenderer

- (instancetype)initWithView:(MTKView *)view error:(NSError **)error
{
    self = [super init];
    if (self == nil) {
        return nil;
    }

    self.commandQueue = [view.device newCommandQueue];
    if (self.commandQueue == nil) {
        if (error != NULL) {
            *error = rendererError(1, @"Failed to create a Metal command queue.");
        }
        return nil;
    }

    NSURL *libraryURL = [NSBundle.mainBundle URLForResource:@"default" withExtension:@"metallib"];
    if (libraryURL == nil) {
        if (error != NULL) {
            *error = rendererError(2, @"The bundled Metal shader library is missing.");
        }
        return nil;
    }

    id<MTLLibrary> library = [view.device newLibraryWithURL:libraryURL error:error];
    if (library == nil) {
        return nil;
    }

    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"triangle_vertex"];
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"triangle_fragment"];
    if (vertexFunction == nil || fragmentFunction == nil) {
        if (error != NULL) {
            *error = rendererError(3, @"The Metal shader library is missing a triangle shader function.");
        }
        return nil;
    }

    MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;
    pipelineDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    self.pipelineState = [view.device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:error];
    return self.pipelineState == nil ? nil : self;
}

- (BOOL)markFailed
{
    @synchronized (self) {
        if (self.failed) {
            return NO;
        }
        self.failed = YES;
        return YES;
    }
}

- (void)reportError:(NSString *)message
{
    if ([self markFailed] && self.errorHandler != nil) {
        self.errorHandler(message);
    }
}

- (void)drawInMTKView:(MTKView *)view
{
    @synchronized (self) {
        if (self.failed) {
            return;
        }
    }

    id<CAMetalDrawable> drawable = view.currentDrawable;
    MTLRenderPassDescriptor *renderPass = view.currentRenderPassDescriptor;
    if (drawable == nil || renderPass == nil) {
        return;
    }

    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];
    if (commandBuffer == nil) {
        [self reportError:@"Failed to create a Metal command buffer."];
        return;
    }

    id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPass];
    if (encoder == nil) {
        [self reportError:@"Failed to create a Metal render command encoder."];
        return;
    }

    [encoder setRenderPipelineState:self.pipelineState];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    [encoder endEncoding];
    [commandBuffer presentDrawable:drawable];

    __weak EngineRenderer *weakSelf = self;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> completedBuffer) {
        if (completedBuffer.status == MTLCommandBufferStatusError) {
            [weakSelf reportError:completedBuffer.error.localizedDescription];
        }
    }];
    [commandBuffer commit];
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size
{
    (void)view;
    (void)size;
}

@end

@interface EngineAppDelegate : NSObject <NSApplicationDelegate>

@property(nonatomic, strong) NSWindow *window;
@property(nonatomic, strong) EngineRenderer *renderer;
@property(nonatomic) int exitStatus;

- (void)showMetalError:(NSString *)message;

@end

@implementation EngineAppDelegate

- (void)showMetalError:(NSString *)message
{
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

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (device == nil) {
        [self showMetalError:@"Engine requires a Mac with a usable Metal device."];
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
    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    view.clearColor = MTLClearColorMake(0.08, 0.09, 0.12, 1.0);
    view.paused = NO;
    view.enableSetNeedsDisplay = NO;

    NSError *error = nil;
    self.renderer = [[EngineRenderer alloc] initWithView:view error:&error];
    if (self.renderer == nil) {
        [self showMetalError:error.localizedDescription ?: @"Failed to initialize the Metal renderer."];
        return;
    }

    __weak EngineAppDelegate *weakSelf = self;
    self.renderer.errorHandler = ^(NSString *message) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [weakSelf showMetalError:message];
        });
    };
    view.delegate = self.renderer;

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
