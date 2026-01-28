//
//  gl_es_test.h
//  test_mgl_es
//
//  Created by Michael Larson on 1/14/26.
//

#ifndef gl_es_test_h
#define gl_es_test_h

#include <stdio.h>

#define GLSL(version, shader) "#version " #version "\n" #shader

void test_gl_es(unsigned window_width, unsigned window_height);

#endif /* gl_es_test_h */
