//
//  MGLView.m
//  MGL
//
//  Created by Michael Larson on 1/18/26.
//

#import "MGLView.h"

#if _REMEMEBER_THS__
// With the shift key down, do slow-mo animation
if ([[NSApp currentEvent] modifierFlags] & NSShiftKeyMask)
    [[NSAnimationContext currentContext] setDuration:1.0];
#endif

@implementation MGLView

- (void) update
{
    [self updateLayer];
}

- (void) setup
{
    
}


- (void) initCADisplayLink
{
    CADisplayLink *displayLink = [self displayLinkWithTarget: self selector: @selector(update)];
    assert(link);
    
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

- (id) initWithCoder:(NSCoder *)coder;
{
    self = [super initWithCoder: coder];

    [self initCADisplayLink];
    
    // init with coder is from a .nib file so contents are flipped
    m_usesInitWithCoder = YES;

    [self setup];
    
    return self;
}

- (id) initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame: frameRect];

    [self initCADisplayLink];

    // init with coder is from a .nib file so contents are flipped
    m_usesInitWithCoder = NO;

    [self setup];
    
    return self;
}

- (void)updateLayer
{
    NSArray *sublayers;
    CALayer *layer, *sublayer;
    NSRect frame;

    frame = [self frame];
    
    layer = [self layer];
    
    sublayers = [layer sublayers];
    
    if (sublayers)
    {
        NSEnumerator *en;
        
        en = [sublayers objectEnumerator];
        
        while(sublayer = [en nextObject])
        {
            [sublayer setFrame: frame];
        }
    }

    [self needsDisplay];
}

- (BOOL) contentsAreFlipped
{
    return m_usesInitWithCoder;
}

- (void)drawRect:(NSRect)rect
{
    NSLog(@"%s %@", __FUNCTION__, NSStringFromRect(rect));
    
    [self needsDisplay];
}

- (void)updateTrackingAreas
{
    NSLog(@"%s", __FUNCTION__);
    
    [self needsDisplay];
}

- (BOOL)canBecomeKeyView
{
    NSLog(@"%s", __FUNCTION__);

    return YES;
}

- (BOOL)acceptsFirstResponder
{
    NSLog(@"%s", __FUNCTION__);

    return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
    NSLog(@"%s", __FUNCTION__);
    
    return YES;
}

- (BOOL)wantsUpdateLayer
{
    NSLog(@"%s", __FUNCTION__);

    return YES;
}

- (void)mouseDown:(NSEvent *)event
{
    NSLog(@"%s", __FUNCTION__);
}

- (void)mouseUp:(NSEvent *)event
{
    NSLog(@"%s", __FUNCTION__);
}

@end
