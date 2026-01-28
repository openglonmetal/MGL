//
//  AppDelegate.m
//  test_vbo
//
//  Created by Michael Larson on 1/24/26.
//

#import "AppDelegate.h"
#import <QuartzCore/CAMetalLayer.h>

@interface AppDelegate ()

@property (strong) IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (void)initUIElements
{
    // setup draw primitives popup button
    NSArray *primitive_names = @[@"DrawArrays", @"DrawElements", @"DrawRangeElements",
                                 @"DrawArraysInstanced", @"DrawElementsInstanced",
                                 @"Random"];
    GLuint primitve_ret[] = {kDrawArrays, kDrawElements, kDrawRangeElements,
                            kDrawArraysInstanced, kDrawElementsInstanced,
                            kRandomPrimitiveType};
        
    [m_primitive_draw_popup_button removeAllItems];
    
    for(NSInteger i=0; i < primitive_names.count; i++)
    {
        NSString *name = primitive_names[i];
        GLuint retValue = primitve_ret[i];
        
        [m_primitive_draw_popup_button addItemWithTitle: name];
        [[m_primitive_draw_popup_button itemAtIndex: i] setTag: retValue];
    }
    
    [m_3x_4x_mode_button setState: false];
}


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    assert(_window);    
    assert(m_VBOview);
    assert(m_tabview);
    assert(m_render_button);
    assert(m_3x_4x_mode_button);
    
    m_glm_ctx = createGLMContext(GL_RGBA, GL_UNSIGNED_BYTE, GL_DEPTH_COMPONENT, GL_FLOAT, 0, 0);
    assert (m_glm_ctx);
    
    m_renderer = [[MGLRenderer alloc] init];
    assert (m_renderer);
    
    [m_renderer createMGLRendererAndBindToContext: m_glm_ctx view: m_VBOview];
    
    MGLsetCurrentContext(m_glm_ctx);
    
    [m_VBOview setNeedsDisplay: YES];
    
    [self.window makeKeyAndOrderFront:nil];
    
    [self initUIElements];
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *) sender;
{
    return YES;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    // Insert code here to tear down your application
}

- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app
{
    return YES;
}

- (void)windowDidResize:(NSNotification *)notification
{
    NSRect windowFrame = [_window frame];
    
    NSLog(@"%s window: %@", __FUNCTION__, NSStringFromRect(windowFrame));
    
}

// button interfaces
- (IBAction)renderButtonPressed:(id)sender
{
    NSButton *button = (NSButton *)sender;

    // Get the title of the button
    NSString *buttonTitle = [button title];

    NSLog(@"%s %@ pressed!", __FUNCTION__, buttonTitle);
}


- (IBAction)drawPrimitivePopupButtonPressed:(id)sender
{
    NSPopUpButton *popup;
    GLuint type;
    
    popup = sender;
    type = (GLint)[[popup selectedItem] tag];
    
    assert(type < kMaxPrimitiveDrawType);
    
    [m_VBOview setPrimitveDrawType: type];
}

- (IBAction)mode_3x_4x_ButtonPressed:(id)sender
{
    NSButton *button;
    
    button = sender;
    
    if([button state])
    {
        [m_VBOview set3x_4x_mode: true];
    }
    else
    {
        [m_VBOview set3x_4x_mode: false];
    }
}

// tabview delegate methods
- (BOOL)tabView:(NSTabView *)tabView shouldSelectTabViewItem:(nullable NSTabViewItem *)tabViewItem
{
    return true;
}

- (void)tabView:(NSTabView *)tabView willSelectTabViewItem:(nullable NSTabViewItem *)tabViewItem
{
    
}

- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(nullable NSTabViewItem *)tabViewItem
{
    
}

- (void)tabViewDidChangeNumberOfTabViewItems:(NSTabView *)tabView
{
    
}

@end
