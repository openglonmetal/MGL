//
//  MGL_Tests.m
//  MGL Tests
//
//  Created by Michael Larson on 1/3/25.
//

#import <XCTest/XCTest.h>

#include <mach/mach_vm.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>

#include <stdbool.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <sys/stat.h>

#define GL_GLEXT_PROTOTYPES 1
#include <GL/glcorearb.h>

extern "C" {
#include "MGLContext.h"
}
#include "MGLRenderer.h"

#import "MGL_test_utils.h"


@interface MGL_Tests : XCTestCase
{
    NSApplication *m_application;
    NSWindow *m_window;
    GLMContext m_glm_ctx;
    MGLRenderer *m_renderer;
}
- (GLuint) winWidth;
- (GLuint) winHeight;
@end

@implementation MGL_Tests

- (NSRect) windowFrame
{
    return [m_window contentLayoutRect];
}

- (GLuint) winWidth
{
    return [self windowFrame].size.width;
}

- (GLuint) winHeight
{
    return [self windowFrame].size.height;
}

- (const char *) getTestDirectory
{
    return "/tmp/MGL_Testing";
}

- (bool) doesTestDirectoryExist
{
    const char *path;
    
    path = [self getTestDirectory];
    
    struct stat st;
    if (stat(path, &st) == 0)
    {
        if (S_ISDIR(st.st_mode))
        {
            return true;
        }
    }
    
    return false;
}

- (char *) getTestResultPath: (const char *)testname
{
    const char *test_dir;
    size_t len;
    
    test_dir = [self getTestDirectory];
    
    len = strlen(test_dir) + strlen(testname);
    len += 20;
    
    char *path;
    path = (char *)malloc(len);
    
    snprintf(path, len, "%s/%s.tga", test_dir, testname);
    
    return path;
}

- (char *) getGoldenImagePath: (const char *)testname
{
    const char *test_dir;
    size_t len;
    
    test_dir = [self getTestDirectory];
    
    len = strlen(test_dir) + strlen(testname);
    len += 20;
    
    char *path;
    path = (char *)malloc(len);
    
    snprintf(path, len, "%s/Golden_%s.tga", test_dir, testname);
    
    return path;
}

- (bool) isGoldenImageAvailable: (const char *)path
{
    FILE *fp;
    
    fp = fopen(path, "r");
    
    if (fp == NULL)
        return false;
    
    fclose(fp);
    
    return true;
}

- (size_t) fileSizeInBytes: (const char *)filePath
{
    struct stat fileStat;
    int err;
    
    err = stat(filePath, &fileStat);
    
    if (err == 0)
    {
        return fileStat.st_size; // File size in bytes
    }

    perror("stat failed");
    
    return 0;
}

- (size_t) readFile: (const char *)path toBuf: (char *)buf ofLen: (size_t) len;
{
    FILE *fp;
    
    if (len == 0)
        return [self fileSizeInBytes: path];;

    if (buf == NULL)
        return 0;
    
    fp = fopen(path, "rb");
    if (fp == NULL)
        return 0;
    
    len = fread(buf, len, 1, fp);
    
    fclose(fp);
    
    // return num items read
    return len;
}

- (bool) compareFiles: (const char *)path golden: (const char *)golden_path
{
    size_t buf_len, golden_buf_len;
    char *buf, *golden_buf;
    bool result;
    
    result = false;

    do
    {
        buf_len = [self fileSizeInBytes: path];
        if (buf_len == 0)
            continue;
        
        golden_buf_len = [self fileSizeInBytes: golden_path];
        if (golden_buf_len == 0)
            continue;

        // compare buffer sizes
        if (buf_len != golden_buf_len)
            continue;

        // read images
        buf = (char *)malloc(buf_len);
        [self readFile: path toBuf: buf ofLen: buf_len];
        
        golden_buf = (char *)malloc(golden_buf_len);
        [self readFile: golden_path toBuf: golden_buf ofLen: golden_buf_len];

        // compare images
        if (memcmp(golden_buf, buf, buf_len) == 0)
        {
            result = true;
        }
        
        if (buf)
            free(buf);
        
        if (golden_buf)
            free(golden_buf);
        
    } while(0);
    
    return result;
}

