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
 * shaders.h
 * MGL
 *
 */

#ifndef shaders_h
#define shaders_h

#include "glcorearb.h"
#include "glm_context.h"

Shader *findShader(GLMContext ctx, GLuint shader);
void mglFreeShader(GLMContext ctx, Shader *ptr);

#endif /* shaders_h */
