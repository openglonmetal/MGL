//
//  AppDelegate.h
//  test_vbo
//
//  Created by Michael Larson on 1/24/26.
//

#import <Cocoa/Cocoa.h>
#import <OpenGL_4_6/OpenGL_4_6.h>

#import "VBOView.h"

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate, NSTabViewDelegate>
{
    IBOutlet NSTabView  *m_tabview;
    IBOutlet VBOView    *m_VBOview;
    
    IBOutlet NSButton   *m_render_button;
    IBOutlet NSPopUpButton  *m_primitive_draw_popup_button;
    
    IBOutlet NSButton   *m_3x_4x_mode_button;

    GLMContext  m_glm_ctx;
    MGLRenderer *m_renderer;
}

- (IBAction)renderButtonPressed:(id)sender;
- (IBAction)drawPrimitivePopupButtonPressed:(id)sender;
- (IBAction)mode_3x_4x_ButtonPressed:(id)sender;

@end