- (bool) writeAndCompareResults: (const char *)testname size:(NSSize)size pixels:(void *)pixels
{
    bool result;
    char *path, *golden_path;

    // create test directory if it doesn't exist
    result = [self doesTestDirectoryExist];
    if (result == false)
    {
        mkdir([self getTestDirectory], 0700);
    }

    path = NULL; // get rid of warning
    do {
        golden_path = [self getGoldenImagePath: testname];
        
        if ([self isGoldenImageAvailable: golden_path] == false)
        {
            result = tga_write(golden_path, size.width, size.height, (uint8_t *)pixels, 4, 4);
            if (result == false)
                continue;
        }
        
        path = [self getTestResultPath: testname];
        
        result = tga_write(path, size.width, size.height, (uint8_t *)pixels, 4, 4);
        if (result == false)
            continue;

        result = [self compareFiles: path golden: golden_path];
        if (result == false)
            continue;
        
        result = true;
    } while(0);
        
    free(golden_path);
    free(path);

    return result;
}

- (bool) compareResults: (const char *)test_name
{
    size_t len;
    len = [self winWidth] * [self winHeight] * 4;
    
    uint8_t *pixels;
    pixels = (uint8_t *)malloc(len);

    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, [self winWidth], [self winHeight], GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glFlush();

    NSSize size;
    bool test_passed;

    test_passed = false;

    size = NSMakeSize([self winWidth], [self winHeight]);
    test_passed = [self writeAndCompareResults: test_name size: size pixels: pixels];
    
    free(pixels);
                      
    if(test_passed == false)
        XCTFail(@"Image comparison failed");
    
    return test_passed;
}

- (void)setUp
{
    dispatch_async(dispatch_get_main_queue(), ^{
        // Initialize NSApplication
        if (NSApp == NULL)
        {
            self->m_application = [NSApplication sharedApplication];
        }

        // Create and configure the NSWindow
        self->m_window = [[NSWindow alloc] initWithContentRect:NSMakeRect(32, 32, 512, 512)
                                                     styleMask:(NSWindowStyleMaskTitled |
                                                                NSWindowStyleMaskClosable)
                                                       backing:NSBackingStoreBuffered
                                                         defer:NO];
        
        // Run the main loop temporarily to simulate user interaction
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow: 1]];
        
        // Show the window
        [self->m_window makeKeyAndOrderFront:nil];
        [self->m_window makeMainWindow];
                
        [self->m_window setBackgroundColor: [NSColor blackColor]];
        
        [self->m_window display];
        [self->m_window makeKeyAndOrderFront:nil];
        
        [NSApp activateIgnoringOtherApps:YES];
        
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow: 1]];

        self->m_glm_ctx = createGLMContext(GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, GL_DEPTH_COMPONENT, GL_FLOAT, 0, 0);
        self->m_renderer = [[MGLRenderer alloc] initMGLRendererFromContext: self->m_glm_ctx andBindToWindow: self->m_window];
        
        if (!self->m_renderer)
        {
            exit(EXIT_FAILURE);
        }
        
        MGLsetCurrentContext(self->m_glm_ctx);
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow: 1]];
    });
}

- (void)tearDown
{
    dispatch_async(dispatch_get_main_queue(), ^{
        self->m_window = nil;
    });

    self->m_renderer = NULL;
    self->m_glm_ctx = NULL;
    
    [super tearDown];
}

