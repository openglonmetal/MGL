//
//  Test_clear.h
//  test_gl
//
//  Created by Michael Larson on 1/20/26.
//

#import "GLTest.h"

#ifdef __cplusplus

class Test_clear : public GLTest
{
public:
    int a;
    int e;
    
    float f;
    
public:
    void setup() override
    {
        a = 0;
        e = 1;
    }

    void update() override
    {
    }

    void draw() override
    {
        f = (float)a/100.0;

        glClearColor(1.0 - f, 0.2, f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        a += e;
        if(a>=100)
        {
            e=-1;
        }
        
        if(a==0)
        {
            e=1;
        }

        MGLswapBuffers(MGLgetCurrentContext());
    }
};

#endif // __cplusplus
