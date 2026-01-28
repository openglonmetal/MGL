//
//  MGLTestView.m
//  MGL
//
//  Created by Michael Larson on 1/18/26.
//

#include <OpenGL_4_6/glcorearb.h>
#include <OpenGL_4_6/MGLContext.h>

#import "MGLTestView.h"
#import "gl_test.h"


@implementation MGLTestView

- (id) initWithFrame:(NSRect)frameRect
{
    return [super initWithFrame: frameRect];
}

- (void)updateLayer
{
    NSRect frame;
        
    frame = [self frame];

    MGLswapBuffers(MGLgetCurrentContext());
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)wantsUpdateLayer
{
    return YES;
}

- (void)mouseUp:(NSEvent *)event
{
    
}

@end
