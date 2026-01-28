//
//  TestVBOView.m
//  test_vbo
//
//  Created by Michael Larson on 1/25/26.
//

#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505

#import "VBOView.h"

#define GLSL(version, shader) "#version " #version "\n" #shader

#define CHECK_GL_ERR()  _checkGLError(__FUNCTION__, __LINE__)

NS_ASSUME_NONNULL_BEGIN

@implementation VBOView

GLuint _checkGLError(const char *func, const GLuint line)
{
    GLuint err;
    NSString *string;
    
    err = glGetError();
    
    if (err == GL_NO_ERROR)
        return err;
    
    switch(err) {
        case GL_INVALID_ENUM:
            string = @"GL_INVALID_ENUM";
            break;

        case GL_INVALID_VALUE:
            string = @"GL_INVALID_ENUM";
            break;

        case GL_INVALID_OPERATION:
            string = @"GL_INVALID_ENUM";
            break;

        case GL_OUT_OF_MEMORY:
            string = @"GL_INVALID_ENUM";
            break;

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            string = @"GL_INVALID_ENUM";
            break;
    }
    
    NSLog(@"GL ERROR %@ function: %s line: %d", string, func, line);
    
    return err;
}

- (nullable instancetype) initWithCoder:(NSCoder *)coder;
{
    NSLog(@"%s", __FUNCTION__);

    self = [super initWithCoder: coder];

    m_first_pass = 1;
    
    m_primiteDrawType = kDrawArrays;

    m_4x_mode = false;
    
    return self;
}

GLuint bindDataToVBO(GLenum target, size_t size, void *ptr, GLenum usage)
{
    GLuint vbo = 0;

    glGenBuffers(1, &vbo);
    glBindBuffer(target, vbo);
    glBufferData(target, size, ptr, usage);
    glBindBuffer(target, 0);

    return vbo;
}

GLuint createVBO(GLenum target, size_t size, GLenum usage)
{
    GLuint vbo = 0;

    glGenBuffers(1, &vbo);
    glBindBuffer(target, vbo);
    glBufferData(target, size, NULL, usage);
    glBindBuffer(target, 0);

    return vbo;
}

GLuint createVAO(GLuint vao)
{
    GLuint new_vao;

    glCreateVertexArrays(1, &new_vao);

    return new_vao;
}

GLuint createAndBindVAO(void)
{
    GLuint new_vao;

    glCreateVertexArrays(1, &new_vao);
    glBindVertexArray(new_vao);

    return new_vao;
}

void bindAttribute(GLuint index, GLuint target, GLuint vbo, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)
{
    glEnableVertexAttribArray(index);
    glBindBuffer(target, vbo);
    glVertexAttribPointer(index, size, type, GL_FALSE, stride, pointer);
}

void bindNullAttribute(GLuint index, GLuint target, GLuint vbo, GLint size, GLenum type, GLboolean normalized, GLsizei stride)
{
    glEnableVertexAttribArray(index);
    glBindBuffer(target, vbo);
    glVertexAttribPointer(index, size, type, GL_FALSE, stride, NULL);
}

GLuint compileGLSLProgram(GLenum shader_count, ...)
{
    va_list argp;
    va_start(argp, shader_count);
    GLuint type;
    const char *src;
    GLuint shader;

    GLuint shader_program = glCreateProgram();

    for(int i=0; i<shader_count; i++)
    {
        type = va_arg(argp, GLuint);
        src = va_arg(argp, const char *);
        assert(src);

        shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, NULL);
        glCompileShader(shader);
        glAttachShader(shader_program, shader);
    }
    
    glLinkProgram(shader_program);

    va_end(argp);

    return shader_program;
}

GLuint createTexture(GLenum target, GLsizei width, GLsizei height, GLsizei depth, const void *pixels, GLint level, GLint internalformat, GLenum format, GLenum type)
{
    GLuint tex;

    glGenTextures(1, &tex);
    glBindTexture(target, tex);
    switch(target)
    {
        case GL_TEXTURE_1D:
            glTexImage1D(target, level, internalformat, width, 0, format, type, pixels);
            break;

        case GL_TEXTURE_2D:
            glTexImage2D(target, level, internalformat, width, height, 0, format, type, pixels);
            break;

        case GL_TEXTURE_3D:
            glTexImage3D(target, level, internalformat, width, height, depth, 0, format, type, pixels);
            break;

        case GL_TEXTURE_CUBE_MAP:
            glTexImage2D(target, level, internalformat, width, height, 0, format, type, pixels);
            break;

    }
    glBindTexture(target, 0);

    return tex;
}

