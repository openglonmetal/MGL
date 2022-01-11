/*
 * Copyright (C) Michael Larson on 1/6/2022
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * utils.c
 * MGL
 *
 */

#include <math.h>

#include "utils.h"

#include "mgl.h"

GLuint ilog2(GLuint val)
{
    if (val == 0)
        return 0;
    
    return (GLuint)floor(log2(val));
}

GLfloat clamp(GLfloat a, GLfloat min, GLfloat max)
{
    a = MAX(a, min);
    a = MIN(a, max);

    return a;
}

GLuint maxLevels(GLuint width, GLuint height, GLuint depth)
{
    return (ilog2(MAX(width, MAX(height, depth))) + 1);
}

GLboolean checkMaxLevels(GLuint levels, GLuint width, GLuint height, GLuint depth)
{
    return levels <= maxLevels(width, height, depth);
}

