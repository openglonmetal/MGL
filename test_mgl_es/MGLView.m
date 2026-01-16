//
//  mgl_view.m
//  test_mgl_es
//
//  Created by Michael Larson on 1/13/26.
//

#import "MGLView.h"
#import "gl_es_test.h"

#include "GLES/gl.h"
#include "MGLContext.h"

NS_ASSUME_NONNULL_BEGIN


@implementation MGLView

- (id) initWithFrame:(NSRect)frameRect
{
    return [super initWithFrame: frameRect];
}

- (void)updateLayer
{
    NSRect frame;
        
    frame = [self frame];
    
    test_gl_es(frame.size.width, frame.size.height);
    
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

@end

NS_ASSUME_NONNULL_END
