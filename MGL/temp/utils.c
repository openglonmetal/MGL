//
//  utils.c
//  mgl
//
//  Created by Michael Larson on 11/20/21.
//

#include "utils.h"

#include "mgl.h"

GLuint ilog2(GLuint val)
{
    uint32_t retval;
    __asm {
        bsr eax, val
        mov retval, eax
    }
    return retval;
}