- (void) setup
{
    m_a = 50;
    m_e = 1;
}

- (void)setPrimitveDrawType:(GLuint)type
{
    assert(type < kMaxPrimitiveDrawType);
    
    m_primiteDrawType = type;
}

-(void)set3x_4x_mode: (GLboolean) mode
{
    m_4x_mode = mode;
}

- (bool)checkVBOErrors
{
    //GLuint vao;
    GLuint vbo;
    void *data;
    size_t size;
    GLuint err;
    
    size = 4 * 4096; // so vm_allocate is called
    data = (void *)malloc(size);
    
    // clear err
    glGetError();

    bool pass = false;
    do {
        // check genBufers
        glGenBuffers(-1, &vbo);
        err = glGetError();
        if (err != GL_INVALID_VALUE)
            continue;
        
        // does nothing
        glGenBuffers(1, NULL);
        err = glGetError();
        if (err != GL_NO_ERROR)
            continue;

        // create buffer and test
        glGenBuffers(1, &vbo);
        if (glIsBuffer(vbo) == false)
            continue;

        // delete buffer and make sure its not a buffer
        glDeleteBuffers(1, &vbo);
        if (glIsBuffer(vbo))
            continue;

        // create buffer again
        glGenBuffers(1, &vbo);
        
        // invalid enum
        glBindBuffer(0, 0);
        err = glGetError();
        if (err != GL_INVALID_ENUM)
            continue;

        // clear bindings
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // should cause a GL_INVALID_OPERATION, no buffer bound
        glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);
        err = glGetError();
        if (err != GL_INVALID_OPERATION)
            continue;

        // should cause a GL_INVALID_OPERATION, no buffer bound
        glBufferSubData(GL_ARRAY_BUFFER, 1023, 1024, data);
        err = glGetError();
        if (err != GL_INVALID_OPERATION)
            continue;

        // check if isBuffer works
        if (glIsBuffer(1234))
            continue;
        if (glIsBuffer(vbo) != true)
            continue;

        // check buffer test
        glBindBuffer(0, vbo);
        err = glGetError();
        if (err != GL_INVALID_ENUM)
            continue;

        // bind an invalid bufer, opengl requires buffers be created
        glBindBuffer(GL_ARRAY_BUFFER, 1234);
        err = glGetError();
        if (err != GL_INVALID_VALUE)
            continue;

        // bind buffer test
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        
        // generate invalid enum on type
        glBufferData(GL_INVALID_ENUM, 4096, 0, GL_STREAM_DRAW);
        err = glGetError();
        if (err != GL_INVALID_ENUM)
            continue;

        // generate invalid enum on useage
        glBufferData(GL_ARRAY_BUFFER, 4096, 0, 0);
        err = glGetError();
        if (err != GL_INVALID_ENUM)
            continue;

        // generate invalid value on size
        glBufferData(GL_ARRAY_BUFFER, -4096, 0, GL_STREAM_DRAW);
        err = glGetError();
        if (err != GL_INVALID_VALUE)
            continue;

        GLuint subdata[] = {0xdeadbeef, 0xcafeface, 0xbeefface, 0xfacebeef};
        
        // generate invalid enum on useage
        glBufferSubData(GL_INVALID_ENUM, 0, sizeof(subdata), subdata);
        err = glGetError();
        if (err != GL_INVALID_ENUM)
            continue;

        // generate no error on zero size
        glBufferSubData(GL_ARRAY_BUFFER, 0, 0, subdata);
        err = glGetError();
        if (err != GL_NO_ERROR)
            continue;

        // generate no error on NULL data
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subdata), NULL);
        err = glGetError();
        if (err != GL_NO_ERROR)
            continue;

        // generate error on neg offset and size
        glBufferSubData(GL_ARRAY_BUFFER, -4096, sizeof(subdata), subdata);
        err = glGetError();
        if (err != GL_INVALID_VALUE)
            continue;
        glBufferSubData(GL_ARRAY_BUFFER, 0, -sizeof(subdata), subdata);
        err = glGetError();
        if (err != GL_INVALID_VALUE)
            continue;

        
        // bind a buffer with some data
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
        
        // generate GL_INVALID_VALUE if size of data is larger than buffer
        glBufferSubData(GL_ARRAY_BUFFER, 0, 2 * size, subdata);
        err = glGetError();
        if (err != GL_INVALID_VALUE)
            continue;

        // generate GL_INVALID_VALUE if offset will over run buffer
        glBufferSubData(GL_ARRAY_BUFFER, size - 1, sizeof(subdata), subdata);
        err = glGetError();
        if (err != GL_INVALID_VALUE)
            continue;

        volatile void *map_ptr;
        
        // bind to zero
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // generate invalide enum for target
        map_ptr = glMapBuffer(GL_INVALID_ENUM, GL_READ_ONLY);
        err = glGetError();
        if (err != GL_INVALID_ENUM)
            continue;

        // generate invalide enum for access
        map_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_INVALID_ENUM);
        err = glGetError();
        if (err != GL_INVALID_ENUM)
            continue;

        GLuint map_vbo;
        glGenBuffers(1, &map_vbo);
        
        // a map on a null buffer should return GL_INVALID_OPERATION
        glBindBuffer(GL_ARRAY_BUFFER, map_vbo);
        map_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
        err = glGetError();
        if (err != GL_INVALID_OPERATION)
            continue;

        // delete map buffer
        glDeleteBuffers(1, &map_vbo);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        
        // should just work
        map_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
        err = glGetError();
        if (err != GL_NO_ERROR)
            continue;

        // should map to something
        if (map_ptr == NULL)
            continue;

        // try mapping it again to produce a GL_INVALID_OPERATION
        map_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
        err = glGetError();
        if (err != GL_INVALID_OPERATION)
            continue;

        // should fail target and produce an invalid enum
        if (glUnmapBuffer(GL_INVALID_ENUM))
            continue;

        // should geneate a invalid enum
        err = glGetError();
        if (err != GL_INVALID_ENUM)
            continue;

        // delete buffer again
        glDeleteBuffers(1, &vbo);
        
        pass = true;
    } while(false);

    if (pass == false)
    {
        printf("VBO Test failed\n");
        
        assert(0);
    }
    
    // clear gl error
    glGetError();

    free(data);

    return pass;
}


