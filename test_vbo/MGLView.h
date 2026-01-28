//
//  MGLTestView.h
//  MGL
//
//  Created by Michael Larson on 1/18/26.
//

#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif

#define GL_GLEXT_PROTOTYPES 1

#import <OpenGL_4_6/OpenGL_4_6.h>
#import <QuartzCore/QuartzCore.h>
#import <Foundation/Foundation.h>

@interface MGLView : NSView
{
    BOOL m_usesInitWithCoder;
}

-(void)setup;

@end


