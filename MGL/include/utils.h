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
 * utils.h
 * MGL
 *
 */

#ifndef utils_h
#define utils_h

#include "glcorearb.h"

#ifndef MAX
#define MAX(_A_, _B_)   (_A_ > _B_ ? _A_ : _B_)
#endif

#ifndef MIN
#define MIN(_A_, _B_)    (_A_ < _B_) ? _A_ : _B_
#endif


#ifdef __cplusplus
extern "C" {
#endif

GLuint ilog2(GLuint val);

GLfloat clamp(GLfloat a, GLfloat min, GLfloat max);

GLuint maxLevels(GLuint width, GLuint height, GLuint depth);
GLboolean checkMaxLevels(GLuint levels, GLuint width, GLuint height, GLuint depth);


#ifdef __cplusplus
}
#endif

#endif /* utils_h */