- (void)updateLayer
{
    const char* vertex_shader[] =
    {
        GLSL(460,
          layout(location = 0) in vec3 position;
          void main() {
            gl_Position = vec4(position.x, position.y, position.z, 1.0);
            }
        ),
        GLSL(460,
             layout(location = 0) in vec3 position;
             void main() {
                gl_Position = vec4(position.x, position.y + 0.25, position.z, 1.0);
            }
        ),
        GLSL(460,
             layout(location = 0) in vec3 position;
             void main() {
                gl_Position = vec4(position.x + 0.25, position.y, position.z, 1.0);
            }
        ),
        GLSL(460,
             layout(location = 0) in vec3 position;
             void main() {
                gl_Position = vec4(position.x + 0.25, position.y + 0.25, position.z, 1.0);
            }
        )
    };
    const char* fragment_shader[] =
    {
        GLSL(460,
             layout(location = 0) out vec4 frag_colour;
             layout(location = 0) uniform int mp;
             void main() {
                frag_colour = vec4(0, 0, float(mp)/100.0, 1.0);
        }
        ),
        GLSL(460,
             layout(location = 0) out vec4 frag_colour;
             layout(location = 0) uniform int mp;
             void main() {
                frag_colour = vec4(float(mp)/100.0, 0, 0, 1.0);
        }
        ),
        GLSL(460,
             layout(location = 0) out vec4 frag_colour;
             layout(location = 0) uniform int mp;
             void main() {
                frag_colour = vec4(0, float(mp)/100.0, 0, 1.0);
        }
        ),
        GLSL(460,
             layout(location = 0) out vec4 frag_colour;
             layout(location = 0) uniform int mp;
             void main() {
                frag_colour = vec4(0, 1, float(mp)/100.0, 1.0);
        }
        )
    };
    const char* instanced_vertex_shader =
    GLSL(460,
         layout(location = 0) in vec3 position;
         layout(location = 0) out int instanceIndex;
         void main() {
            instanceIndex = gl_InstanceID;
        
            if (gl_InstanceID == 0)
                gl_Position = vec4(position.x, position.y, position.z, 1.0);
            else if (gl_InstanceID == 1)
                gl_Position = vec4(position.x + 0.2, position.y, position.z, 1.0);
            else if (gl_InstanceID == 2)
                gl_Position = vec4(position.x + 0.4, position.y, position.z, 1.0);
            else if (gl_InstanceID == 3)
                gl_Position = vec4(position.x, position.y + 0.4, position.z, 1.0);
            else
                gl_Position = vec4(position.x + 0.4, position.y + 0.4, position.z, 1.0);
        }
    );
    const char* instance_fragment_shader =
    GLSL(460,
         layout(location = 0) flat in int instanceIndex;
         layout(location = 0) out vec4 frag_colour;
         void main() {
            if (instanceIndex == 0)
                frag_colour = vec4(1, 0, 0, 1.0);
            else if (instanceIndex == 1)
                frag_colour = vec4(0, 1, 0, 1.0);
            else if (instanceIndex == 2)
                frag_colour = vec4(0, 0, 1, 1.0);
            else if (instanceIndex == 3)
                frag_colour = vec4(1, 0, 1, 1.0);
            else
                frag_colour = vec4(1, 1, 1, 1.0);
        }
    );

    GLfloat points[] = {
       0.0f,  0.5f,  0.0f,
       0.5f, -0.5f,  0.0f,
      -0.5f, -0.5f,  0.0f
    };
    
    GLuint indices[] = {
        0, 1, 2
    };
    
    if (m_first_pass)
    {
        if ([self checkVBOErrors] == false)
            NSLog(@"checkVBOErrors failed");
            
        m_vao = createAndBindVAO();
        
        size_t vbo_size;
        
        vbo_size = 4096;
        
        for(int i=0; i<16; i++)
        {
            if (i == 0)
            {
                m_vbo[i] = bindDataToVBO(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
                m_elements[i] = bindDataToVBO(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
            }
            else
            {
                assert(vbo_size);
                
                m_vbo[i] = createVBO(GL_ARRAY_BUFFER, vbo_size, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, m_vbo[i]);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                m_elements[i] = createVBO(GL_ELEMENT_ARRAY_BUFFER, vbo_size, GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elements[i]);
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                vbo_size <<= 1;
                
                if (vbo_size > (64*1024*1024))
                {
                    vbo_size = 4096;
                }
            }
        }
        
        CHECK_GL_ERR();

        bindNullAttribute(0, GL_ARRAY_BUFFER, m_vbo[0], 3, GL_FLOAT, false, 0);
        CHECK_GL_ERR();

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        CHECK_GL_ERR();
        
        // create 16 unique shaders so random can swap these around
        int k=0;
        for(int i=0; i<4; i++)
        {
            for(int j=0; j<4; j++)
            {
                m_shader_program[k] = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader[i], GL_FRAGMENT_SHADER, fragment_shader[j]);
                assert(m_shader_program[k]);
                CHECK_GL_ERR();
                k++;
            }
        }
        
        m_instanced_shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, instanced_vertex_shader, GL_FRAGMENT_SHADER, instance_fragment_shader);
        assert(m_instanced_shader_program);
        CHECK_GL_ERR();

        glUseProgram(0);
        CHECK_GL_ERR();

        m_first_pass = 0;
    }
    
    
    GLuint numPasses;
    
    numPasses = 1;

    if (m_primiteDrawType == kRandomPrimitiveType)
    {
        numPasses = (GLuint)random();
        numPasses = numPasses % 128;
    }
    
    for(int pass=0; pass<numPasses; pass++)
    {
        GLuint drawType;
        GLuint programNum;
        GLuint vboNum;
        GLuint elemNum;
        
        programNum = 0;
        vboNum = 0;
        elemNum = 0;
        
        drawType = m_primiteDrawType;

        if (m_primiteDrawType == kRandomPrimitiveType)
        {
            drawType = (GLuint)random();
            drawType = drawType % (kRandomPrimitiveType - 1);

            programNum = (GLuint)random();
            programNum = programNum % 16;
            assert(programNum < 16);
            
            vboNum = (GLuint)random();
            vboNum = vboNum % 16;
            assert(vboNum < 16);

            elemNum = (GLuint)random();
            elemNum = elemNum % 16;
            assert(elemNum < 16);
        }

        GLboolean use_unforms;
        
        switch(drawType)
        {
            case kDrawArrays:
            case kDrawElements:
            case kDrawRangeElements:
                use_unforms = true;
                glUseProgram(m_shader_program[programNum]);
                break;
                
            case kDrawArraysInstanced:
            case kDrawElementsInstanced:
                use_unforms = false;
                glUseProgram(m_instanced_shader_program);
                break;
                
            default:
                assert(0);
                break;
        }
        
        //char *drawTypeStr[] = {"DrawArrays", "DrawElements", "DrawRangeElements", "DrawArraysInstanced",
        //    "DrawElementsInstanced"};
        
        //printf("pass: %d drawType: %s programNum:%d vbo:%d elem:%d\n", pass, drawTypeStr[drawType], programNum, vboNum, elemNum);
        
        GLboolean needs_elements;
        
        switch(drawType)
        {
            case kDrawElements:
            case kDrawRangeElements:
            case kDrawElementsInstanced:
                needs_elements = true;
                break;
                
            default:
                needs_elements = false;
        }
        
        if (pass == 0)
        {
            glClear(GL_COLOR_BUFFER_BIT);
        }
        
        glVertexArrayElementBuffer(m_vao, 0);
        CHECK_GL_ERR();
        
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo[vboNum]);
        CHECK_GL_ERR();
        
        bindNullAttribute(0, GL_ARRAY_BUFFER, m_vbo[vboNum], 3, GL_FLOAT, false, 0);
        
        if (use_unforms)
        {
            GLint programID;
            glGetIntegerv(GL_CURRENT_PROGRAM, &programID);
            
            assert(programID);
            
            GLint mp_loc = glGetUniformLocation(programID, "mp");
            
            glUniform1i(mp_loc, m_a);
        }
        
        // use 4x stuff for elements to test
        if (m_4x_mode)
        {
            if (needs_elements)
                glVertexArrayElementBuffer(m_vao, m_elements[elemNum]);
            else
                glVertexArrayElementBuffer(m_vao, 0);
        }
        else
        {
            if (needs_elements)
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elements[elemNum]);
            else
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        
        CHECK_GL_ERR();
        
        switch(drawType)
        {
            case kDrawArrays:
                glClearColor(1.0, 0.0, 0.0, 0.0);
                glDrawArrays(GL_TRIANGLES, 0, 3);
                CHECK_GL_ERR();
                break;
                
            case kDrawElements:
                glClearColor(0.0, 1.0, 0.0, 0.0);
                glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
                CHECK_GL_ERR();
                break;
                
            case kDrawRangeElements:
                glClearColor(0.0, 1.0, 0.0, 0.0);
                glDrawRangeElements(GL_TRIANGLES, 0, 3, 3, GL_UNSIGNED_INT, 0);
                CHECK_GL_ERR();
                break;
                
            case kDrawArraysInstanced:
                glClearColor(1.0, 0.0, 1.0, 0.0);
                glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 3);
                CHECK_GL_ERR();
                break;
                
            case kDrawElementsInstanced:
                glClearColor(1.0, 1.0, 1.0, 0.0);
                glDrawElementsInstanced(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0, 3);
                CHECK_GL_ERR();
                break;
                
            default:
                assert(0);
        }
        
        m_a += m_e;
        
        if(m_a >= 100)
        {
            m_e = -1;
        }
        
        if(m_a == 0)
        {
            m_e =1 ;
        }
    }
    
    MGLswapBuffers(0);
}

- (void)drawRect:(NSRect)rect
{
    NSLog(@"%s %@", __FUNCTION__, NSStringFromRect(rect));
    
    [self needsDisplay];
}

@end

NS_ASSUME_NONNULL_END
