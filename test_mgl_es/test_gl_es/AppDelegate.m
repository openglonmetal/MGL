//
//  AppDelegate.m
//  test_mgl_es
//
//  Created by Michael Larson on 1/13/26.
//

#import "AppDelegate.h"

#include "GLES/gl.h"
#include "GLES/glext.h"
#include "GLES3/gl3.h"
#include "GLES3/gl31.h"
#include "GLES3/gl32.h"

@interface AppDelegate ()

@property (strong) IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    assert(_window);
    
    NSRect frame;
    MGLView *view;
    
    frame = [[_window contentView] frame];
    
    view = [[MGLView alloc] initWithFrame: frame];
    [view setWantsLayer:YES];
    assert(view);
    
    [_window setContentView: view];
    [_window makeFirstResponder: view];
    [_window setDelegate: self];
    [_window setTitle: @"Test MGL ES"];
    [_window setAcceptsMouseMovedEvents: YES];
    [_window setRestorable: NO];
    
    GLMContext glm_ctx = createGLMContext(GL_RGBA, GL_UNSIGNED_BYTE, GL_DEPTH_COMPONENT, GL_FLOAT, 0, 0);
    assert (glm_ctx);

    MGLRenderer *renderer = [[MGLRenderer alloc] init];
    assert (renderer);

    [renderer createMGLRendererAndBindToContext: glm_ctx view: view];
    
    MGLsetCurrentContext(glm_ctx);
        
    [view setNeedsDisplay: YES];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}

@end