- (void)testGLClear
{
    XCTestExpectation *expectation = [self expectationWithDescription:@"Task on main thread completed"];
    
    // Ensure everything runs on the main thread
    dispatch_async(dispatch_get_main_queue(), ^{
        int a = 0;
        int e = 1;
        
        int count;
        count = 200;
        while(count--)
        {
            glClearColor(0.2, 0.2, (float)a/100.0, 0.0);
            glClear(GL_COLOR_BUFFER_BIT);
            
            a += e;
            if(a>=100){e=-1;}
            if(a==0){e=1;}

            MGLswapBuffers(NULL);
        }

        bool result;

        result = [self compareResults: "testGLClear"];

        [expectation fulfill];
    });
    
    // Wait for the expectation
    [self waitForExpectationsWithTimeout:60.0 handler:^(NSError * _Nullable error) {
        XCTAssertNil(error, @"The task did not complete in time");
    }];
}

- (void)testGLDrawArrays
{
    XCTestExpectation *expectation = [self expectationWithDescription:@"Task on main thread completed"];
    
    // Ensure everything runs on the main thread
    dispatch_async(dispatch_get_main_queue(), ^{
        GLuint vbo = 0, vao = 0;

        const char* vertex_shader =
        GLSL(460,
             layout(location = 0) in vec3 position;
             void main() {
                gl_Position = vec4(position, 1.0);
            }
        );

        const char* fragment_shader =
        GLSL(460,
             layout(location = 0) out vec4 frag_colour;
             void main() {
                frag_colour = vec4(0.5, 0.0, 0.5, 1.0);
            }
        );

        float points[] = {
           0.0f,  0.5f,  0.0f,
           0.5f, -0.5f,  0.0f,
          -0.5f, -0.5f,  0.0f
        };

        vbo = bindDataToVBO(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
        vao = bindVAO();

        bindAttribute(0, GL_ARRAY_BUFFER, vbo, 3, GL_FLOAT, false, 0, NULL);

        GLuint shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);
        glUseProgram(shader_program);

        glViewport(0, 0, [self winWidth], [self winHeight]);

        glClearColor(0.2, 0.2, 0.2, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glDrawArrays(GL_TRIANGLES, 0, 3);
            
        MGLswapBuffers(NULL);

        bool result;

        result = [self compareResults: "testGLDrawArrays"];

        [expectation fulfill];
    });
    
    // Wait for the expectation
    [self waitForExpectationsWithTimeout:120 handler:^(NSError * _Nullable error) {
        XCTAssertNil(error, @"The task did not complete in time");
    }];
}

- (void)testGLDrawArraysUniform1i
{
    XCTestExpectation *expectation = [self expectationWithDescription:@"Task on main thread completed"];
    
    // Ensure everything runs on the main thread
    dispatch_async(dispatch_get_main_queue(), ^{
        GLuint vbo = 0, vao = 0;

        const char* vertex_shader =
        GLSL(460,
             layout(location = 0) in vec3 position;
             void main() {
                gl_Position = vec4(position.x, position.y, position.z, 1.0);
            }
        );
        const char* fragment_shader =
        GLSL(460,
             layout(location = 0) out vec4 frag_colour;
             layout(location = 0) uniform int mp;
             void main() {
                frag_colour = vec4(0.0, 0.0, float(mp)/100.0, 1.0);
            }
        );

        float points[] = {
           0.0f,  0.5f,  0.0f,
           0.5f, -0.5f,  0.0f,
          -0.5f, -0.5f,  0.0f
        };

        vbo = bindDataToVBO(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
        vao = bindVAO();

        bindAttribute(0, GL_ARRAY_BUFFER, vbo, 3, GL_FLOAT, false, 0, NULL);

        GLuint shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);
        glUseProgram(shader_program);

        glViewport(0, 0, [self winWidth], [self winHeight]);

        GLint mp_loc = glGetUniformLocation(shader_program, "mp");
        std::cout << mp_loc << std::endl;
        
        int a = 0;
        int e = 1;
        
        glClearColor(0.2, 0.2, 0.2, 0.0);
        
        glClear(GL_COLOR_BUFFER_BIT);
        MGLswapBuffers(NULL);

        glUseProgram(shader_program);
        
        int count;
        count = 100;
        while(count--)
        {
            glBindVertexArray(vao);
            glUniform1i(mp_loc, a);
            
            glClearColor(0.2, 0.2, (float)a/100.0, 0.0);
            glClear(GL_COLOR_BUFFER_BIT);
            
            glDrawArrays(GL_TRIANGLES, 0, 3);
            
            MGLswapBuffers(NULL);
            
            a += e;
            if(a>=100){e=-1;}
            if(a==0){e=1;}
        }
        
        bool result;

        result = [self compareResults: "testGLDrawArraysUniform1i"];

        [expectation fulfill];
    });
    
    // Wait for the expectation
    [self waitForExpectationsWithTimeout:120.0 handler:^(NSError * _Nullable error) {
        XCTAssertNil(error, @"The task did not complete in time");
    }];
}

- (void) testReadPixels
{
    XCTestExpectation *expectation = [self expectationWithDescription:@"Task on main thread completed"];
    
    // Ensure everything runs on the main thread
    dispatch_async(dispatch_get_main_queue(), ^{
        const char* vertex_shader =
        GLSL(450 core,
             layout(location = 0) in vec3 position;
             layout(location = 1) in vec3 in_color;
             layout(location = 2) in vec2 in_texcords;
             
             layout(location = 0) out vec4 out_color;
             layout(location = 1) out vec2 out_texcoords;
             
             void main() {
                gl_Position = vec4(position, 1.0);
                out_color = vec4(in_color, 1.0);
                out_texcoords = in_texcords;
            }
        );

        const char* fragment_shader =
        GLSL(450 core,
             layout(location = 0) in vec4 in_color;
             layout(location = 1) in vec2 in_texcords;
             
             layout(location = 0) out vec4 frag_colour;
             
             uniform sampler2D image;
             
             void main() {
                vec4 tex_color = texture(image, in_texcords);
            
                frag_colour = in_color * tex_color;
            }
        );
        
        GLuint vbo = 0, col_vbo = 0, tex_vbo = 0;

        float points[] = {
            0.0f,  0.5f,  0.0f,
            0.5f, -0.5f,  0.0f,
            -0.5f, -0.5f,  0.0f
        };

        float color[] = {
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
        };

        float texcoords[] = {
            0.5f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
        };

        vbo = bindDataToVBO(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);
        col_vbo = bindDataToVBO(GL_ARRAY_BUFFER, 9 * sizeof(float), color, GL_STATIC_DRAW);
        tex_vbo = bindDataToVBO(GL_ARRAY_BUFFER, 6 * sizeof(float), texcoords, GL_STATIC_DRAW);

        GLuint vao = 0;
        glCreateVertexArrays(1, &vao);
        glBindVertexArray(vao);

        bindAttribute(0, GL_ARRAY_BUFFER, vbo, 3, GL_FLOAT, false, 0, NULL);
        bindAttribute(1, GL_ARRAY_BUFFER, col_vbo, 3, GL_FLOAT, false, 0, NULL);
        bindAttribute(2, GL_ARRAY_BUFFER, tex_vbo, 2, GL_FLOAT, false, 0, NULL);

        GLuint shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);
        glUseProgram(shader_program);

        GLuint tex;
        tex = createTexture(GL_TEXTURE_2D, 256, 256, 0, genTexturePixels(GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0x10, 256, 256));
        glBindTexture(GL_TEXTURE_2D, tex);

        glViewport(0, 0, [self winWidth], [self winHeight]);

        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        MGLswapBuffers();
        
        bool result;
        result = [self compareResults: "testReadPixels"];

        [expectation fulfill];
    });
    
    // Wait for the expectation
    [self waitForExpectationsWithTimeout:120.0 handler:^(NSError * _Nullable error) {
        XCTAssertNil(error, @"The task did not complete in time");
    }];
}

#if 0
- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}
#endif

@end
